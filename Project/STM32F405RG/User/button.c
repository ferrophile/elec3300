#include "button.h"

static ButtonHandler handlers[2] = {0};

void button_init(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	// Init interrupt stuff here as well
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5| GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource8);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource9);
	
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line8 | EXTI_Line9;
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
}

u8 button_get_state(u8 id) {
	if (id == BUTTON_1)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);
	if (id == BUTTON_2)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
	if (id == SW_1)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3);
	if (id == SW_2)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
	if (id == SW_3)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
	if (id == SW_4)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6);
	if (id == SW_5)	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7);
}

void button_set_handler(u8 id, ButtonHandler handler) {
	handlers[id] = handler;
}

void EXTI9_5_IRQHandler() {
	if (handlers[BUTTON_1] && EXTI_GetFlagStatus(EXTI_Line8) == SET) {
		handlers[BUTTON_1]();
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	
	if (handlers[BUTTON_2] && EXTI_GetFlagStatus(EXTI_Line9) == SET) {
		handlers[BUTTON_2]();
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
}
 