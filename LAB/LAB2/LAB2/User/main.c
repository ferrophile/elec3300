#include "stm32f10x.h"

/*------------------------------------------------------------
LAB 2: I/O INTERFACE, INTERRUPT FUNCTION OF ARM 

Task 1 to 3 are for initialization of the device. 

Task 1 – You need to configure the RGB LED as the output. 
Task 2 – You need to configure the 2 keys K1, K2, as the input.
Task 3 – You need to configure the interrupt function of K1.

Task 4 to 6 are implementation of the LAB functions.

Task 4 – Square wave generation and measurement
Task 5 – In the while(1) loop of main.c, comment the Task 4 code, and implement an 
         if else statement to check if K2 is pressed, it will light up Blue LED, else Blue LED will be turned off. 
Task 6 – Open the file stm32f10x_it.c, modify the function
         void EXTI0_IRQHandler(void) 
         so that whenever K1 is pressed, the Green LED will toggle.


Before writing the function, please make sure in stm32f10x_conf.h, the following #include lines are uncommented
			"stm32f10x_exit.h"
			"stm32f10x_gpio.h" 
			"stm32f10x_rcc.h" 
			"misc.h" 
-------------------------------------------------------------*/

void Delayus(int duration);
void RGBLED_CONFIG(void);
void EXTI_CONFIG(void);
void KEY_CONFIG(void);

int main()
{	
	RGBLED_CONFIG();
	KEY_CONFIG();
	EXTI_CONFIG();
	
	GPIOB->BSRR = GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1;
	while( 1 )
	{
		// Task 4
		
		/*
		GPIOB->BRR = GPIO_Pin_5;
		Delayus(1);
		GPIOB->BSRR = GPIO_Pin_5;
		Delayus(1);
		*/
		
		// Task 5
		
		if (GPIOC->IDR & GPIO_Pin_13) {
			GPIOB->BRR = GPIO_Pin_1;	//Turn on
		} else {
			GPIOB->BSRR = GPIO_Pin_1;	//Turn off
		}
	}
}

void RGBLED_CONFIG (void)
{		
	/* Task 1: Configure the RGB LED as output */
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void KEY_CONFIG (void)
{		
	/* Task 2: Configure K1 and K2 as input */
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void EXTI_CONFIG (void)
{
	/* Task 3:  Configure the interrupt function of K1 */
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	GPIO_EXTILineConfig(RCC_APB2Periph_GPIOA, GPIO_Pin_0);
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Delayus (int duration)
{
		while(duration--) 
		{
			int i=0x02;				
			while(i--)
			__asm("nop");
		}
}

