/*
 * HKUST Robocon 2017
 * Wheelbase Debugger
 *
 * Hong Wing PANG (hwpang)
 * Jul 2017
 */

#include "main.h"

void (*curState)(void);

extern u32 curSteps[2];
u32 ticksImg = 0;

void scanner_standby();
void scanner_rotate_plate();
void scanner_inc_height();

void scanner_standby() {
}

void scanner_rotate_plate() {
	if (curSteps[1] == 0) {
		stepper_rotate(STEPPER_1, 200, 360);
		curState = scanner_inc_height;
	}
}

void scanner_inc_height() {
	if (curSteps[0] == 0) {
		stepper_rotate(STEPPER_2, 25, 360);
		curState = scanner_rotate_plate;
	}
}

void scanner_start_btn_handler() {
	if (curState == scanner_standby) {
		stepper_rotate(STEPPER_2, 25, 360);
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
	gpio_rcc_init_all();
	
	ticks_init();
	tft_init(0, BLACK, WHITE, RED);
	led_init();
	buzzer_init();
	btn_init();
	
	laser_init();
	stepper_init();
	btn_reg_OnClickListener(BUTTON_1, scanner_start_btn_handler);
	btn_reg_OnClickListener(BUTTON_2, scanner_lower_btn_handler);
	
	curState = scanner_standby;
	
	while(1) {
		if (get_ticks() != ticksImg) {
			ticksImg = get_ticks();
			
			if (ticksImg % 20 == 0) {				
				tft_clear();
				tft_println("Time: %d", ticksImg);
				tft_println("Vel:  %d %d", stepper_get_vel(STEPPER_1), stepper_get_vel(STEPPER_2));
				tft_println("Dist: %d", laser_get_distance());
				tft_println("%d %d", curSteps[0], curSteps[1]);
				
				if (curState == scanner_standby)
					tft_println("Standby");
				else
					tft_println("Scanning");
				
				tft_update();
				btn_update();
				(* curState)();
			}
		}
	}
}
