#include "laser.h"

const u32 voltToDistSlope = 30725;
const u32 voltToDistSlopePresc = 10000;
const u32 voltToDistOffset = 100;
const u32 voltToDistCutoff = 2880;

void laser_init(void) {
	adc_init();
}

u32 laser_get_distance(void) {
	u32 reading = get_adc(ADC_PORT_1);
	if (reading > voltToDistCutoff)
		return 0;
	
	reading = reading * voltToDistSlopePresc / voltToDistSlope;
	reading += voltToDistOffset;
	
	return reading;
}
