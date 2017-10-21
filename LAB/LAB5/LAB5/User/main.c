#include "stm32f10x.h"
#include "lcd.h"
#include <stdio.h>
/*------------------------------------------------------------
LAB 5: ADC FUNCTIONS

Task 1 – Display Single ADC Conversion result on LCD when K1 is pressed.
Task 2 – Display Continuous Conversion ADC results on LCD.
Task 3 – LDR Measurements.
Task 4 – EMG Sensor Measurements.

Before writing the function, please make sure in stm32f10x_conf.h, the following #include lines are uncommented
			"stm32f10x_gpio.h"
			"stm32f10x_fsmc.h" 
			"stm32f10x_rcc.h" 
			"stm32f10x_adc.h"
			"misc.h" 
-------------------------------------------------------------*/

void Delayus(int duration);
GPIO_InitTypeDef GPIO_InitStructure;
ADC_InitTypeDef ADC_InitStructure;

char buffer[50];

void GPIO_Configuration(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void ADC_Configuration(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 1, ADC_SampleTime_55Cycles5);
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
}

int main(void)
{	
		
	LCD_INIT ();      
	/* Task 1 – Display Single ADC Conversion result on LCD when K1 is pressed. 					*/
	/* Initialization Part																																*/
	/* a.	Configure the PC.4 as analogue input (Refer to Tutorial 5)											*/
	/* b.	Configure K1 and input (Refer to LAB2 and Tutorial 2)														*/
	/* c.	Configure ADC1 to get input from PC.4 																					*/
	/*		Below is the ADC Setting																												*/
	/*		Mode:	Independent																																*/
	/*		Scan Conversion Mode:	Disable																										*/
	/* 		Continuous Conversion Mode:	Disable																							*/
	/*		External Trigger Conversion:	None																							*/
	/*		Data Alignment: Right																														*/
	/*		Number of Channel: 1																														*/
	/* d.	Configure ADC Clock, please refer to the tutorial notes.												*/
	/* e.	Connect Channel 14 to ADC1 and enable ADC1, please refer to the tutorial notes. */
	/* f.	Do a self-calibration on ADC1, please refer to the tutorial notes. 							*/
	GPIO_Configuration();
	ADC_Configuration();
	LCD_DrawString(10, 10, "The ADC value measured:");
	
	while ( 1 ){
	/* Implementation Part																																*/
	/* a.	When K1 is pressed, start a software conversion.																*/
	/* b.	Wait for conversion time, after that get the conversion results.								*/
	/* c.	Display the conversion result on LCD, you may need to refer to LAB3 for display */
		//if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);
			Delayus(1);
			sprintf(buffer, "0x%X", ADC_GetConversionValue(ADC1));
		//}
		LCD_DrawString(10, 26, buffer);


	/* Task 2 – Display Continuous Conversion ADC results on LCD.													*/
	/* Change your program such that the LCD will be able to update the result at a 			*/
	/* certain period without pressing K1 																								*/
		
		
	/* Task 3 – LDR Measurements. Refer to LAB Sheet																			*/
	/* Task 4 – EMG Sensor Measurements. Refer to LAB Sheet																*/
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
