#include <stdio.h>
#include "jsumo_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "JSUMO_MODULE";

#define JUSMO_PIN 4

static QueueHandle_t xQueueJsumo;

static void IRAM_ATTR jsumo_isr_handler(void* arg) {
    jsumo_states_t state;
    int jsumo_lvl = gpio_get_level(JUSMO_PIN);

    // Level 0 = LINE_DETECTED (QRE1113 pulls low when reflective surface found)
    if (jsumo_lvl == 1)
        state = ON;
    else
        state = OFF;

    xQueueOverwriteFromISR(xQueueJsumo, &state, NULL);
}

QueueHandle_t jsumo_init()
{
    xQueueJsumo = xQueueCreate(1, sizeof(jsumo_states_t));

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << JUSMO_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK( gpio_config(&io_conf) );

    // ESP_ERR_INVALID_STATE means another component already installed the service, which is fine
    esp_err_t ret = gpio_install_isr_service(0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(ret);
    }
    gpio_isr_handler_add(JUSMO_PIN, jsumo_isr_handler, NULL);

    ESP_LOGD(TAG, "Initialized jsumo on pins %d", JUSMO_PIN);

    return xQueueJsumo;
}
