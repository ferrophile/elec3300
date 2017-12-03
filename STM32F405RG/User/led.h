#ifndef LED_H
#define LED_H

#include "stm32f4xx.h"

#define LED_COUNT 2

enum {
	LED_A,
	LED_B
};

void led_init(void);
void led_on(u8 id);
void led_off(u8 id);
void led_toggle(u8 id);

#endif
