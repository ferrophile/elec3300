#ifndef BUTTON_H
#define BUTTON_H

#include "stm32f4xx.h"

#define BUTTON_COUNT 7
typedef void (*ButtonHandler)(void);

// Released = 1 since GPIO is set to pullup
enum {
	BUTTON_PRESSED,
	BUTTON_RELEASED
};

enum {
	BUTTON_1,
	BUTTON_2,
	SW_LEFT,
	SW_DOWN,
	SW_RIGHT,
	SW_MIDDLE,
	SW_UP
};

void button_init(void);
u8 button_get_state(u8 id);
void button_set_handler(u8 id, ButtonHandler handler);

#endif
