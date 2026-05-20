#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "motor_controll.h"
#include "line_detector.h"
#include "jsumo_module.h"
#include "ir_array.h"
#include "ir_to_drive.h"
#include "main.h"

static const char *TAG = "MAIN";

void vMainLoop( void * pvParameters );
program_state_t updateStartSequence();
program_state_t updateSearch();
program_state_t updateAttack();
program_state_t updateEvade();

// Queues ect...
QueueHandle_t xLineQueueListener;
QueueHandle_t xIRQueueListener;
QueueHandle_t xJsumoQueueListner;
static ir_array_direction_t last_ir_dir = none; // is never none, retains the last seen direction of an oponent
static ir_array_direction_t cur_ir_dir = none;  // up to date can be none
static bool reset;

void app_main(void)
{
    ESP_LOGD(TAG, "Program started.");

    // Initialize motors and sensors
    motor_controll_init();
    xLineQueueListener = line_detector_init();
    xIRQueueListener = ir_array_init();
    xJsumoQueueListner = jsumo_init();

    // Initialize main program
    static TaskHandle_t xHandle = NULL;
    xTaskCreate( vMainLoop, "vMainLoop", 2048, NULL, tskIDLE_PRIORITY + 1, &xHandle );
    configASSERT( xHandle );
}

static const char* state_name(program_state_t s) {
    switch (s) {
        case start_sequence: return "START_SEQUENCE";
        case search:         return "SEARCH";
        case attack:         return "ATTACK";
        case evade:          return "EVADE";
        case stop:           return "STOP";
        default:             return "UNKNOWN";
    }
}

// Task to be created.
void vMainLoop( void * pvParameters )
{
    ESP_LOGD(TAG, "Main loop initialized..");
    static program_state_t state = stop;
    static program_state_t prev_state = -1;
    reset = false;

    for( ;; )
    {

        // Jsumo overides all states
        jsumo_states_t jsumoState;
        if ( xQueueReceive(xJsumoQueueListner, &( jsumoState ), 0) ) {
            if      (jsumoState == OFF)
                state = stop;
            else if (jsumoState == ON) {
                state = start_sequence;
                reset = true;
            }
        }

        if (state == stop) {
            // Block task until jsumo signals ON — do not busy-wait or return
            xQueueReceive(xJsumoQueueListner, &jsumoState, portMAX_DELAY);
            if (jsumoState == ON) {
                state = start_sequence;
                reset = true;
            }
            continue;
        }

        // Overides all states and enters evasion state if line detected. Handles exit logic.
        line_detector_states_t pxLineDetected;
        if ( xQueueReceive(xLineQueueListener, &( pxLineDetected ), 0) ) {
            if      (pxLineDetected == LINE_DETECTED)
                state = evade;
            else if (pxLineDetected == NO_LINE && state == evade) 
                state = search;
        }

        // Updates direction
        static ir_array_direction_t new_ir_dir;
        if ( xQueueReceive(xIRQueueListener, &( new_ir_dir ), 0) ) {
            // The last_ir_dir is never supposed to be none, retains the last seen direction of an oponent
            if (new_ir_dir != none)
                last_ir_dir = cur_ir_dir;
            if (last_ir_dir == none && new_ir_dir == none)
                last_ir_dir = cur_ir_dir = new_ir_dir;
            cur_ir_dir = new_ir_dir;
        }

        // updating state
        if      (state == start_sequence)
            state = updateStartSequence();
        else if (state == search)
            state = updateSearch();
        else if (state == attack)
            state = updateAttack();
        else if (state == evade)
            state = updateEvade();

        if (state != prev_state) {
            ESP_LOGI(TAG, "State: %s -> %s", state_name(prev_state), state_name(state));
            prev_state = state;
        }
    }
}

// Runs start sequence
program_state_t updateStartSequence() { 
    static uint8_t step_idx = 0;

    if (reset) {
        step_idx = 0;
        reset = false;
    }

    if (step_idx < START_SEQ_LEN) {
        const control_sequence_t* current_cmd = &START_SEQUENCE[step_idx];
        motor_controll_apply_drive_state(current_cmd->action);
        vTaskDelay( current_cmd->duration_ms / portTICK_PERIOD_MS);
        step_idx++;
    }

    /*
    if (step_idx < START_SEQ_LEN)
        return start_sequence;
    else
        return search;
    */

    if (step_idx == START_SEQ_LEN)
        step_idx = 0;

    return start_sequence;
}

// Moves towards last position enemy was spotted
program_state_t updateSearch() {
    if (cur_ir_dir != none)
        return attack;

    if (last_ir_dir == none)
        motor_controll_apply_drive_state(BASE_SEARCH_DIRECTION);
    else
        motor_controll_apply_drive_state( ir_direction_to_drive_fwd(last_ir_dir) );

    return search;
}

// Moves forward towards enemy
program_state_t updateAttack() { 
    if (cur_ir_dir == none)
        return search;

    motor_controll_apply_drive_state( ir_direction_to_drive_fwd(cur_ir_dir) );
    
    return attack; 
}

// State flow handled exterernally
program_state_t updateEvade()  {
    motor_controll_apply_drive_state( ir_direction_to_drive_flat(last_ir_dir) );
    return evade;
}