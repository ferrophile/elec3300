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
	float x;
	float y;
	float z;
} Vector3D;

extern u16 heightCnt;
extern u16 totalHeight;

static void scanner_standby(void);
static void scanner_get_height(void);
static void scanner_rotate_plate(void);
static void scanner_dec_height(void);

void scanner_init(void);
void scanner_run(void);
void scanner_show(void);

void scanner_stl_write_header(u32 trigCnt);
void scanner_stl_write_face(Vector3D * nml, Vector3D * vert1, Vector3D * vert2, Vector3D * vert3);

void scanner_math_handler(void);

#endif
