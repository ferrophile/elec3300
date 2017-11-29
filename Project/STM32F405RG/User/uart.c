#include "uart.h"

static void uart_gpio_init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);
}

void uart_init() {
	uart_gpio_init();
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART3, &USART_InitStruct);
	
	//USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART3, ENABLE);
}

void uart_tx_byte(u8 byte) {
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	USART_SendData(USART3, byte);
}

void uart_tx_word(u16 word) {
	uart_tx_byte((u8)(word & 0xFF00) >> 8);
	uart_tx_byte((u8)(word & 0x00FF));
}

void uart_tx_dword(u32 dword) {
	uart_tx_byte((u8)(dword & 0xFF000000) >> 24);
	uart_tx_byte((u8)(dword & 0x00FF0000) >> 16);
	uart_tx_byte((u8)(dword & 0x0000FF00) >> 8);
	uart_tx_byte((u8)(dword & 0x000000FF));
}
