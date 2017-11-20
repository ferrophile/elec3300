#ifndef __STEPPER_H
#define __STEPPER_H

#include "stm32f4xx.h"
#include "ticks.h"
#include "macro_math.h"

typedef enum {
	STEPPER_1,
	STEPPER_2
} STEPPER;

void stepper_init(void);
void stepper_rotate(STEPPER id, s16 vel, u32 degree);
u16 stepper_get_vel(STEPPER id);
void stepper_set_vel(STEPPER id, s16 vel);

#endif
