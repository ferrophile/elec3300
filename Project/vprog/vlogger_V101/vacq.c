#include <system.h>
#include <icd2.h>

// PIC18F1320
#ifdef _PIC18F1320
#pragma DATA  _CONFIG1H, _IESO_ON_1H & _FSCM_ON_1H & _INTIO2_OSC_1H; // internal OSC
//#pragma DATA  _CONFIG1H, _IESO_ON_1H & _FSCM_ON_1H & _EC_OSC_1H; // external OSC
#pragma DATA  _CONFIG2L, _PWRT_OFF_2L & _BOR_OFF_2L & _BORV_27_2L;
#else
#error "Only PIC18F1320 devices supported"
#endif // _PIC18F1320

#pragma	CLOCK_FREQ				4000000;

// define if external RTC clock available
#undef RTC_ENABLED

#include "monitor.h"
#include "ADC.h"

// directory name to create to store log files in
rom char *dirName   			= "VLOG";
#define DIRNAME_LEN				4
// filename template used to generate unique file on disk
// dot in filename before extension is not required
rom char *fileFormat			= "VLOG0000BIN";
#define FILEFORMAT_LEN			11
// number of bytes to write per OPW/WRF/CLF cycle
#define WRITE_BLOCK_SIZE		512
// number of bytes written to disk during a WRF command
unsigned short writeBlockCount;

// timer 1 definitions
#define TMR1_1MS_DEBUG			0xF860
#define TMR1_1MS_RTC			0xFFE0

// timer 1 interrupt handler
#define tmr1Interrupt           pir1.TMR1IF
void tmr1InterruptHandler();
void tmr1Init();
// timer variables
volatile unsigned long msTick;
volatile unsigned char adcTimer;

// milliseconds between samples
#define TMR1_SAMPLE_TIME		5

// LED states
enum tLedState
{
	stReady,
	stActive,
	stOn,
	stDiskError,
	stSampleError,
};
enum tLedState ledState;
unsigned short ledCount;

// button states
enum tButtonState
{
	stHeld,
	stPressed,
};
volatile char buttonState;

enum tFileStatus
{
	stNone,
	stFileOpen,
	stFileClosed,
	stFileWriteInProgress,
};

// use a circular buffer for output data to allow grouping 
// writes to disk
// max number of bytes to buffer before writes
#define DATA_BLOCK_SIZE 96
unsigned char bufData[DATA_BLOCK_SIZE];
unsigned char *bufHead, *bufTail;
unsigned char bufCount;
// define start and end of buffer
#define bufStart &bufData[0]
#define bufEnd &bufData[DATA_BLOCK_SIZE]
// threshold for buffer write is size minus maximum sentence length
#define DATA_BLOCK_THRESHOLD (DATA_BLOCK_SIZE - 22)

// last error state
enum tError
{
	errNone,
	errDirectory,
	errFileOpen,
	errFileWrite,
	errFileClose,
	errSample,
};
enum tError error = errNone;

// ACTIVE LED is RB5 (Port B bit 5)
#define PORT_LED_ACTIVE portb.5
#define TRIS_LED_ACTIVE trisb.5

// START BUTTON is RA4 (Port A bit 4)
#define PORT_S1 porta.4
#define TRIS_S1 trisa.4


//***************************************************************************
// Name: interrupt
//
// Description: Handle interrupts from all sources and call required modules.
//
// Parameters: None.
//
// Returns: None.
//
// Comments: None.
//
//***************************************************************************
void interrupt( void )
{
	// handle timer1 interrupt
    if (tmr1Interrupt)
	{
		tmr1InterruptHandler();
	}

	// handle interrupts (if any) from monitor
	if (monInterrupt)
	{
		monInterruptHandler();
	}

	// handle interrupts from ADC
	if (adcInterrupt)
	{
		adcInterruptHandler();
	}
}


