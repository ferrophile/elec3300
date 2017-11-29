#include "main.h"

u32 ticksImg = 0;

/*
u8 buffer[5] = {0};

void test() {
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, 'E');
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, CMD_CR);
}

void read() {
	static u8 i = 0;
	u8 data = USART_ReceiveData(USART2);
	if (data != 0 && i < 5) {
		buffer[i] = data;
		i++;
	}
}
*/

int main(void) {
	SystemInit();
	SystemCoreClockUpdate();
	
	led_init();
	ticks_init();
	button_init();
	stepper_init();
	tft_init(PIN_ON_LEFT, BLACK, WHITE, RED);
	uart_init();
	
	led_on(LED_A);
	led_off(LED_B);
	
	scanner_init();
	//button_set_handler(SW_MIDDLE, test);
	
	while(1) {
		if (ticksImg != get_ticks()) {
			ticksImg = get_ticks();
			
			if (ticksImg % 20 == 0) {
				tft_clear();
				tft_println("%d", ticksImg);
				tft_println("%d %d %d", stepper_get_vel(STEPPER_1), stepper_get_params(STEPPER_1)->countsBetweenPulses, stepper_get_params(STEPPER_1)->targetStepCount);
				tft_println("%d %d %d", stepper_get_vel(STEPPER_2), stepper_get_params(STEPPER_2)->countsBetweenPulses, stepper_get_params(STEPPER_2)->targetStepCount);
				tft_println("");
				tft_println("%d %d", stepper_get_count(STEPPER_1), stepper_get_count(STEPPER_2));
				
				if (scanner_is_scanning())
					tft_println("Scanning");
				else
					tft_println("Standby");
				
				//tft_println("%d %d %d %d %d", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
				tft_update();
				scanner_run();
				//read();
			}
			
			if (ticksImg % 500 == 0) {
				led_toggle(LED_A);
				led_toggle(LED_B);
			}
		}
	}
}
