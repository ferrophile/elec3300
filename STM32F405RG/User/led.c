#include "led.h"

static u8 ledState[LED_COUNT];

void led_init(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_ResetBits(GPIOC, GPIO_Pin_9 | GPIO_Pin_10);
	for (u8 i = 0; i < LED_COUNT; i++)
		ledState[i] = 0;
}

void led_on(u8 id) {
	if (id == LED_A) GPIO_SetBits(GPIOC, GPIO_Pin_9);	
	if (id == LED_B) GPIO_SetBits(GPIOC, GPIO_Pin_10);
	
	ledState[id] = 1;
}

void led_off(u8 id) {
	if (id == LED_A) GPIO_ResetBits(GPIOC, GPIO_Pin_9);	
	if (id == LED_B) GPIO_ResetBits(GPIOC, GPIO_Pin_10);
	
	ledState[id] = 0;
}

void led_toggle(u8 id) {
	if (ledState[id])
		led_off(id);
	else
		led_on(id);
}
