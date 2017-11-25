#include "stepper.h"

// List of steppers.
static STEPPER stepperList[STEPPER_COUNT];
static u32 stepCount[STEPPER_COUNT];

// This specifies the no. of timer counts for a trigger pulse.
static const u8 triggerPulseCount = 2; // 6 us

static void stepper_gpio_init(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	// STEP Pin
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	// DIR Pin
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_TIM3);
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
}

static void stepper_timer_init(void) {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	TIM_TimeBaseStruct.TIM_Prescaler = 503; // 3us per count
	TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStruct.TIM_Period = 0xFFFFFFFF; // TIM3 is 32-bit
	TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStruct);
	
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_Toggle;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OC1Init(TIM3, &TIM_OCInitStruct);
	TIM_OC1PreloadConfig(TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC1, DISABLE);

	TIM_OC2Init(TIM3, &TIM_OCInitStruct);
	TIM_OC2PreloadConfig(TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC2, DISABLE);
	
	TIM_OC3Init(TIM3, &TIM_OCInitStruct);
	TIM_OC3PreloadConfig(TIM3, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC3, DISABLE);
	
	TIM_Cmd(TIM3, ENABLE);
}

static void stepper_nvic_init(void) {
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

static void stepper_set_dir_pin(u8 id, u8 dir) {
	if (dir) {
		if (id == STEPPER_1) GPIO_SetBits(GPIOB, GPIO_Pin_15);
		if (id == STEPPER_2) GPIO_SetBits(GPIOB, GPIO_Pin_14);
		if (id == STEPPER_3) GPIO_SetBits(GPIOB, GPIO_Pin_13);
	} else {
		if (id == STEPPER_1) GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		if (id == STEPPER_2) GPIO_ResetBits(GPIOB, GPIO_Pin_14);
		if (id == STEPPER_3) GPIO_ResetBits(GPIOB, GPIO_Pin_13);
	}
}

static u32 stepper_get_pulse_count(s16 vel) {
	if (vel == 0)
		return 0;
	
	// Steps per revolution = 200
	// Frequency of pulses = RPM * 200 / 60
	// Period of pulses = 60 / (RPM * 200)
	// Period of timer count = 504 / 168E6 = 3E-6 (3us)
	// No. of counts per pulse = 60 / (RPM * 200 * 3E-6) = 1E5 / RPM
	
	u32 countsPerPulse = 100000 / ABS(vel);
	return countsPerPulse - triggerPulseCount;
}

void stepper_init(void) {
	STEPPER stepper;
	
	for (u8 i = 0; i < STEPPER_COUNT; i++) {
		stepper.velocity = 0;
		stepper.pulseState = 0;
		stepper.countsBetweenPulses = 0;
		stepper.interruptChannel = TIM_IT_CC1 << i;
		stepperList[i] = stepper;
	}
	
	stepper_gpio_init();
	stepper_timer_init();
	stepper_nvic_init();
}

s16 stepper_get_vel(u8 id) {
	return stepperList[id].velocity;
}

STEPPER * stepper_get_params(u8 id) {
	return &stepperList[id];
}

u32 stepper_get_count(u8 id) {
	return stepCount[id];
}

void stepper_set_vel(u8 id, s16 vel) {
	stepperList[id].velocity = vel;
	stepperList[id].countsBetweenPulses = stepper_get_pulse_count(vel);
	
	if (vel == 0) {
		TIM_ITConfig(TIM3, stepperList[id].interruptChannel, DISABLE);
	} else {
		u32 curCount = TIM_GetCounter(TIM3) + 1;
		if (id == STEPPER_1) TIM_SetCompare1(TIM3, curCount);
		if (id == STEPPER_2) TIM_SetCompare2(TIM3, curCount);
		if (id == STEPPER_3) TIM_SetCompare3(TIM3, curCount);
		
		TIM_ITConfig(TIM3, stepperList[id].interruptChannel, ENABLE);
		stepper_set_dir_pin(id, (vel>0 ? 1 : 0));
	}
}

void TIM3_IRQHandler(void) {
	for (u8 i = 0; i < STEPPER_COUNT; i++) {
		u16 interruptChannel = stepperList[i].interruptChannel;
		if (TIM_GetITStatus(TIM3, interruptChannel)) {
			TIM_ClearITPendingBit(TIM3, interruptChannel);
			
			// TIM3_CHx pin (STEP pin) will be automatically toggled by hardware.
			// This section only changes TIM3_CCRx to schedule the next toggle.
			// There is no need for modulus, since TIM3_ARR is set to 2^32-1 so
			// unsigned 32-bit addition and overflow should be correct.
			
			if (stepperList[i].pulseState) {
				// Falling edge; pin will be pulled down, wait for period until next pulse
				if (i == STEPPER_1) TIM_SetCompare1(TIM3, TIM_GetCounter(TIM3) + stepperList[i].countsBetweenPulses);
				if (i == STEPPER_2) TIM_SetCompare2(TIM3, TIM_GetCounter(TIM3) + stepperList[i].countsBetweenPulses);
				stepperList[i].pulseState = 0;
				stepCount[i]++;
			} else {
				// Rising edge; pin will be pulled up for short pulse
				if (i == STEPPER_1) TIM_SetCompare1(TIM3, TIM_GetCounter(TIM3) + triggerPulseCount);
				if (i == STEPPER_2) TIM_SetCompare2(TIM3, TIM_GetCounter(TIM3) + triggerPulseCount);
				stepperList[i].pulseState = 1;
			}
		}
	}
}
