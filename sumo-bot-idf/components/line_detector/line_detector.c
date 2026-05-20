/**
 * Handles the SENS36 Line Sensor Breakout module
 */

#include <stdio.h>
#include "line_detector.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "Line_Detector";

#define LINE_DETECTOR1_PIN 34
#define LINE_DETECTOR2_PIN 35
#define LINE_DETECTION_INTERVAL 2000

static QueueHandle_t xQueueLine;

static void IRAM_ATTR line_detector_isr_handler(void* arg) {
    line_detector_states_t state;
    int level1 = gpio_get_level(LINE_DETECTOR1_PIN);
    int level2 = gpio_get_level(LINE_DETECTOR2_PIN);

    // Level 0 = LINE_DETECTED (QRE1113 pulls low when reflective surface found)
    if (level1 == 0 || level2 == 0)
        state = LINE_DETECTED;
    else
        state = NO_LINE;

    xQueueOverwriteFromISR(xQueueLine, &state, NULL);
}

QueueHandle_t line_detector_init()
{
    xQueueLine = xQueueCreate(1, sizeof(line_detector_states_t));

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LINE_DETECTOR1_PIN) | (1ULL << LINE_DETECTOR2_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK( gpio_config(&io_conf) );

    // ESP_ERR_INVALID_STATE is acceptable if another component already installed the service
    gpio_install_isr_service(0);
    gpio_isr_handler_add(LINE_DETECTOR1_PIN, line_detector_isr_handler, NULL);
    gpio_isr_handler_add(LINE_DETECTOR2_PIN, line_detector_isr_handler, NULL);

    ESP_LOGD(TAG, "Initialized on pins %d and %d", LINE_DETECTOR1_PIN, LINE_DETECTOR2_PIN);

    return xQueueLine;
}
