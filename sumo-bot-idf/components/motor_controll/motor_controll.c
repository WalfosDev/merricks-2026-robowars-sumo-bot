/***
 * Simple configuration for the BTS7960.
 */
#include <stdio.h>
#include "motor_controll.h"
#include "esp_attr.h"
#include "bdc_motor.h"
#include "esp_log.h"

// --- Constants ---
static const char *TAG = "MOTOR_CONTROLL";

static const bdc_motor_config_t RM_CONFIG = {
    .pwma_gpio_num = 32, // R pwm
    .pwmb_gpio_num = 27, // L pwn
    .pwm_freq_hz = 25000 // motor frequency
};

static const bdc_motor_config_t LM_CONFIG = {
    .pwma_gpio_num = 26, // R pwm
    .pwmb_gpio_num = 25, // L pwn
    .pwm_freq_hz = 25000 // motor frequency
};

static const dual_motor_output_t MOTOR_OUTPUT_LUT[] = {
    [M_STOP]    = { .lm_speed = 0,   .rm_speed = 0,   .lm_state = STP, .rm_state = STP },
    
    /* Pure Directional */
    [M_F]       = { .lm_speed = 800, .rm_speed = 800, .lm_state = FWD, .rm_state = FWD },
    [M_B]       = { .lm_speed = 800, .rm_speed = 800, .lm_state = REV, .rm_state = REV },
    
    /* Hard Pivots (Spin in place) */
    [M_L]       = { .lm_speed = 600, .rm_speed = 600, .lm_state = REV, .rm_state = FWD },
    [M_R]       = { .lm_speed = 600, .rm_speed = 600, .lm_state = FWD, .rm_state = REV },
    
    /* Forward Diagonals (Arc Turns) */
    [M_FL]      = { .lm_speed = 300, .rm_speed = 800, .lm_state = FWD, .rm_state = FWD },
    [M_FR]      = { .lm_speed = 800, .rm_speed = 300, .lm_state = FWD, .rm_state = FWD },
    
    /* Backward Diagonals (Arc Turns) */
    [M_BL]      = { .lm_speed = 300, .rm_speed = 800, .lm_state = REV, .rm_state = REV },
    [M_BR]      = { .lm_speed = 800, .rm_speed = 300, .lm_state = REV, .rm_state = REV }
};

static const bdc_motor_mcpwm_config_t TIMER_CONFIG = {
    .group_id = 0,
    .resolution_hz = 25000000
};

// --- Program ---
static bdc_motor_handle_t rm_handle;
static bdc_motor_handle_t lm_handle;

void motor_controll_init() {
    ESP_LOGD(TAG, "Initializing Motors");
    ESP_ERROR_CHECK( bdc_motor_new_mcpwm_device(&RM_CONFIG, &TIMER_CONFIG, &rm_handle) );
    ESP_ERROR_CHECK( bdc_motor_new_mcpwm_device(&LM_CONFIG, &TIMER_CONFIG, &lm_handle) );
    ESP_ERROR_CHECK( bdc_motor_enable(rm_handle) );
    ESP_ERROR_CHECK( bdc_motor_enable(lm_handle) );
}

void motor_controll_apply_drive_state(drive_state_t new_state) {
    static drive_state_t last_state = M_STOP;
    
    if (new_state == last_state) return;  // no-op, nothing changed

    const dual_motor_output_t* output = &MOTOR_OUTPUT_LUT[new_state];
    //ESP_LOGD(TAG, "Applying Drive State %d", new_state);

    ESP_ERROR_CHECK( bdc_motor_set_speed(lm_handle, output->lm_speed) );
    if (output->lm_state == FWD)
        ESP_ERROR_CHECK( bdc_motor_forward(lm_handle) );
    else if (output->lm_state == REV)
        ESP_ERROR_CHECK( bdc_motor_reverse(lm_handle) );
    else if (output->lm_state == STP)
        ESP_ERROR_CHECK( bdc_motor_brake(lm_handle) );

    ESP_ERROR_CHECK( bdc_motor_set_speed(rm_handle, output->rm_speed) );
    if (output->rm_state == FWD)
        ESP_ERROR_CHECK( bdc_motor_forward(rm_handle) );
    else if (output->rm_state == REV)
        ESP_ERROR_CHECK( bdc_motor_reverse(rm_handle) );
    else if (output->rm_state == STP)
        ESP_ERROR_CHECK( bdc_motor_brake(rm_handle) );
}
