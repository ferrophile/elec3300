#ifndef SCANNER_H
#define SCANNER_H

#include "ticks.h"
#include "uart.h"
#include "button.h"
#include "lcd_main.h"

#include "stepper.h"
#include "laser.h"

#define LASER_STEPPER STEPPER_1
#define PLATE_STEPPER STEPPER_2
#define LASER ADC_PORT_1

typedef struct {
	s32 x;
	s32 y;
	s32 z;
} Vector3D;

extern u16 heightCnt;

static void scanner_standby(void);
static void scanner_rotate_plate(void);
static void scanner_inc_height(void);

void scanner_init(void);
void scanner_run(void);
void scanner_show(void);

void scanner_stl_write_header(u32 trigCnt);
void scanner_stl_write_face(Vector3D * nml, Vector3D (* verts)[3]);

void scanner_math_handler(void);

#endif