//***************************************************************************
// Name: tmr1InterruptHandler
//
// Description: Handle interrupts from Timer 1.
//
// Parameters: None.
//
// Returns: None.
//
// Comments: The timer interrupt is handled and flag cleared.
//
//***************************************************************************
void tmr1InterruptHandler()
{
	//clear timer 1 interrupt bit
    pir1.TMR1IF = 0; 

	// reinitialise timer for 1ms
#ifdef RTC_ENABLED
	// 1024 ticks per second when external RTC clock available
    tmr1h |= (TMR1_1MS_RTC>>8);
    tmr1l |= (TMR1_1MS_RTC&0xff);
#else
	// 1024 simulated ticks per second
    tmr1h += (TMR1_1MS_DEBUG>>8);
    tmr1l += (TMR1_1MS_DEBUG&0xff);
#endif

	// increment millisecond tick counter (32 bits)
	++msTick;

	// if the ADC timer is enabled
	if (adcTimer)
	{
		--adcTimer;

		// start an ADC conversion
		if (adcTimer == 0)
		{
			adcStart();
			adcTimer = TMR1_SAMPLE_TIME;
		}
	}

	// check for button press events
	if (PORT_S1 == 0)
	{
		// stage 1 - button is currently pressed event
		buttonState.stHeld = 1;
	}
	else
	{
		if (buttonState.stHeld)
		{
			// stage 2 - button pressed and released event
			buttonState.stPressed = 1;
		}
	}

	// update LEDs
	++ledCount;

	if (ledState == stActive)
	{
		// 1s on/1s off flash (NOT in time with sample counter)
		PORT_LED_ACTIVE = (ledCount & 0x0400)?0:1;
	}
	else if (ledState == stReady)
	{
		// slow pulse
		PORT_LED_ACTIVE = (ledCount & 0x0F00)?0:1;
	}
	else if (ledState == stDiskError)
	{
		// rapid flash
		PORT_LED_ACTIVE = (ledCount & 0x0080)?0:1;
	}
	else if (ledState == stSampleError)
	{
		// rapid flash 8 pulses
		PORT_LED_ACTIVE = (ledCount & 0x0880)?0:1;
	}
	else if (ledState == stOn)
	{
		// always on
		PORT_LED_ACTIVE = 1;
	}
}


//***************************************************************************
// Name: tmr1Init
//
// Description: Initialise Timer 1.
//
// Parameters: None.
//
// Returns: None.
//
// Comments: The timer interrupt is enabled.
//
//***************************************************************************
void tmr1Init()
{
	// Setup Timer 1
	t1con = 0x00;
#ifdef RTC_ENABLED
	// 1024 ticks per second when external RTC clock available
    tmr1h = (TMR1_1MS_RTC>>8);
    tmr1l = (TMR1_1MS_RTC&0xff);
    //t1con.T1OSCEN = 1;
    //t1con.TMR1CS = 1;
    //t1con.TMR1ON = 1;
    t1con = 0x0f;
#else
	// 1024 simulated ticks per second
    tmr1h = (TMR1_1MS_DEBUG>>8);
    tmr1l = (TMR1_1MS_DEBUG&0xff);
    //t1con.T1OSCEN = 0;
    //t1con.TMR1CS = 0;
    //t1con.TMR1ON = 1;
    t1con = 0x01;
#endif

	// clear existing interrupt conditions 
	pir1.TMR1IF = 0;
	// enable Timer 1 interrupt
	pie1.TMR1IE = 1;
}

//***************************************************************************
// Name: checkDisk
//
// Description: Check if a disk is present and recognised.
//
// Parameters: None.
//
// Returns: stReady if disk connected, stDiskError if not.
//
// Comments: Checks for the '>' prompt or ND prompt when a <CR> is sent.
//
//***************************************************************************
enum tLedState checkDisk()
{
	enum vResponse resp;
	enum tLedState state;

	// send carriage return
	monCmdSend(CMD_CR);
	resp = monPrompt();
	
	if (resp == Resp_Prompt_ND)
	{
		state = stDiskError;
	}
	else
	{
		state = stReady;
	}

