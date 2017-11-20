#ifndef TICKS_H
#define TICKS_H

#include "stm32f4xx.h"

void ticks_init(void);
u32 get_ticks(void);
void delay_us(u16 delay);

#endif
