#include "ticks.h"

static const u8 us_delay = 28;

void ticks_init() {
	SysTick_Config(SystemCoreClock / 1000);
	ticks = 0;
}

u32 get_ticks() {
	return ticks;
}

void _delay_us(u16 delay) {
	u32 count = delay * us_delay;
	while(count--) {
		__asm__("nop");
	}
}

void _delay_ms(u16 delay) {
	u32 count = delay * us_delay * 1000;
	while(count--) {
		__asm__("nop");
	}
}