	return state;
}


//***************************************************************************
// Name: bufPush bufPop
//
// Description: Implement a circular FIFO buffer.
//
// Parameters: Data to add to buffer.
//
// Returns: Data from the buffer.
//
// Comments: Implements a circular buffer to store reading data from the
//           source device before writing to disk.
//
//***************************************************************************
void bufPush(unsigned char data)
{
	if (bufCount == DATA_BLOCK_SIZE)
	{
		ledState = stSampleError;
		error = errSample;
	}
	else
	{
		*bufHead++ = data;
		++bufCount;
		if (bufHead == bufEnd)
		{
			bufHead = bufStart;
		}
	}
}

unsigned char bufPop()
{
	unsigned char data;

	data = *bufTail++;
	--bufCount;
	if (bufTail == bufEnd)
	{
		bufTail = bufStart;
	}

	return data;
}

//***************************************************************************
// Name: monWriteLong monWriteLongSub
//
// Description: Write a 32-bit unsigned integer to a FIFO buffer as a
//              decimal value.
//
// Parameters: data - value to convert and write.
//
// Returns: None.
//
// Comments: Simple decimal conversion routine using modulus value to avoid
//           the need for 32-bit division. Leading zeros are not displayed.
//
//***************************************************************************
void monWriteLongSub(unsigned long *pData, unsigned long modulus, char *pLead)
{
	char ch = '0';

	while (*pData >= modulus)
	{
		ch++;
		*pData -= modulus;
		*pLead = 1;
	}
	if (*pLead)
	{
		monWrite(ch);
		++writeBlockCount;
	}
}

void monWriteLong(unsigned long data)
{
	char lead = 0;
	monWriteLongSub(&data, 1000000000, &lead);
	monWriteLongSub(&data, 100000000, &lead);
	monWriteLongSub(&data, 10000000, &lead);
	monWriteLongSub(&data, 1000000, &lead);
	monWriteLongSub(&data, 100000, &lead);
	monWriteLongSub(&data, 10000, &lead);
	monWriteLongSub(&data, 1000, &lead);
	monWriteLongSub(&data, 100, &lead);
	monWriteLongSub(&data, 10, &lead);
	lead = 1;
	monWriteLongSub(&data, 1, &lead);
}

//***************************************************************************
// Name: monWriteSignedChar monWriteChar monWriteCharSub
//
// Description: Write an 8-bit unsigned integer to a FIFO buffer as a
//              decimal value.
//
// Parameters: data - value to convert and write.
//
// Returns: None.
//
// Comments: Simple decimal conversion routine using modulus value to avoid
//           the need for division. Leading zeros are not displayed.
//
//***************************************************************************
void monWriteCharSub(unsigned char *pData, char modulus, char *pLead)
{
	char ch = '0';

	while (*pData >= modulus)
	{
		ch++;
		*pData -= modulus;
		*pLead = 1;
	}
	if (*pLead)
	{
		monWrite(ch);
		++writeBlockCount;
	}
}

void monWriteChar(unsigned char data)
{
	char lead = 0;
	monWriteCharSub(&data, 100, &lead);
	monWriteCharSub(&data, 10, &lead);
	lead = 1;
	monWriteCharSub(&data, 1, &lead);
}

void monWriteSignedChar(signed char data)
{
	char lead = 0;
	unsigned char udata;

	if (data < 0)
	{
		monWrite('-');
		udata = -data;
	}
	else
	{
		udata = (unsigned char)data;
	}
	monWriteCharSub(&udata, 100, &lead);
	monWriteCharSub(&udata, 10, &lead);
	lead = 1;
	monWriteCharSub(&udata, 1, &lead);
}


