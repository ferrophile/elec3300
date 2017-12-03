#include "stm32f4xx.h"

#include "led.h"
#include "scanner.h"

u32 ticksImg = 0;

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	
	led_init();
	ticks_init();
	button_init();
	stepper_init();
	laser_init();
	tft_init(PIN_ON_LEFT, BLACK, WHITE, RED);
	uart_init();
	
	led_on(LED_A);
	led_off(LED_B);
	
	scanner_init();
	
	while(1) {
		if (ticksImg != get_ticks()) {
			ticksImg = get_ticks();
			
			if (ticksImg % 20 == 0) {
				scanner_show();
				scanner_run();
			}
			
			if (ticksImg % 500 == 0) {
				led_toggle(LED_A);
				led_toggle(LED_B);
			}
		}
	}
}
