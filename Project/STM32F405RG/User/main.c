#include "main.h"

u32 ticksImg = 0;

void scanner_up_handler() {
	stepper_set_vel(STEPPER_1, stepper_get_vel(STEPPER_1) + 100);
}

void scanner_down_handler() {
	stepper_set_vel(STEPPER_1, stepper_get_vel(STEPPER_1) - 100);
}

void scanner_left_handler() {
	stepper_set_deg(STEPPER_1, -100, 360);
}

void scanner_right_handler() {
	stepper_set_deg(STEPPER_1, 100, 360);
}

void scanner_middle_handler() {
	stepper_set_vel(STEPPER_1, 0);
}

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	
	led_init();
	ticks_init();
	button_init();
	stepper_init();
	tft_init(PIN_ON_LEFT, BLACK, WHITE, RED);
	
	led_on(LED_A);
	led_off(LED_B);
	
	button_set_handler(SW_UP, scanner_up_handler);
	button_set_handler(SW_DOWN, scanner_down_handler);
	button_set_handler(SW_LEFT, scanner_left_handler);
	button_set_handler(SW_RIGHT, scanner_right_handler);
	button_set_handler(SW_MIDDLE, scanner_middle_handler);
	
	while(1) {
		if (ticksImg != get_ticks()) {
			ticksImg = get_ticks();
			
			if (ticksImg % 20 == 0) {
				tft_clear();
				tft_println("%d", ticksImg);
				tft_println("%d %d %d", stepper_get_vel(STEPPER_1), stepper_get_params(STEPPER_1)->countsBetweenPulses, stepper_get_params(STEPPER_1)->targetStepCount);
				tft_println("%d %d %d", stepper_get_vel(STEPPER_2), stepper_get_params(STEPPER_2)->countsBetweenPulses, stepper_get_params(STEPPER_2)->targetStepCount);
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
