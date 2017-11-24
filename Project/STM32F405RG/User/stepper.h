#ifndef STEPPER_H
#define STEPPER_H

#include "stm32f4xx.h"

#define STEPPER_COUNT 3

// Stepper ID
enum {
	STEPPER_1,
	STEPPER_2,
	STEPPER_3
};

typedef struct {
	s16 velocity;							// Stepper velocity in RPM
	u32 countsBetweenPulses;	// Timer counts between pulses to generate the above velocity
	u8 pulseState;						// 1 - STEP pin is high (pulse trigger); 0 - STEP pin is low
	u16 interruptChannel;			// TIM_IT_CCx
} STEPPER;

void stepper_init(void);
s16 stepper_get_vel(u8 id);
void stepper_set_vel(u8 id, s16 vel);
void stepper_set_deg(u8 id, s16 vel, u32 degree);

#endif
