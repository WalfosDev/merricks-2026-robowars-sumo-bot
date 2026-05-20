#ifndef LINE_DETECTOR_H
#define LINE_DETECTOR_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

QueueHandle_t line_detector_init();

typedef enum {
    LINE_DETECTED,
    NO_LINE
} line_detector_states_t;

#endif