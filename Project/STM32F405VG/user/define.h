#ifndef _DEFINE_H
#define _DEFINE_H

#define LOOP1_MS 20
#define LOOP2_MS 200

/**
	PIN_ON_TOP = 		0
	PIN_ON_LEFT = 	1
	PIN_ON_BOTTOM =	2
	PIN_ON_RIGHT = 	3
*/
#define ORIENTATION_SETTING 2

#ifndef ARM_MATH_CM4
	#define ARM_MATH_CM4
#endif

#define CLIMBING_PNEU 				PNEU_1
#define SHOOTING_PNEU 				PNEU_2
#define LAUGHING_PNEU 				PNEU_3

#define BUTTON_1							BOARD_BTN_1
#define BUTTON_2							BOARD_BTN_2
#define JOYSTICK_N 						JS_BTN_A
#define JOYSTICK_E 						JS_BTN_B
#define JOYSTICK_S 						JS_BTN_C
#define JOYSTICK_W 						JS_BTN_D
#define JOYSTICK_M						JS_BTN_M

#define WALL_LIMIT_SWITCH 		LIM_SW_1

#define LIFTING_SERVO 				SERVO1
#define CANNOT_LIFT_SERVO 		SERVO3

#endif 
