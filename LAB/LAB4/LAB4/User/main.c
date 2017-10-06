#include "stm32f10x.h"
/*------------------------------------------------------------
LAB 4: TIMER AND PWM FUNCTIONS

Task 1 – Output SYSCLK via MCO.
Task 2 – Generate a student ID dependent PWM at TIM3_CH1, using SYSCLK as input source.
Task 3 – Generate a student ID dependent PWM at TIM4_CH1, using TIM3 as input source.  
Task 4 – You need to change the optimization to see if you output a different wave.
Task 5 – Generate a 700Hz sound by modifying Task 3.
Task 6 – Control a SG90 servo motor by K1 and K2.

Before writing the function, please make sure "stm32f10x_gpio.h", "stm32f10x_rcc.h", "stm32f10x_tim.h", #include "misc.h" 
located in stm32F10x_conf.h are uncommented, you will be using it.

-------------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;
TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;
void MCO_CONFIG (void);
void KEY_CONFIG (void);
void Delayus(int duration);

int main(void)
{	
	//MCO_CONFIG();
	KEY_CONFIG();
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = 720;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	TIM_Cmd(TIM3, ENABLE);
	
	/*
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 72;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	*/
	
	TIM_SelectMasterSlaveMode(TIM3, TIM_MasterSlaveMode_Enable);
	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
	
	TIM_TimeBaseStructure.TIM_Period = 2000;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 150;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	
	TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Gated);
	TIM_SelectInputTrigger(TIM4, TIM_TS_ITR2);
	
	TIM_Cmd(TIM4, ENABLE);
	
	while( 1 )
	{
		if (GPIOA->IDR & GPIO_Pin_0) {
			if (GPIOC->IDR & GPIO_Pin_13) {
				//Both pressed
				TIM_SetCompare1(TIM4, 150);
			} else {
				//K1 Pressed only
				TIM_SetCompare1(TIM4, 117);
			}
		} else {
			if (GPIOC->IDR & GPIO_Pin_13) {
				//K2 Pressed only
				TIM_SetCompare1(TIM4, 183);
			}
		}		
	}
}

void MCO_CONFIG (void)
{		
	/*
	 *  Task 1 – Output SYSCLK via MCO.
	 */
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	RCC->CFGR |= (RCC_MCO_SYSCLK << 24);
}

void KEY_CONFIG (void)
{		
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
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
