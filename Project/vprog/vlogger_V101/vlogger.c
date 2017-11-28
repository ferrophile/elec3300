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

#pragma	CLOCK_FREQ 4000000;

// define if external RTC clock available
#undef RTC_ENABLED

#include "monitor.h"

// directory name to create to store log files in
rom char *dirName   			= "VLOG";
#define DIRNAME_LEN				4
// filename template used to generate unique file on disk
// dot in filename before extension is not required
rom char *fileFormat			= "VLOG0000CSV";
#define FILEFORMAT_LEN			11
// number of bytes to write per OPW/WRF/CLF cycle
#define WRITE_BLOCK_SIZE		512

// timer 1 definitions
#define TMR1_1MS_DEBUG			0xF860
#define TMR1_1MS_RTC			0xFFE0
// timer 1 interrupt handler
#define tmr1Interrupt           pir1.TMR1IF
void tmr1InterruptHandler();
void tmr1Init();

// usb definitions
#define FTDI_VID				0x0403
#define DLP_TILT_VID			FTDI_VID
#define DLP_TILT_PID			0xFBFA

// timer variables
volatile unsigned long msTick;
volatile unsigned short syncTick;

// milliseconds between samples
#define TMR1_COUNT				1000

// LED states
enum tLedState
{
	stReady,
	stActive,
	stOn,
	stDiskError,
	stDeviceError,
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
#define DATA_BLOCK_SIZE 64
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
	errReadDevice,
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
	
	// decrement synchronisation counter
	if (syncTick) --syncTick;

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
	else if (ledState == stDeviceError)
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
// Name: checkDlpTilt
//
// Description: Check if a DLP-TILT module is present.
//
// Parameters: None.
//
// Returns: stReady if disk connected, stDeviceError if not.
//
// Comments: Polls the connected USB devices using the QD command to find
//           a DLP-TILT module by comparing the VID and PID of each device
//           with DLP_TILT_VID the DLP_TILT_PID.
//
//***************************************************************************
enum tLedState checkDlpTilt()
{
	enum vResponse resp;
	enum tLedState state;
	unsigned char dev;
	unsigned char param[3];
	unsigned char qdBuf[MON_QD_SIZE];
	char i;

	for (dev = 0; dev < 16; dev++)
	{
		clear_wdt();

		// send query device command
		monCmdSendParam(CMD_QD, 1, &dev);
		for (i = 0; i < MON_QD_SIZE; i++)
		{
			qdBuf[i] = monReadWait();
		}
		resp = monPrompt();
		
		if (resp == Resp_Prompt_OK)
		{
			// USB device at this device number
			if (qdBuf[0])
			{
				// check VID is same as DLT-TILT
				if ((qdBuf[MON_QD_VID  ] == (DLP_TILT_VID & 0xff)) &&
					(qdBuf[MON_QD_VID+1] == (DLP_TILT_VID >> 8)))
				{
					// check PID is same as DLT-TILT
					if ((qdBuf[MON_QD_PID  ] == (DLP_TILT_PID & 0xff)) &&
						(qdBuf[MON_QD_PID+1] == (DLP_TILT_PID >> 8)))
					{
						// found DLP-TILT
						
						// set current to device
						monCmdSendParam(CMD_SC, 1, &dev);
						resp = monPrompt();
						if (resp == Resp_Prompt_OK)
						{
							// set baud on device to 38400 baud
							param[0] = 0x4e; param[1] = 0xc0; param[2] = 0;
							monCmdSendParam(CMD_FBD, 3, &param);
							// TODO: set baud on device to 115200 baud
							resp = monPrompt();
							if (resp == Resp_Prompt_OK)
							{
								// break from for loop
								state = stReady;
								break;
							}
						}
					}
				}
			}
		}
	}

	if (dev == 16) 
	{
		state = stDeviceError;
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
// Name: bufPush_ConvLong2Decimal bufPush_ConvLong2DecimalSub
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
void bufPush_ConvLong2DecimalSub(unsigned long *pData, unsigned long modulus, char *pLead)
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
		bufPush(ch);
	}
}

void bufPush_ConvLong2Decimal(unsigned long data)
{
	char lead = 0;
	bufPush_ConvLong2DecimalSub(&data, 1000000000, &lead);
	bufPush_ConvLong2DecimalSub(&data, 100000000, &lead);
	bufPush_ConvLong2DecimalSub(&data, 10000000, &lead);
	bufPush_ConvLong2DecimalSub(&data, 1000000, &lead);
	bufPush_ConvLong2DecimalSub(&data, 100000, &lead);
	bufPush_ConvLong2DecimalSub(&data, 10000, &lead);
	bufPush_ConvLong2DecimalSub(&data, 1000, &lead);
	bufPush_ConvLong2DecimalSub(&data, 100, &lead);
	bufPush_ConvLong2DecimalSub(&data, 10, &lead);
	lead = 1;
	bufPush_ConvLong2DecimalSub(&data, 1, &lead);
}

//***************************************************************************
// Name: bufPush_ConvChar2Decimal bufPush_ConvUChar2Decimal bufPush_ConvChar2DecimalSub
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
void bufPush_ConvChar2DecimalSub(unsigned char *pData, char modulus, char *pLead)
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
		bufPush(ch);
	}
}

