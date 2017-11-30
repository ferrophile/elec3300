#ifndef ADC_PORT_H
#define ADC_PORT_H

#include "stm32f4xx.h"

// Laser ID
enum {
	ADC_PORT_1,
	ADC_PORT_2,
	ADC_PORT_3,
	ADC_PORT_4
};

void laser_init(void);
u16 laser_get_reading(u8 id);
u16 laser_get_raw_value(void);

#endif
