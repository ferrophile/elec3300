#include "scanner.h"

static void (*curState)(void);
u16 heightCnt;
u16 totalHeight;
#define MAX_SPEED 3000

static void scanner_standby() {
	
	static s16 speed = 0;
	if (button_get_state(SW_UP) == 0 && speed < MAX_SPEED) {
		speed += 50;
		stepper_set_vel(LASER_STEPPER, speed);
	}
	
	if (button_get_state(SW_DOWN) == 0 && speed > -MAX_SPEED) {
		speed -= 50;
		stepper_set_vel(LASER_STEPPER, speed);
	}
	
	if (button_get_state(SW_UP) && button_get_state(SW_DOWN)) {
		speed = 0;
		stepper_set_vel(LASER_STEPPER, speed);
	}
}

static void scanner_get_height() {
	if (stepper_is_idle(LASER_STEPPER)) {
		heightCnt++;
		if (laser_get_reading(LASER) > 3000) {
			totalHeight = heightCnt - 1;
			scanner_stl_write_header(400 * totalHeight);
			stepper_set_deg(LASER_STEPPER, -400, 360);
			curState = scanner_dec_height;
		} else {
			stepper_set_deg(LASER_STEPPER, 400, 360);
		}
	}
}

static void scanner_rotate_plate() {
	if (stepper_is_idle(PLATE_STEPPER)) {
		if (heightCnt) {
			stepper_set_handler(PLATE_STEPPER, 0);
			stepper_set_deg(LASER_STEPPER, -400, 360);
			curState = scanner_dec_height;
		} else {
			stepper_set_vel(LASER_STEPPER, 0);
			stepper_set_vel(PLATE_STEPPER, 0);
			curState = scanner_standby;
		}
	}
}

static void scanner_dec_height() {
	if (stepper_is_idle(LASER_STEPPER)) {
		heightCnt--;
		
		stepper_set_handler(PLATE_STEPPER, scanner_math_handler);
		stepper_set_deg(PLATE_STEPPER, 50, 360);
		curState = scanner_rotate_plate;
	}
}

static void scanner_start_btn_handler() {
	if (curState == scanner_standby) {
		heightCnt = 0;
		totalHeight = 0;
		
		stepper_set_deg(LASER_STEPPER, 400, 360);
		curState = scanner_get_height;
	} else {
		stepper_set_vel(LASER_STEPPER, 0);
		stepper_set_vel(PLATE_STEPPER, 0);
		curState = scanner_standby;
	}
}

void scanner_init() {
	button_set_handler(SW_MIDDLE, scanner_start_btn_handler);
	
	curState = scanner_standby;
}

void scanner_run() {
	(* curState)();
}

void scanner_show() {
	tft_clear();
	tft_println("%d", get_ticks());
	tft_println("%d %d %d", stepper_get_vel(STEPPER_1), stepper_get_params(STEPPER_1)->countsBetweenPulses, stepper_get_params(STEPPER_1)->targetStepCount);
	tft_println("%d %d %d", stepper_get_vel(STEPPER_2), stepper_get_params(STEPPER_2)->countsBetweenPulses, stepper_get_params(STEPPER_2)->targetStepCount);
	tft_println("");
	tft_println("%d %d", stepper_get_count(STEPPER_1), stepper_get_count(STEPPER_2));
	tft_println(curState == scanner_standby ? "Standby" : "Scanning");
	tft_println("%d", heightCnt);
	tft_println("%d", laser_get_reading(ADC_PORT_1));
	tft_update();
}
