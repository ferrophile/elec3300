#include "main.h"

u32 ticksImg = 0;

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	
	led_init();
	ticks_init();
	stepper_init();
	tft_init(PIN_ON_BOTTOM, BLACK, WHITE, RED);
	
	led_on(LED_A);
	led_off(LED_B);
	
	while(1) {
		if (ticksImg != get_ticks()) {
			ticksImg = get_ticks();
			
			if (ticksImg % 20 == 0) {
				tft_clear();
				tft_println("%d", ticksImg);
				tft_update();
			}
			
			if (ticksImg % 500 == 0) {
				led_toggle(LED_A);
				led_toggle(LED_B);
			}
		}
	}
}
