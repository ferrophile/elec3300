#ifndef STEPPER_H
#define STEPPER_H

#include "stm32f4xx.h"

#define STEPPER_COUNT 3

#define ABS(x) ((x)<0 ? -(x) : (x))
#define SIGN(x) ((x) / ABS(x))

typedef void (*SetCompareFunc)(TIM_TypeDef* TIMx, uint32_t Compare);
typedef void (*StepHandler)(void);

// Stepper ID
enum {
	STEPPER_1,
	STEPPER_2,
	STEPPER_3
};

typedef struct {
	/*-- Status variables --*/
	s16 velocity;							// Stepper velocity in RPM
	u16 countsBetweenPulses;	// Timer counts between pulses to generate the above velocity
	u8 pulseState;						// 1 - STEP pin is high (pulse trigger); 0 - STEP pin is low
	u8 setVelocityFlag;				// Flag for set velocity request
	s32 stepCount;						// No. of (signed) steps moved, ignoring skips
	s32 targetStepCount;			// Step count destination
	StepHandler handler;			// Handler function for each step (clear it if not needed)
	
	/*-- STM SPL constructs --*/
	u16 channel;
	u16 interruptChannel;
	SetCompareFunc setCompareFunc;
} Stepper;

void stepper_init(void);
void stepper_set_handler(u8 id, StepHandler newHandler);
s16 stepper_get_vel(u8 id);
void stepper_set_vel(u8 id, s16 vel);
void stepper_set_deg(u8 id, s16 vel, u32 degree);
u32 stepper_get_count(u8 id);
u8 stepper_is_idle(u8 id);

Stepper * stepper_get_params(u8 id);

#endif