//***************************************************************************
// Name: main
//
// Description: Main control loop for VLogger PIC Demo 1.
//
// Parameters: None.
//
// Returns: None.
//
// Comments: Initialises PICDEM 2 PLUS board.
//           Synchronises with the Vinculum Firmware.
//           Puts the Vinculum Firmware into SCS mode.
//           Detects the presence of a disk.
//
//***************************************************************************
void main()
{
    char data;
	char count;
	char i;
	enum vResponse resp;
	char param[4];
	char filename[12];
	enum tFileStatus fileStatus;

	// turn on internal oscillator & RC 
	osccon = 0x73;
	// turn off interrupts
	intcon = 0;
	intcon2 = 0x80;
	intcon3 = 0x00;
	pie1 = 0;

	TRIS_LED_ACTIVE = 0;
	TRIS_S1 = 1;

	// clear millisecond counter
	msTick = 0;

	// clear ADC timer
	adcTimer = 0;

/*
    // Setup Timer 0
    clear_bit(t0con,TMR0ON);
    // clear existing interrupt conditions 
    clear_bit(intcon,TMR0IF);
    
    tmr0l = 0;
    tmr0h = 0;
    set_bit(intcon2,TMR0IP);

    // Setup Timer 0
    // 16 bit
    clear_bit(t0con,T08BIT);
    // internal clock cycle
    clear_bit(t0con,T0CS);
    // prescaler on
    set_bit(t0con,PSA);
    // div by 256
    set_bit(t0con,T0PS2);
    set_bit(t0con,T0PS1);
    set_bit(t0con,T0PS0);
    // enable Timer 0
    set_bit(t0con,TMR0ON);
    // enable Timer 0 interrupt
    set_bit(intcon,TMR0IE);
*/
	// clear file status
	fileStatus = stNone;

	// initial LED state
	ledState = stOn;

	// setup timer 1
	tmr1Init();
	// setup monitor interface
	monInit();
	// setup analog to digital conversion
	adcInit();

	// start interrrupt handling
	intcon.GIE = 1;
	intcon.PEIE = 1;

	// wait 100 ms for banner
	delay_ms(100);

	// ensure Vinculum is synchronised
	// ignore all messages and prompts until sync operation completes
	while (1)
	{
        clear_wdt();
        do
		{
			resp = monResponse();
		} while (resp != Resp_None);

        monCmdSend('E');
        do
		{
			resp = monPrompt();
		} while (resp != Resp_Prompt_UE);

		monCmdSend('e');
		if (monPrompt() == Resp_Prompt_LE)
		{
			// change to Short Command Set Mode
			monCmdSend(CMD_SCS);
			resp = monPrompt();
			if (resp == Resp_Prompt_OK)
			{
				break;
			}
		}
	};

	// main loop
	while (1)
	{
		// STATE: Uninitialised

		// assume button is not pressed at startup
		buttonState = 0;
		ledState = stReady;

		while (1)
		{
			// wait until start button is pressed AND disk connected
			while (1)
			{
				clear_wdt();

				delay_ms(200);

				// check for presence of USB Flash Disk
				ledState = checkDisk();

				if (ledState == stReady)
				{
					if (buttonState.stHeld)
					{
						ledState = stOn;
					}
					if (buttonState.stPressed)
					{
						break;
					}
				}

				buttonState.stPressed = 0;
			};

			// wait for button release
			while (!buttonState.stPressed)
			{
				clear_wdt();
			} 
            
			buttonState = 0;

			// STATE: disk detected and start button pressed

			if (ledState == stOn)
			{
				// logging has been activated - flash LED accordingly
				ledState = stActive;
				error = errNone;

				// initialise circular buffer state
				bufHead = bufTail = bufStart;
				bufCount = 0;

				// initialise file state and counters
				fileStatus = stFileClosed;

				// create logging directory
				for (i = 0; i < 4; i++)
				{
					filename[i] = dirName[i];
				}
				monCmdSendParam(CMD_MKD, 4, filename);
				resp = monPrompt();

				// change directory to logging directory
				monCmdSendParam(CMD_CD, 4, filename);
				resp = monPrompt();
				if (resp != Resp_Prompt_OK)
				{
					// make directory or cd failed
					error = errDirectory;
					break;
				}

				// create template for unique filename
				for (i = 0; i < 12; i++)
				{
					filename[i] = fileFormat[i];
				}

				// loop through possible filenames until a unique
				// combination found
				while (1)
				{
					// perform a dir to see if filename exists
					monCmdSendParam(CMD_DIR, 11, filename);
					
					// carriage return preceeds file information returned
					// by dir command
					monReadWait();

					// an OK prompt indicates file already exists
					resp = monPrompt();
					if (resp != Resp_Prompt_OK)
					{
						// filename does not exist on disk
						break; // from while
					}

					// increment counter in filename
					for (i = 7; i > 3; i--)
					{
						if (++filename[i] > '9')
						{
							filename[i] = '0';
						}
						else
						{
							break; // from for loop
						}
					}
				}

				// check for disk removed during filename creation
				if (resp == Resp_Prompt_ND)
				{
					ledState = stDiskError;
					break;
				}

				// STATE: unique filename created, start data capture

				// start ADC conversion
				adcTimer = TMR1_SAMPLE_TIME;

				do
				{
					clear_wdt();

					// open logging file
					monCmdSendParam(CMD_OPW, 11, filename);
					resp = monPrompt();
					if (resp != Resp_Prompt_OK)
					{
						ledState = stDiskError;
						error = errFileOpen;
						break;
					}
					
					fileStatus = stFileOpen;

					// create parameter for WRF command
					param[0] = (WRITE_BLOCK_SIZE>>16)>>8;
					param[1] = WRITE_BLOCK_SIZE>>16;
					param[2] = WRITE_BLOCK_SIZE>>8;
					param[3] = WRITE_BLOCK_SIZE;

					// send WRF <WRITE_BLOCK_SIZE>
					monCmdSendParam(CMD_WRF, 4, param);

					// update status
					fileStatus = stFileWriteInProgress;

					// write start of block timestamp
					monWrite((msTick>>16)>>8);
					monWrite(msTick>>16);
					monWrite(msTick>>8);
					monWrite(msTick&0xff);

					// take size of timestamp from data block size
					writeBlockCount = sizeof(unsigned long);

					// write data pending in buffer
					while (writeBlockCount < WRITE_BLOCK_SIZE)
					{
						if (bufCount)
						{
							monWrite(bufPop());
							++writeBlockCount;
						}

						if ((buttonState) || (error == errSample)) break;
					}

					// write last data until file block size has been reached
					while (writeBlockCount < WRITE_BLOCK_SIZE)
					{
						monWrite(0x00);
						++writeBlockCount;
					}

					// wait for response from Vinculum at completion of WRF command
					resp = monPrompt();
					if (resp != Resp_Prompt_OK)
					{
						// WRF command failed - something wrong
						ledState = stDiskError;
						error = errFileWrite;
						break;
					}

					// update status
					fileStatus = stFileOpen;

					// close logging file
					monCmdSendParam(CMD_CLF, 11, filename);
					resp = monPrompt();
					if (resp != Resp_Prompt_OK)
					{
						ledState = stDiskError;
						error = errFileClose;
						break;
					}
					
					// signal file to be reopened
					fileStatus = stFileClosed;

					// if button is pressed
				} while ((!buttonState) && (error == errNone));

				// stop ADC conversion
				adcTimer = 0;

				if (buttonState.stHeld)
				{
					ledState = stOn;
    				// wait for button release
    				while (!buttonState.stPressed)
    				{
    					clear_wdt();
    				} 
				}
	            
				buttonState = 0;

				// change back up a directory
				filename[0] = filename[1] = '.';
				monCmdSendParam(CMD_CD, 2, filename);
				resp = monPrompt();
				if (resp != Resp_Prompt_OK)
				{
					// cd .. failed
					error = errDirectory;
					break;
				}
			}		

			// if disk is removed detect this and break from this while
		};
		// never leave main loop
	};
}    

