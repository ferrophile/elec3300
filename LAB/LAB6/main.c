#include "stm32f10x.h"
#include "lcd.h"
#include "i2c_GY80.h"
#include "math.h"
#include <stdio.h>

/*------------------------------------------------------------
LAB 6: I2C APPLICATION ON GY80 

Task 1 – Display Compass Result on LCD
Task 2 – Build your 7-segment display circuit and display the last digit on the 7-segment display

Before writing the function, please make sure in stm32f10x_conf.h, the following #include lines are uncommented
			"stm32f10x_fsmc.h" 
			"stm32f10x_gpio.h"
			"stm32f10x_i2c.h"
			"stm32f10x_rcc.h" 
			"misc.h" 
-------------------------------------------------------------*/

#define HMC5883L_Addr	0x3C
#define L3G4200_Addr	0xD2
#define BMP085_Addr	  0xEE
#define ADXL345_Addr	0xA6

#define I2C_SLAVE_ADDRESS7 0x3C	//compass
#define I2C_Speed	GPIO_Speed_50MHz


/* Private function prototypes -----------------------------------------------*/
void HMC_Init(void);
void Delayus(int duration);
void GPIO_Configuration(void);

char buffer[50];
s16 x,y,z;
s16 result;
u8 i = 0;

//A-G in order
u8 char_map[10][7] = {
	{1, 1, 1, 1, 1, 1, 0},
	{0, 1, 1, 0, 0, 0, 0},
	{1, 1, 0, 1, 1, 0, 1},
	{1, 1, 1, 1, 0, 0, 1},
	{0, 1, 1, 0, 0, 1, 1},
	{1, 0, 1, 1, 0, 1, 1},
	{1, 0, 1, 1, 1, 1, 1},
	{1, 1, 1, 0, 0, 0, 0},
	{1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 0, 1, 1}};

GPIO_TypeDef * ports[7] = {GPIOC, GPIOC, GPIOC, GPIOC, GPIOB, GPIOB, GPIOB};
u32 pins[7] = {GPIO_Pin_8, GPIO_Pin_9, GPIO_Pin_10, GPIO_Pin_11, GPIO_Pin_8, GPIO_Pin_9, GPIO_Pin_12};
	
int main(void)
{
	I2C_InitTypeDef  I2C_InitStructure; /* I2C configuration */
	GPIO_InitTypeDef  GPIO_InitStructure; 
	
	RCC_APB1PeriphClockCmd(I2C_GY80_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(I2C_GY80_GPIO_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_12;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  LCD_INIT(); 						// LCD_INIT
  I2C_GY80_Init();        // I2C Init
  HMC_Init();             // HMC Init
	
	GPIO_Configuration();
	
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;/* I2C Peripheral Enable */
	I2C_Cmd(I2C_GY80, ENABLE);/* Apply I2C configuration after enabling it */
	I2C_Init(I2C_GY80, &I2C_InitStructure);
	
	
	
  while (1) {
	/* Please add code below to complete the LAB6 */
  /* You might want to create your own functions */	
		I2C_ByteWrite(0x3C, 0x02, 0x01);
		Delayus(6000); 
		
		x = (I2C_ByteRead(0x3C, 0x03)<< 8) + I2C_ByteRead(0x3C, 0x04);
		z = (I2C_ByteRead(0x3C, 0x05)<< 8) + I2C_ByteRead(0x3C, 0x06);
		y = (I2C_ByteRead(0x3C, 0x07)<< 8) + I2C_ByteRead(0x3C, 0x08);
		
		
		result = (int)(atan2(y,x)*180 /62832*20000) ;
		if(result <= 0)
		{
			result += 360;
		}
		
		sprintf(buffer, "%d %d %d   %d" ,x, y, z, result);
		LCD_DrawString(0, 0, "                           ");
		LCD_DrawString(0, 0, buffer);
		
		for (i = 0; i < 7; i++) {
			if (char_map[result % 10][i])
				GPIO_ResetBits(ports[i], pins[i]);
			else
				GPIO_SetBits(ports[i], pins[i]);
		}
		
  }
}



void HMC_Init(void)
{
  I2C_ByteWrite(HMC5883L_Addr, 0x00, 0x70);
  I2C_ByteWrite(HMC5883L_Addr, 0x01, 0xA0);
  Delayus(10000);
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

