#ifndef MOTOR_CONTROLL_H
#define MOTOR_CONTROLL_H

typedef enum {
    M_STOP,
    M_L,
    M_R,
    M_F,
    M_FL,
    M_FR,
    M_B,
    M_BL,
    M_BR
} drive_state_t;

typedef enum {
    FWD,
    REV,
    STP
} motor_state_t;

typedef struct {
    const uint32_t lm_speed;
    const uint32_t rm_speed;
    const motor_state_t lm_state;
    const motor_state_t rm_state;
} dual_motor_output_t;

void motor_controll_init();
void motor_controll_apply_drive_state(drive_state_t new_state);

#endif