void bufPush_ConvUChar2Decimal(unsigned char data)
{
	char lead = 0;
	bufPush_ConvChar2DecimalSub(&data, 100, &lead);
	bufPush_ConvChar2DecimalSub(&data, 10, &lead);
	lead = 1;
	bufPush_ConvChar2DecimalSub(&data, 1, &lead);
}

void bufPush_ConvChar2Decimal(signed char data)
{
	char lead = 0;
	unsigned char udata;

	if (data < 0)
	{
		bufPush('-');
		udata = -data;
	}
	else
	{
		udata = (unsigned char)data;
	}
	bufPush_ConvChar2DecimalSub(&udata, 100, &lead);
	bufPush_ConvChar2DecimalSub(&udata, 10, &lead);
	lead = 1;
	bufPush_ConvChar2DecimalSub(&udata, 1, &lead);
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
	unsigned short writeBlockCount;
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
			// STATE : USB Flash Disk and DLP-TILT detected

			// wait until start button is pressed 
			// AND both DLP-TILT AND disk connected
			while (1)
			{
				clear_wdt();

				pie1.TMR1IE = 0;
				syncTick = 500;
				pie1.TMR1IE = 1;
				while (syncTick);

				// check USB devices for DLP-TILT device connected
				ledState = checkDlpTilt();

				if (ledState == stReady)
				{
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
				}

				buttonState.stPressed = 0;
			};

			// wait for button release
			while (!buttonState.stPressed)
			{
				clear_wdt();
			} 
			
			buttonState = 0;

			// STATE: disk and DLP-TILT detected and start button pressed

			if (ledState == stOn)
			{
				// logging has been activated - flash LED accordingly
				ledState = stActive;
				error = errNone;

				// initialise syncronising counter
				pie1.TMR1IE = 0;
				syncTick = TMR1_COUNT;
				pie1.TMR1IE = 1;

				// initialise circular buffer state
				bufHead = bufTail = bufStart;
				bufCount = 0;

				// initialise file state and counters
				writeBlockCount = 0;
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

				do
				{
					clear_wdt();

					error = errNone;

					// synchronise with timer
					if (syncTick)
					{
						while (1)
						{
							pie1.TMR1IE = 0;
							if (syncTick == 0) break;
							pie1.TMR1IE = 1;
						
							if (buttonState) break;
							clear_wdt();
						};
						// next sync event is in 1000 ticks time (1 second)
						syncTick = TMR1_COUNT;
						pie1.TMR1IE = 1;
					}
					else
					{
						// synchronisation error occurred
						error = errSample;
					}


					if (!buttonState)
					{
						if (fileStatus != stFileOpen)
						{
							// open file for write
							monCmdSendParam(CMD_OPW, 11, filename);
							resp = monPrompt();
							if (resp != Resp_Prompt_OK)
							{
								ledState = stDiskError;
								error = errFileOpen;
								break;
							}
							writeBlockCount = 0;
							
							fileStatus = stFileOpen;
						}
					}

					if (!buttonState)
					{
						// send one byte of data to the DLP-TILT
						monCmdSendByteParam(CMD_DSD, 1);
						// data to send 'z' - instruction to sample XY reading
						monWrite('z');
						// wait for response from monitor
						resp = monPrompt();
						if (resp == Resp_Prompt_CF)
						{
							// DRD command failed
							error = errReadDevice;
							break;
						}

						// write time-stamp of sample to data buffer (in decimal)
						bufPush_ConvLong2Decimal(msTick);

						// total count of bytes received from DLP-TILT
						count = 0;
						// read and store accelerometer readings from the DLP-TILT
						do
						{
							// send command to read data from currently selected device
							monCmdSend(CMD_DRD);
							// response is number of bytes to read followed by a carriage return
							data = monReadWait();
							// ignore carriage return which follows
							monReadWait();

							// store data returned from command
							if (data > 0)
							{
								// add bytes received in this request to total byte count
								count += data;
								for (i = 0; i < data; i++)
								{
									// write data received to buffer separated by commas
									bufPush(',');
									// convert byte to decimal characters
									bufPush_ConvUChar2Decimal(monReadWait());
								}
								// write carriage return
								bufPush(0x0d);
							}
							// get command response
							// Note: DRD returns CF (Command Failed) if the device NAKs
							//       which may be caused by no data available
							resp = monPrompt();

							// exit when a CF is received and count > 0
						} while ((!count) || (resp != Resp_Prompt_CF));
					}

					// write data from buffer if we have reached the threshold
					// or the stop signal has been received
					if ((bufCount >= DATA_BLOCK_THRESHOLD) || (buttonState.stHeld))
					{
						// data requires to be written
						if (bufCount)
						{
							fileStatus = stFileWriteInProgress;

							// create parameter for WRF commands
							param[0] = (bufCount>>16)>>8;
							param[1] = bufCount>>16;
							param[2] = bufCount>>8;
							param[3] = bufCount;

							monCmdSendParam(CMD_WRF, 4, param);

							writeBlockCount += bufCount;

							while (bufCount)
							{
								monWrite(bufPop());
							}
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
						}

						// close file if write block size reached
						// or the stop signal has been received
						if ((writeBlockCount >= WRITE_BLOCK_SIZE) || (buttonState.stHeld))
						{
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
						}
					}

					// if button is pressed
				} while ((!buttonState) && (error == errNone));

				// in the event of an failure to close file
				if (fileStatus == stFileOpen)
				{
					error = errFileClose;
				}

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

