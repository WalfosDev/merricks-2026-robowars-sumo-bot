#ifndef IR_TO_DRIVE_H
#define IR_TO_DRIVE_H

#include "ir_array.h"
#include "motor_controll.h"

drive_state_t ir_direction_to_drive_fwd(ir_array_direction_t dir);
drive_state_t ir_direction_to_drive_flat(ir_array_direction_t dir);

#endif