#ifndef JSUMO_H
#define JSUMO_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

QueueHandle_t jsumo_init();

typedef enum {
    ON,
    OFF
} jsumo_states_t;

#endif