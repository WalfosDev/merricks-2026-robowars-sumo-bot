#include <stdio.h>
#include "ir_to_drive.h"
#include "ir_array.h"
#include "motor_controll.h"

static const drive_state_t ir_drive_fwd_map[] = {
    [none]      = M_STOP,
    [IR_W]      = M_FL,
    [IR_WNW]    = M_FL,
    [IR_NW]     = M_F,
    [IR_NNW]    = M_F,
    [IR_N]      = M_F,
    [IR_NNE]    = M_F,
    [IR_NE]     = M_FR,
    [IR_ENE]    = M_FR,
    [IR_E]      = M_FR
};

static const drive_state_t ir_drive_flat_map[] = {
    [none]      = M_STOP,
    [IR_W]      = M_L,
    [IR_WNW]    = M_L,
    [IR_NW]     = M_L,
    [IR_NNW]    = M_L,
    [IR_N]      = M_R,
    [IR_NNE]    = M_R,
    [IR_NE]     = M_R,
    [IR_ENE]    = M_R,
    [IR_E]      = M_R
};

drive_state_t ir_direction_to_drive_fwd(ir_array_direction_t dir) {
    return ir_drive_fwd_map[dir];
}

drive_state_t ir_direction_to_drive_flat(ir_array_direction_t dir) {
    return ir_drive_flat_map[dir];
}