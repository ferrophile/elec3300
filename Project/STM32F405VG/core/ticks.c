#include "ticks.h"

// IMPORTANT:
// The tick variable is defined in stm32f4xx_it.c
// And it is incremented by SysTicks

/**
  * @brief  Initialization of ticks timer
  * @param  None
  * @retval None
  */
void ticks_init() {
	SysTick_Config(SystemCoreClock/1000);
	msTicks = 0;
}

//Generate a delay in us  (inaccurate when value too small)
void _delay_us(uint16_t us){
	volatile uint32_t count = us*28;
	while(count--){
		__asm__("nop");
	}
}

//Generate a delay in ms
void _delay_ms(uint16_t ms){
	volatile uint32_t count = ms*28000;
	while(count--){
		__asm__("nop");
	}
}
