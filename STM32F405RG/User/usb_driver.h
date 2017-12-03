#ifndef USB_DRIVER_H
#define USB_DRIVER_H

#include "stm32f4xx.h"

#define MAX_RESPONSE_LEN 4
#define RESPONSE_NO 17

// Disk Commands
#define CMD_DIR		0x01	//Directory list or file information
							//Format: 01 0D	
							//Format: 01 20 file 0D	
#define CMD_CD		0x02	//Change current directory
							//Format: 02 20 file 0D	
#define CMD_DLD		0x05	//Delete subdirectory from current directory
							//Format: 05 20 file 0D	
#define CMD_MKD		0x06	//Make a new subdirectory in the current directory
							//Format: 06 20 file 0D	
#define CMD_DLF		0x07	//Delete a file
							//Format: 07 20 file 0D	
#define CMD_WRF		0x08	//Write the number of bytes specified in the 1st parameter to the currently open file
							//Format: 08 20 dword 0D data	
#define CMD_OPW		0x09	//Open a file for writing or create a new file
							//Format: 09 20 file 0D	
#define CMD_CLF		0x0a	//Close the currently open file
							//Format: 0A 20 file 0D	
#define CMD_RDF		0x0b	//Read the number of bytes specified in the 1st parameter from the currently open file
							//Format: 0B 20 dword 0D	
#define CMD_REN		0x0c	//Rename a file or directory
							//Format: 0C 20 file 20 file 0D	
#define CMD_OPR		0x0E	//Open a file for reading
							//Format: 0E 20 file 0D	
#define CMD_SEK		0x28	//Seek to the byte position specified by the 1st parameter in the currently open file
							//Format: 28 20 dword 0D	
#define CMD_FS		0x12	//Returns the free space available on disk if less than 4GB is free
							//Format: 12 0D	
#define CMD_FSE		0x93	//Returns the free space available on disk
							//Format: 93 0D	

// USB Device Commands
#define CMD_QP1		0x2B	//Query port 1
							//Format: 2B 0D	
#define CMD_QP2		0x2C	//Query port 2
							//Format: 2C 0D	
#define CMD_QD		0x85	//Query device number specified in the 1st parameter
							//Format: 85 20 byte 0D	
#define CMD_SC		0x86	//Set device number specified in the 1st parameter as the current device
							//Format: 86 20 byte 0D	
#define CMD_DSD		0x83	//Send data to USB device where the size of the data is specified in the 1st parameter
							//Format: 83 20 byte 0D data	
#define CMD_DRD		0x84	//Read back data from USB device
							//Format: 84 0D	
#define CMD_SSU		0x9A	//Send setup data to device control endpoint with optional follow-on data
							//Format: 9A 20 qword 0D (optional data)
#define CMD_SF		0x87	//Set device specified in the 1st parameter as an FTDI device
							//Format: 87 20 byte 0D	

// FTDI Device commands
#define CMD_FBD		0x18	//Set baud rate
							//Format: 18 20 divisor 0D	
#define CMD_FMC		0x19	//Set modem control
							//Format: 19 20 word 0D	
#define CMD_FSD		0x1a	//Set data characteristics
							//Format: 1A 20 word 0D	
#define CMD_FFC		0x1b	//Set flow control
							//Format: 1B 20 byte 0D	
#define CMD_FGM		0x1c	//Get modem status
							//Format: 1C 0D	
#define CMD_FSL		0x22	//Set latency timer
							//Format: 22 20 byte 0D	
#define CMD_FSB		0x23	//Set bit mode
							//Format: 23 20 word 0D	
#define CMD_FGB		0x24	//Get bit mode
							//Format: 24 0D	

// General Purpose Commands
#define CMD_CR		0x0d	//Empty carriage return
							//Format: 0D
#define CMD_SCS		0x10	//Short Command Set
							//Format: 10 0D
#define CMD_IPH		0x91	//Binary Numerical Input
							//Format: 91 0D

// Prompts and messages returned by VNC1L in Short Command Set
enum vResponse {
// Prompts returned by all VNC1L firmware
	Resp_Prompt_OK, // > (Success)
	Resp_Prompt_ND, // ND (Success)
	Resp_Prompt_UE, // E echo
	Resp_Prompt_LE, // e echo
	Resp_Prompt_CF, // CF (Command Failed)
	Resp_Prompt_BC, // BC (Bad Command)
	Resp_Prompt_DF, // DF (Disk Full)
	Resp_Prompt_FI, // FI (File Invalid)
	Resp_Prompt_RO, // RO (Read Only)
	Resp_Prompt_FO, // FO (File Open)
	Resp_Prompt_NE, // NE (Dir Not Empty)
	Resp_Prompt_FN, // FN (Filename Invalid)
	Resp_Prompt_End,
	// Asynchronous messages returned by all VNC1L firmware
	Resp_Message_NU, // NU / No Upgrade
	Resp_Message_DD1, // DD1 / Device Detected USB Port 1)
	Resp_Message_DD2, // DD2 / Device Detected USB Port 2)
	Resp_Message_DR1, // DR1 / Device Removed USB Port 1)
	Resp_Message_DR2, // DR2 / Device Removed USB Port 2)
	Resp_Message_Splash, // Ver ...
	Resp_None = 0xff,
};

typedef struct {
	u8 id;
	u8 length;
	char * msg;
} Response;

void usb_driver_init(void);

#endif
