#ifndef IR_ARRAY_H
#define IR_ARRAY_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

QueueHandle_t ir_array_init();

typedef enum {
    none,
    IR_W,    // Dead Left
    IR_WNW,  // Mostly Left, slightly Front
    IR_NW,   // 45 degrees Front-Left
    IR_NNW,  // Mostly Front, slightly Left
    IR_N,    // Dead Front
    IR_NNE,  // Mostly Front, slightly Right
    IR_NE,   // 45 degrees Front-Right
    IR_ENE,  // Mostly Right, slightly Front
    IR_E     // Dead Right
} ir_array_direction_t;

#endif