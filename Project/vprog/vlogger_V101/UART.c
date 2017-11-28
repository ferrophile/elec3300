#include <system.h>

#include "UART.h"

//***************************************************************************
//  
//  Constants and variables for UART
//  
//***************************************************************************

#define PORT_RX   portb.1				// RX is RB1 (Port B bit 1)
#define TRIS_RX   trisb.1

#define PORT_RTS  portb.3				// RTS# is RB3 (Port B bit 3)
#define TRIS_RTS  trisb.3

#define PORT_TX   portb.4				// TX is RB4 (Port B bit 4)
#define TRIS_TX	  trisb.4

#define PORT_CTS  portb.2				// CTS# is RB2 (Port B bit 2)
#define TRIS_CTS  trisb.2


// use a circular buffer for UART incoming data
#define UART_BUFFER_SIZE 16
unsigned char uartData[UART_BUFFER_SIZE];
unsigned char *uartHead, *uartTail;
unsigned char uartCount;
// definitions of start and end of buffer
#define uartStart &uartData[0]
#define uartEnd &uartData[UART_BUFFER_SIZE]
// upper limit must be 4 or more bytes before buffer size
#define UART_BUFFER_UPPER_LIMIT ((UART_BUFFER_SIZE*3)/4) 
#define UART_BUFFER_LOWER_LIMIT (UART_BUFFER_SIZE/4)
// enable/disable flow control on CTS/RTS
#define UART_CTS_RTS_ENABLED 1

//***************************************************************************
//
// Internal Routines
//
//***************************************************************************



//***************************************************************************
//
// External Routines
//
//***************************************************************************

//***************************************************************************
// Name: uartRx
//
// Description: Receive data using circular FIFO buffer. 
//
// Parameters: None.
//
// Returns: None.
//
// Comments: Must only be called from interrupt() routine.
//
//***************************************************************************
void uartRx()
{
	// clear UART interrupt bit
	pir1.RCIF = 0;

	// if using flow control
	if (UART_CTS_RTS_ENABLED)								
	{
		// check for high threshold for FIFO
		if (uartCount >= UART_BUFFER_UPPER_LIMIT)				
		{
			// set RTS# inactive (high)
			PORT_RTS = 1;									
		}
	}

	// if FIFO is not already full
	if (uartCount < UART_BUFFER_SIZE)						
	{
		// add received byte to FIFO and then increment head pointer
		*uartHead++ = rcreg;							
		// increment byte count
		uartCount++;								

		// wrap head pointer if greater than allocated FIFO end address
		if (uartHead == uartEnd)					
		{
			uartHead = uartStart;
		}	
	}
}

//***************************************************************************
// Name: uartInit
//
// Description: Initialise the UART interface. 
//
// Parameters: None.
//
// Returns: None.
//
// Comments: Sets up pins connecting to the UART interface.
//
//***************************************************************************
void uartInit()
{
	// disable uart mode
	rcsta.SPEN = 0;

	// disable AN5 and AN6 to allow use as digital intputs on RB1 and RB4
	adcon1.PCFG5 = 1;
	adcon1.PCFG6 = 1;

    // configure pin direction (1 for input, 0 for output)
	TRIS_RTS = 0;		// RTS output
	PORT_RTS = 1;		// Set RTS# inactive (high)
	TRIS_CTS = 1;		// CTS input
	TRIS_TX = 1;		// TX input
	TRIS_RX = 1;		// RX input

	// BRG16 = 1
	baudctl.BRG16 = 1;
	// BRGH = 1, SYNC = 0, TXEN = 1
	txsta.SYNC = 0;
	txsta.BRGH = 1;
	txsta.TXEN = 1;
	// 9600 baud
	spbrg = 207;
	spbrgh = 0;

	// configure initial pin states
	// RTS starts low
	PORT_RTS = 0;

	// initialise circular buffer state
	uartHead = uartTail = uartStart;
	uartCount = 0;

	// enable uart mode
	rcsta.SPEN = 1;
	rcsta.CREN = 1;

	// enable UART interrupts
	pie1.RCIF = 0;
	pie1.RCIE = 1;
}

//***************************************************************************
// Name: uartReadWait
//
// Description: Blocking read of character from UART. 
//
// Parameters: None.
//
// Returns: uartData - Byte received.
//
// Comments: Waits until a character is read from the UART bus and returns.
//
//***************************************************************************
char uartReadWait()
{
	char uartData;

	while (uartRead(&uartData) == XFER_RETRY);

	return uartData;
}

//***************************************************************************
// Name: uartRead
//
// Description: Non-blocking read of character from UART bus. 
//
// Parameters: None.
//
// Returns: pUartData - Byte received.
//          int XFER_OK if data received, XFER_RETRY if not.
//
// Comments: Check for a character on the UART bus and return.
//
//***************************************************************************
int uartRead(char *pUartData)
{
	// if FIFO is empty return retry flag
	if (uartCount == 0)
	{
		*pUartData = 0xFE;
		return XFER_RETRY;
	}

	// disable UART interrupts
	pie1.RCIE = 0;										

	// read a byte from the FIFO at position uartTail and then inc pointer
	*pUartData = *uartTail++;
	// decrement byte count	
	uartCount--;										
	
	// wrap if tail pointer is greater than allocated FIFO end address
	if (uartTail == uartEnd)					
	{
		uartTail = uartStart;
	}
	
	// re-enable UART interrupts
	pie1.RCIE = 1;										

	// if using flow control
	if (uartCount < UART_BUFFER_LOWER_LIMIT)
	{
		if (UART_CTS_RTS_ENABLED)								
		{
			// Set RTS# active (low)
			PORT_RTS = 0;										
		}
	}

	// return success flag
	return XFER_OK;
}

//***************************************************************************
// Name: uartWrite
//
// Description: Blocking write of character to UART bus. 
//
// Parameters: uartData - Byte to be transmitted.
//
// Returns: None.
//
// Comments: Waits until a character is transmitted on the UART bus.
//
//***************************************************************************
void uartWrite(char uartData)
{
	// wait for transmission to finish
	while (!txsta.TRMT);

	// if using flow control
	if (UART_CTS_RTS_ENABLED)								
	{
		// wait if CTS is not clear
		while (PORT_CTS);
	}

	// send data byte
	txreg = uartData;
}

