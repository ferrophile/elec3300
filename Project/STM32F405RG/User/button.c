#include "button.h"

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

void button_interrupt(u8 id) {
	u8 pinSource;
	u32 extiLine;
	u8 irqChannel;
	
	switch (id) {
	case BUTTON_1:
		pinSource = EXTI_PinSource8;
		extiLine = EXTI_Line8;
		irqChannel = EXTI9_5_IRQn;
		break;
	case BUTTON_2:
		pinSource = EXTI_PinSource9;
		extiLine = EXTI_Line9;
		irqChannel = EXTI9_5_IRQn;
		break;
	case SW_1:
		pinSource = EXTI_PinSource3;
		extiLine = EXTI_Line3;
		irqChannel = EXTI3_IRQn;
		break;
	case SW_2:
		pinSource = EXTI_PinSource4;
		extiLine = EXTI_Line4;
		irqChannel = EXTI4_IRQn;
		break;
	case SW_3:
		pinSource = EXTI_PinSource5;
		extiLine = EXTI_Line5;
		irqChannel = EXTI9_5_IRQn;
		break;
	case SW_4:
		pinSource = EXTI_PinSource6;
		extiLine = EXTI_Line6;
		irqChannel = EXTI9_5_IRQn;
		break;
	case SW_5:
		pinSource = EXTI_PinSource7;
		extiLine = EXTI_Line7;
		irqChannel = EXTI9_5_IRQn;
		break;
	}
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, pinSource);
	
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = extiLine;
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

void EXTI9_5_IRQHandler() {
	if (EXTI_GetFlagStatus(EXTI_Line9) == SET) {
		//test_count++;
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
}
 