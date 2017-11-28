#ifndef UART_H
#define UART_H

#define monInit uartInit
#define monRead uartRead
#define monReadWait uartReadWait
#define monWrite uartWrite
#define monInterrupt pir1.RCIF
#define monInterruptHandler uartRx

#define XFER_OK    0
#define XFER_RETRY 1

void uartInit();
char uartReadWait();
int uartRead(char *pUartData);
void uartWrite(char uartData);
void uartRx(); // interrupt routine

#define UART_RX_FIFO_OVERRUN 0x01

#endif