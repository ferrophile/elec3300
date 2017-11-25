#include "main.h"

u32 ticksImg = 0;
s16 scanner_vel = 0;

void scanner_up_handler() {
	stepper_set_vel(STEPPER_1, stepper_get_vel(STEPPER_1) + 50);
}

void scanner_dn_handler() {
	stepper_set_vel(STEPPER_1, stepper_get_vel(STEPPER_1) - 50);
}

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	
	led_init();
	ticks_init();
	button_init();
	stepper_init();
	tft_init(PIN_ON_BOTTOM, BLACK, WHITE, RED);
	
	led_on(LED_A);
	led_off(LED_B);
	
	button_set_handler(BUTTON_2, scanner_up_handler);
	
	while(1) {
		if (ticksImg != get_ticks()) {
			ticksImg = get_ticks();
			
			if (ticksImg % 20 == 0) {
				tft_clear();
				tft_println("%d", ticksImg);
				tft_println("%d %d", stepper_get_vel(STEPPER_1), stepper_get_params(STEPPER_1)->countsBetweenPulses);
				tft_println("%d %d", stepper_get_vel(STEPPER_2), stepper_get_params(STEPPER_2)->countsBetweenPulses);
				tft_println("");
				tft_println("%d %d", stepper_get_count(STEPPER_1), stepper_get_count(STEPPER_2));
				tft_update();
			}
			
			if (ticksImg % 500 == 0) {
				led_toggle(LED_A);
				led_toggle(LED_B);
			}
		}
	}
}
