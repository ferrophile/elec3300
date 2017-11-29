#ifndef UART_H
#define UART_H

#include "stm32f4xx.h"
#include <stdarg.h>
#include <stdio.h>

void uart_init(void);
void uart_tx_byte(u8 byte);
void uart_tx_word(u16 word);
void uart_tx_dword(u32 dword);
void uart_tx_float(float num);

void uart_tx_text(const char * data, ...);

#endif
