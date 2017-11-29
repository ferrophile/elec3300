#include "scanner.h"

static void (*curState)(void);
u16 heightCnt;

static void scanner_standby() {
}

static void scanner_rotate_plate() {
	if (stepper_is_idle(PLATE_STEPPER)) {
		stepper_set_handler(PLATE_STEPPER, 0);
		stepper_set_deg(LASER_STEPPER, 200, 360);
		curState = scanner_inc_height;
	}
}

static void scanner_inc_height() {
	if (stepper_is_idle(LASER_STEPPER)) {
		heightCnt++;
		stepper_set_handler(PLATE_STEPPER, scanner_math_handler);
		stepper_set_deg(PLATE_STEPPER, 25, 360);
		curState = scanner_rotate_plate;
	}
}

static void scanner_start_btn_handler() {
	if (curState == scanner_standby) {
		scanner_stl_write_header(400);
		heightCnt = 0;
		
		stepper_set_handler(PLATE_STEPPER, scanner_math_handler);
		stepper_set_deg(PLATE_STEPPER, 25, 360);
		curState = scanner_rotate_plate;
	} else {
		stepper_set_vel(LASER_STEPPER, 0);
		stepper_set_vel(PLATE_STEPPER, 0);
		curState = scanner_standby;
	}
}

static void scanner_lower_btn_handler() {
	static u8 btnState = 0;
	
	if (curState == scanner_standby) {
		if (btnState == 0)
			stepper_set_vel(LASER_STEPPER, -400);
		if (btnState == 1)
			stepper_set_vel(LASER_STEPPER, -600);
		if (btnState == 2)
			stepper_set_vel(LASER_STEPPER, -800);
		if (btnState == 3)
			stepper_set_vel(LASER_STEPPER, 0);
		btnState = (btnState + 1) % 4;
	}
}

void scanner_init() {
	button_set_handler(BUTTON_1, scanner_start_btn_handler);
	button_set_handler(BUTTON_2, scanner_lower_btn_handler);
	
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
	tft_update();
}
