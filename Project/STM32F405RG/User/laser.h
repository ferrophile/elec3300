#ifndef LASER_H
#define LASER_H

#include "stm32f4xx.h"
#include "ticks.h"

void laser_init(void);
u32 laser_get_distance(void);
u16 laser_get_raw_value(void);

#endif
