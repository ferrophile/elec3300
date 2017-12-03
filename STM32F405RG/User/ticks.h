#ifndef TICKS_H
#define TICKS_H

#include "stm32f4xx.h"
#include "stm32f4xx_it.h"

void ticks_init(void);
u32 get_ticks(void);
void _delay_us(u16 delay);
void _delay_ms(u16 delay);

#endif
