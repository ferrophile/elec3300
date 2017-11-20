#include "main.h"

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	
	ticks_init();
	stepper_init();
	
	while(1) {
		
	}
}
