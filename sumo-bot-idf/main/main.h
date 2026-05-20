#ifndef MAIN_H
#define MAIN_H

#include "motor_controll.h"

// -- Type Definitions ---
typedef enum {
    start_sequence,
    search,
    attack,
    evade,
    stop
} program_state_t;

typedef struct {
    uint32_t duration_ms;
    drive_state_t action;
} control_sequence_t;

// --- Constants & Params ---
#define START_SEQ_LEN 3
static const control_sequence_t START_SEQUENCE[] = {
    [0] = { .duration_ms = 100, .action = M_FL },
    [1] = { .duration_ms = 200, .action = M_F },
    [2] = { .duration_ms = 100, .action = M_FR }
};

// The default search direction the robot turns, depends on where positioned after start_sequence
#define BASE_SEARCH_DIRECTION M_FR

#endif