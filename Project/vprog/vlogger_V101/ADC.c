#include <system.h>

// PIC18F1320

#include "ADC.h"

//***************************************************************************
//  
//  Constants and variables for ADC
//  
//***************************************************************************

extern volatile unsigned long msTick;
extern void bufPush(unsigned char);

// Initial value of ADCON0 register 
// VCFG Vss Vdd
// CHS 0
// ADC Enabled
#define	ADCON0_SETUP    (1<<ADON)

// Initial value of ADCON1 register 
// RA7:RA1 digital, RA0 analogue
#define	ADCON1_SETUP    (0xFF ^ (1<<PCFG0))

// Initial value of ADCON2 register 
// ADFM: Left Justified
// ACQT2:ACQT0: 4Tad
// ADCS2:ADCS0: FOSC/4
#define	ADCON2_SETUP    0x14

//***************************************************************************
//
// External Routines
//
//***************************************************************************

//***************************************************************************
// Name: adcInit
//
// Description: Initialise the ADC. 
//
// Parameters: None.
//
// Returns: None.
//
// Comments: Sets up pins connecting the ACD to the outside world. 
//
//***************************************************************************
void adcInit()
{
	// Turn A/D conversion on
	adcon0 = ADCON0_SETUP;
	// RA0 is analogue input
	adcon1 = ADCON1_SETUP;
	// RA0 is analogue input
	adcon2 = ADCON2_SETUP;
}	

//***************************************************************************
// Name: adcStart
//
// Description: Runs a conversion of the ADC resulting in an interrupt.
//
// Parameters: None.
//
// Returns: Nothing.
//
// Comments: May only be called from an interrupt routine.
//
//***************************************************************************
void adcStart()
{
	adcon0.GO_DONE = 1;
}

//***************************************************************************
// Name: adcRead
//
// Description: Runs a conversion of the ADC
//
// Parameters: None.
//
// Returns: 8-bit ADC reading.
//
// Comments: May only be called from an interrupt routine.
//
//***************************************************************************
void adcRead()
{
	// clear interrupt flag
	pir1.ADIF = 0;

	// store conversion in data buffer
	bufPush(adresh);
}

