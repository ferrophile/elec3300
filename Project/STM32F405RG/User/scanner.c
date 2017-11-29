#include "scanner.h"

static void (*curState)(void);

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
		scanner_stl_write_header(0);
		stepper_set_deg(PLATE_STEPPER, 25, 360);
		curState = scanner_rotate_plate;
		heightCnt = 0;
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

u8 scanner_is_scanning() {
	return (u8)(curState != scanner_standby);
}
