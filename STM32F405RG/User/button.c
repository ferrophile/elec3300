#include "button.h"

static ButtonHandler handlers[BUTTON_COUNT] = {0};

void button_init(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5| GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource3);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource5);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource6);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource7);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource8);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource9);
	
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line3 | EXTI_Line4 | EXTI_Line5 | EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);
}

u8 button_get_state(u8 id) {
	if (id == BUTTON_1)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);
	if (id == BUTTON_2)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
	if (id == SW_LEFT)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);
	if (id == SW_DOWN)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
	if (id == SW_RIGHT)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
	if (id == SW_MIDDLE)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6);
	if (id == SW_UP)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7);
	return 1;
}

void button_set_handler(u8 id, ButtonHandler handler) {
	handlers[id] = handler;
}

void EXTI3_IRQHandler() {
	if (EXTI_GetFlagStatus(EXTI_Line3) == SET) {
		if (handlers[SW_LEFT]) handlers[SW_LEFT]();
		EXTI_ClearITPendingBit(EXTI_Line3);
	}
}

void EXTI4_IRQHandler() {
	if (EXTI_GetFlagStatus(EXTI_Line4) == SET) {
		if (handlers[SW_DOWN]) handlers[SW_DOWN]();
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}

void EXTI9_5_IRQHandler() {
	if (EXTI_GetFlagStatus(EXTI_Line8) == SET) {
		if (handlers[BUTTON_1]) handlers[BUTTON_1]();
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	
	if (EXTI_GetFlagStatus(EXTI_Line9) == SET) {
		if (handlers[BUTTON_2]) handlers[BUTTON_2]();
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
	
	if (EXTI_GetFlagStatus(EXTI_Line5) == SET) {
		if (handlers[SW_RIGHT]) handlers[SW_RIGHT]();
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
	
	if (EXTI_GetFlagStatus(EXTI_Line6) == SET) {
		if (handlers[SW_MIDDLE]) handlers[SW_MIDDLE]();
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	
	if (EXTI_GetFlagStatus(EXTI_Line7) == SET) {
		if (handlers[SW_UP]) handlers[SW_UP]();
		EXTI_ClearITPendingBit(EXTI_Line7);
	}
}
