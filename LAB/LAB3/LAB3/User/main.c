#include "stm32f10x.h"
#include "lcd.h"
#include <stdio.h>

/*-----------------------------------------------------------------------------------------------
LAB 3: COLOR LCD INTERFACING

Task 1 – Use either LCD_DrawChar or LCD_DrawString procedures in main.c to write your name on the LCD. 
Task 2 – Implement the LCD_DrawDot to turn on a particular dot on the LCD.
Task 3 – Implement LCD_DrawCircle by using LCD_DrawDot
Task 4 – Program display white background color at the beginning. After K2 pressed, it will display Olympic Sign

Before writing the function, please make sure in stm32f10x_conf.h, the following #include lines are uncommented
			"stm32f10x_fsmc.h"
			"stm32f10x_gpio.h" 
			"stm32f10x_rcc.h" 
			"misc.h" 
------------------------------------------------------------------------------------------------*/

void Delayus(int duration);


int main ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	LCD_INIT ();        
	/*
	 *  Task 1 – Use either DrawChar or DrawString procedures in main.c to write your name on the LCD.
	 */
	LCD_DrawString(0, 0, "Hong Wing PANG");

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	while ( 1 ){
	/*
	 *  Task 4 – Program display white background color at the beginning. After K2 pressed, it will display Olympic Sign
	 */
		static int isNotPressed = 1;
		if ((GPIOC->IDR & GPIO_Pin_13) && isNotPressed) {
			isNotPressed = 0;
			LCD_DrawCircle(60, 160, 30, BLUE);
			LCD_DrawCircle(90, 190, 30, YELLOW);
			LCD_DrawCircle(120, 160, 30, BLACK);
			LCD_DrawCircle(150, 190, 30, GREEN);
			LCD_DrawCircle(180, 160, 30, RED);
		}
	}
}

void Delayus(int duration)
{
		while(duration--) 
		{
			int i=0x02;				
			while(i--)
			__asm("nop");
		}
}


/* ------------------------------------------end of file---------------------------------------- */

