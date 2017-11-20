#include "ticks.h"

static u32 ticks = 0;
static const u8 us_delay = 28;

void ticks_init() {
	SysTick_Config(SystemCoreClock / 1000);
	ticks = 0;
}

u32 get_ticks() {
	return ticks;
}

void delay_us(u16 delay) {
	u32 count = delay * us_delay;
	while(count--) {
		__asm__("nop");
	}
}
