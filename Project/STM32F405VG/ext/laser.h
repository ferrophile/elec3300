#ifndef __LASER_H
#define __LASER_H

#include "stm32f4xx.h"
#include "ticks.h"
#include "adc.h"

void laser_init(void);
u32 laser_get_distance(void);

#endif
