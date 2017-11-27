#include "main.h"

u32 ticksImg = 0;
void (*curState)(void);

void scanner_standby();
void scanner_rotate_plate();
void scanner_inc_height();

void scanner_standby() {
}

void scanner_rotate_plate() {
	if (stepper_is_idle(STEPPER_2)) {
		stepper_set_deg(STEPPER_1, 200, 360);
		curState = scanner_inc_height;
	}
}

void scanner_inc_height() {
	if (stepper_is_idle(STEPPER_1)) {
		stepper_set_deg(STEPPER_2, 25, 360);
		curState = scanner_rotate_plate;
	}
}

void scanner_start_btn_handler() {
	if (curState == scanner_standby) {
		stepper_set_deg(STEPPER_2, 25, 360);
		curState = scanner_rotate_plate;
	} else {
		stepper_set_vel(STEPPER_1, 0);
		stepper_set_vel(STEPPER_2, 0);
		curState = scanner_standby;
	}
}

void scanner_lower_btn_handler() {
	static u8 btnState = 0;
	
	if (curState == scanner_standby) {
		if (btnState == 0)
			stepper_set_vel(STEPPER_1, -400);
		if (btnState == 1)
			stepper_set_vel(STEPPER_1, -600);
		if (btnState == 2)
			stepper_set_vel(STEPPER_1, -800);
		if (btnState == 3)
			stepper_set_vel(STEPPER_1, 0);
		btnState = (btnState + 1) % 4;
	}
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
	
	button_set_handler(BUTTON_1, scanner_start_btn_handler);
	button_set_handler(BUTTON_2, scanner_lower_btn_handler);
	
	curState = scanner_standby;
	
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
				
				if (curState == scanner_standby)
					tft_println("Standby");
				else
					tft_println("Scanning");
				
				tft_println("%X", TIM3->CCMR1);
				tft_update();
				(* curState)();
			}
			
			if (ticksImg % 500 == 0) {
				led_toggle(LED_A);
				led_toggle(LED_B);
			}
		}
	}
}
