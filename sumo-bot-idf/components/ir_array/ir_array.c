#include <stdio.h>
#include "ir_array.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"
// #include "esp_adc/adc_oneshot.h"
// #include "hal/adc_types.h"


// --- Constants ---
#define IR_REC_PERIOD_US 500
// #define IR_REC_THRESHOLD 2000
// #define IR_REC_CHANNEL ADC_CHANNEL_8 //GPIO35

// --- Program ---
static const char *TAG = "IR_ARRAY";

void ir_array_isr(void* arg);

// adc_oneshot_unit_handle_t ir_rec_adc_handler;

QueueHandle_t xQueueIR;
static esp_timer_handle_t ir_timer_handle;


QueueHandle_t ir_array_init()
{
    // @todo: Handle queue unable to be created, probably imposible for this application
    xQueueIR = xQueueCreate(1, sizeof( struct ir_array_direction_t *) );

    // Create interupt
    esp_timer_create_args_t TIMER_CONFIG = {
        .callback = ir_array_isr,
        .dispatch_method = ESP_TIMER_ISR,
        .name = "ir_array_isr"
    };
    ESP_ERROR_CHECK( esp_timer_create(&TIMER_CONFIG, &ir_timer_handle) );
    ESP_ERROR_CHECK( esp_timer_start_periodic(ir_timer_handle, IR_REC_PERIOD_US) );

    // ADC init commented out — null pointer issue
    // adc_oneshot_unit_init_cfg_t init_config = {
    //     .unit_id = ADC_UNIT_2,
    // };
    // ESP_ERROR_CHECK( adc_oneshot_new_unit(&init_config, &ir_rec_adc_handler) );
    // adc_oneshot_chan_cfg_t config = {
    //     .bitwidth = ADC_BITWIDTH_DEFAULT,
    //     .atten = ADC_ATTEN_DB_12,
    // };
    // ESP_ERROR_CHECK( adc_oneshot_config_channel(ir_rec_adc_handler, IR_REC_CHANNEL, &config) );

    return xQueueIR;
}


void IRAM_ATTR ir_array_isr(void* arg) {
    // Dummy — always reports IR_N (no target detected)
    ir_array_direction_t cur_direction = IR_N;
    xQueueSendToBackFromISR( xQueueIR, ( void * ) &cur_direction, false);
}