#include "stepper.h"

u32 curSteps[2] = {0, 0};
u16 stepsPerCycle = 200;
u16 rpm[2] = {0, 0};

const u16 TIM_PRESC = 1680;
const u16 MAX_RPM = 800;

void stepper_init(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = TIM_PRESC - 1;	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseStructure.TIM_Period = 3000; // 1 RPM
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM5, ENABLE);
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM4, ENABLE);
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	
	GPIO_ResetBits(GPIOE, GPIO_Pin_1);
	GPIO_ResetBits(GPIOE, GPIO_Pin_9);
}

static void stepper_set_dir_pin(STEPPER id, u8 dir) {
	if (id == STEPPER_1) {
		if (dir == 0)
			GPIO_ResetBits(GPIOE, GPIO_Pin_1);
		else if (dir == 1)
			GPIO_SetBits(GPIOE, GPIO_Pin_1);
	}
	
	if (id == STEPPER_2) {
		if (dir == 0)
			GPIO_ResetBits(GPIOE, GPIO_Pin_9);
		else if (dir == 1)
			GPIO_SetBits(GPIOE, GPIO_Pin_9);
	}
}

static void stepper_set_rpm(STEPPER id, u16 new_rpm) {
	if (new_rpm > MAX_RPM)
		return;
	
	rpm[id] = new_rpm;
	if (id == STEPPER_1) {
		if (new_rpm == 0) {
			TIM_Cmd(TIM5, DISABLE);
		} else {
			u32 new_arr = SystemCoreClock / TIM_PRESC * 60 / (new_rpm * stepsPerCycle);
			TIM_SetAutoreload(TIM5, (u16)new_arr);
			TIM_Cmd(TIM5, ENABLE);
		}
	}
	
	if (id == STEPPER_2) {
		if (new_rpm == 0) {
			TIM_Cmd(TIM4, DISABLE);
		} else {
			u32 new_arr = SystemCoreClock / TIM_PRESC * 60 / (new_rpm * stepsPerCycle);
			TIM_SetAutoreload(TIM4, (u16)new_arr);
			TIM_Cmd(TIM4, ENABLE);
		}
	}
}

void stepper_rotate(STEPPER id, s16 vel, u32 degree) {
	curSteps[id] = ABS(degree * stepsPerCycle / 360);
	stepper_set_dir_pin(id, (vel >= 0) ? 1 : 0);
	stepper_set_rpm(id, vel);
	
	if (id == STEPPER_1) {
		if (degree == 0)
			TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);
		else
			TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	}
	
	if (id == STEPPER_2) {
		if (degree == 0)
			TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
		else
			TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	}
}

u16 stepper_get_vel(STEPPER id) {
	return rpm[id];
}

void stepper_set_vel(STEPPER id, s16 vel) {
	curSteps[id] = 2147483647;
	stepper_set_dir_pin(id, (vel >= 0) ? 1 : 0);
	stepper_set_rpm(id, ABS(vel));
	
	if (id == STEPPER_1) {
		if (vel == 0)
			TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);
		else
			TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	}

	if (id == STEPPER_2) {
		if (vel == 0)
			TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
		else
			TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	}
}

void TIM5_IRQHandler(void) {
	if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
		GPIO_SetBits(GPIOE, GPIO_Pin_0);
		_delay_us(5);
		GPIO_ResetBits(GPIOE, GPIO_Pin_0);
		curSteps[0]--;
		
		if (curSteps[0] == 0)
			TIM_ITConfig(TIM5, TIM_IT_Update, DISABLE);
	}
}

void TIM4_IRQHandler(void) {
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) {
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		GPIO_SetBits(GPIOE, GPIO_Pin_8);
		_delay_us(5);
		GPIO_ResetBits(GPIOE, GPIO_Pin_8);
		curSteps[1]--;
		
		if (curSteps[1] == 0)
			TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
	}
}
