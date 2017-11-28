#include <system.h>

#include "SPI.h"

//***************************************************************************
//  
//  Pin definitions for SPI
//  
//***************************************************************************

// NOTE: pin names are relative to the VDIP2 device

#define PORT_SDI  portb.1				// SDI is RB1 (Port B bit 1)
#define TRIS_SDI  trisb.1

#define PORT_SDO  portb.2				// SDO is RB2 (Port B bit 2)
#define TRIS_SDO  trisb.2

#define PORT_SCLK portb.4				// SCLK is RB4 (Port B bit 4)
#define TRIS_SCLK trisb.4

#define PORT_CS   portb.3				// CS is RB3 (Port B bit 3)
#define TRIS_CS   trisb.3

//***************************************************************************
//  
//  Constants and variables for SPI
//  
//***************************************************************************

#define DIR_SPIWRITE 0
#define DIR_SPIREAD  1

//***************************************************************************
//
// Internal Routines
//
//***************************************************************************

int spiXfer(int spiDirection, char *pSpiData);

//***************************************************************************
// Name: spiDelay
//
// Description: Short delay.
//
// Parameters: None.
//
// Returns: None.
//
// Comments: Delay of zero instructions for maximum speed operation.
//           Delay serves as placeholder when processor speed will exceed
//           Vinculum's 12MHz SPI frequency limit.
//
//***************************************************************************
#define spiDelay()

//***************************************************************************
// Name: spiXfer
//
// Description: Read or write transfer on SPI bus.
//
// Parameters: spiDirection - DIR_SPIWRITE/DIR_SPIREAD
//             pSpiData - pointer to data byte to write or read to
//
// Returns: retData - XFER_OK for success, XFER_RETRY to retry.
//
// Comments: Bit bangs SPI interface pins.
//
//***************************************************************************
int spiXfer(int spiDirection, char *pSpiData)
{
	unsigned char retData;
	unsigned char bitData;
	
	// CS goes high to enable SPI communications
	PORT_CS = 1;

	// Clock 1 - Start State
	PORT_SDI = 1;

	spiDelay();
	PORT_SCLK = 1;
	spiDelay();
	PORT_SCLK = 0;

	// Clock 2 - Direction
	PORT_SDI = spiDirection;

	spiDelay();
	PORT_SCLK = 1;
	spiDelay();
	PORT_SCLK = 0;

	// Clock 3 - Address
	PORT_SDI = 0;

	spiDelay();
	PORT_SCLK = 1;
	spiDelay();
	PORT_SCLK = 0;

	// Clocks 4..11 - Data Phase
	bitData = 0x80;
	if (spiDirection)
	{
		// read operation
		retData = 0;

		while (bitData)
		{
			spiDelay();
			retData |= PORT_SDO?bitData:0;
			PORT_SCLK = 1;
			spiDelay();
			PORT_SCLK = 0;
			bitData = bitData >> 1;
		}

		*pSpiData = retData;
	}
	else
	{
		// write operation
		retData = *pSpiData;

		while (bitData)
		{
			PORT_SDI = (retData & bitData)?1:0; 
			spiDelay();
			PORT_SCLK = 1;
			spiDelay();
			PORT_SCLK = 0;
			bitData = bitData >> 1;
		}
	}

	// Clock 12 - Status bit
	spiDelay();
	bitData = PORT_SDO;
	PORT_SCLK = 1;
	spiDelay();
	PORT_SCLK = 0;

	// CS goes high to enable SPI communications
	PORT_CS = 0;

	// Clock 13 - CS low
	spiDelay();
	PORT_SCLK = 1;
	spiDelay();
	PORT_SCLK = 0;

    return bitData;
}

//***************************************************************************
//
// External Routines
//
//***************************************************************************

//***************************************************************************
// Name: spiInit
//
// Description: Initialise the SPI interface. 
//
// Parameters: None.
//
// Returns: None.
//
// Comments: Sets up pins connecting to the SPI interface.
//
//***************************************************************************
void spiInit()
{
	// Configure pin direction (1 for input, 0 for output)
	TRIS_SDO = 1;		        // SDO input 			
	TRIS_SDI = 0;				// SDI output
	TRIS_SCLK = 0;				// SCLK output
	TRIS_CS = 0;				// CS output

	// Configure initial pin states

	// SDO starts low
	PORT_SDI = 0;
	// SCLK starts low
	PORT_SCLK = 0;
	// CS starts low
	PORT_CS = 0;
}

//***************************************************************************
// Name: spiReadWait
//
// Description: Blocking read of character from SPI bus. 
//
// Parameters: None.
//
// Returns: spiData - Byte received.
//
// Comments: Waits until a character is read from the SPI bus and returns.
//
//***************************************************************************
char spiReadWait()
{
	char spiData;

	while (spiXfer(DIR_SPIREAD, &spiData));

	return spiData;
}

//***************************************************************************
// Name: spiRead
//
// Description: Non-blocking read of character from SPI bus. 
//
// Parameters: None.
//
// Returns: pSpiData - Byte received.
//          int XFER_OK if data received, XFER_RETRY if not.
//
// Comments: Check for a character on the SPI bus and return.
//
//***************************************************************************
int spiRead(char *pSpiData)
{
	return spiXfer(DIR_SPIREAD, pSpiData);
}

//***************************************************************************
// Name: spiWrite
//
// Description: Blocking write of character to SPI bus. 
//
// Parameters: spiData - Byte to be transmitted.
//
// Returns: None.
//
// Comments: Waits until a character is transmitted on the SPI bus.
//
//***************************************************************************
void spiWrite(char spiData)
{
	while (spiXfer(DIR_SPIWRITE, &spiData));
}

