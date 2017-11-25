#ifndef STEPPER_H
#define STEPPER_H

#include "stm32f4xx.h"

#define STEPPER_COUNT 3

#define ABS(x) ((x)<0 ? -(x) : (x))
#define SIGN(x) ((x) / ABS(x))

// Stepper ID
enum {
	STEPPER_1,
	STEPPER_2,
	STEPPER_3
};

typedef struct {
	s16 velocity;							// Stepper velocity in RPM
	u16 countsBetweenPulses;	// Timer counts between pulses to generate the above velocity
	u8 pulseState;						// 1 - STEP pin is high (pulse trigger); 0 - STEP pin is low
	u16 interruptChannel;			// TIM_IT_CCx
	u8 setVelocityFlag;				// Flag for set velocity request
	s32 stepCount;						// No. of (signed) steps moved, ignoring skips
	s32 targetStepCount;			// Step count destination
} STEPPER;

void stepper_init(void);
s16 stepper_get_vel(u8 id);
void stepper_set_vel(u8 id, s16 vel);
void stepper_set_deg(u8 id, s16 vel, u32 degree);
u32 stepper_get_count(u8 id);

STEPPER * stepper_get_params(u8 id);

#endif
