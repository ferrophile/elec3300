#include "uart.h"

static void uart_gpio_init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void uart_init() {
	uart_gpio_init();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &USART_InitStruct);
	USART_Cmd(USART1, ENABLE);
}

void uart_tx_byte(u8 byte) {
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, byte);
}

// Use little-endian
void uart_tx_word(u16 word) {
	uart_tx_byte((u8)(word & 0x00FF));
	uart_tx_byte((u8)((word & 0xFF00) >> 8));
}

void uart_tx_dword(u32 dword) {
	uart_tx_byte((u8)(dword & 0x000000FF));
	uart_tx_byte((u8)((dword & 0x0000FF00) >> 8));
	uart_tx_byte((u8)((dword & 0x00FF0000) >> 16));
	uart_tx_byte((u8)((dword & 0xFF000000) >> 24));
}

void uart_tx_float(float num) {
	u32 * cast = (u32 *) &num;
	uart_tx_dword(*cast);
}

void uart_tx_text(const char * data, ...) {
	va_list arglist;
	uint8_t buf[255], *fp;
	
	va_start(arglist, data);
	vsprintf((char*)buf, data, arglist);
	va_end(arglist);
	
	fp = buf;
	while (*fp){
		uart_tx_byte(*fp++);
	}
}
