#ifndef LASER_H
#define LASER_H

#include "stm32f4xx.h"
#include "ticks.h"

void laser_init(void);
u32 laser_get_distance(void);

#endif
