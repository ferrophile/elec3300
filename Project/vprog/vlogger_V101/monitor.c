#include <system.h>

#include "monitor.h"

//***************************************************************************
//  
//  String definitions for prompts and messages
//  
//***************************************************************************

// ROM arrays can be a maximum of 255 characters long and must be addressed
// using [] operators.
// The format of this array is prompt code, number of characters in prompt,
// then the text of the prompt to match. Entries must be ordered by length.
// ROM reads are comparitively slow so this array is arranged to reduce
// the number of ROM reads required to determine the prompt code.
// NOTE: Only Short Command Set codes are recognised.
#define MAX_RESPONSE_LEN 4
rom char *pResponse	= {
	// SCS Prompts
    Resp_Prompt_OK, 2, '>',0x0d,
    Resp_Prompt_UE, 2, 'E',0x0d,
    Resp_Prompt_LE, 2, 'e',0x0d,

	Resp_Prompt_ND, 3, 'N','D',0x0d,
    Resp_Prompt_CF, 3, 'C','F',0x0d,
    Resp_Prompt_BC, 3, 'B','C',0x0d,
    Resp_Prompt_DF, 3, 'D','F',0x0d,
    Resp_Prompt_FI, 3, 'F','I',0x0d,
    Resp_Prompt_RO, 3, 'R','O',0x0d,
    Resp_Prompt_FO, 3, 'F','O',0x0d,
    Resp_Prompt_NE, 3, 'N','E',0x0d,
    Resp_Prompt_FN, 3, 'F','N',0x0d,
    Resp_Message_NU, 3, 'N','U',0x0d,

	Resp_Message_DD1, 4, 'D','D','1',0x0d,
    Resp_Message_DD2, 4, 'D','D','2',0x0d,
    Resp_Message_DR1, 4, 'D','R','1',0x0d,
    Resp_Message_DR2, 4, 'D','R','2',0x0d,
    0, 0,
};


//***************************************************************************
//
// External Routines
//
//***************************************************************************

//***************************************************************************
// Name: monSendByte
//
// Description: Send a single byte to the Vinculum Monitor.
//
// Parameters: monData - Byte data.
//
// Returns: None.
//
// Comments: 
//
//***************************************************************************
inline void monSendByte(char monData)
{
	monWrite(monData);
}

//***************************************************************************
// Name: monCmdSend
//
// Description: Send a single byte command to the Vinculum Monitor.
//
// Parameters: monCmd - Byte code of command.
//
// Returns: None.
//
// Comments: Supports only SCS commands without parameters.
//
//***************************************************************************
void monCmdSend(char monCmd)
{
	if (monCmd != CMD_CR)
	{
		monWrite(monCmd);
	}
	monWrite(0x0d);
}

//***************************************************************************
// Name: monCmdSendByteParam
//
// Description: Send a single byte command to the Vinculum Monitor with a
//              single byte parameter.
//
// Parameters: monCmd - Byte code of command.
//             monParam - Byte parameter for command.
//
// Returns: None.
//
// Comments: Supports only SCS commands with a single byte parameter.
//
//***************************************************************************
void monCmdSendByteParam(char monCmd, unsigned char monParam)
{
	monWrite(monCmd);
	monWrite(' ');
	monWrite(monParam);
	monWrite(0x0d);
}

//***************************************************************************
// Name: monCmdSendByteParam
//
// Description: Send a single byte command to the Vinculum Monitor with a
//              single byte parameter.
//
// Parameters: monCmd - Byte code of command.
//             monParam - Byte parameter for command.
//
// Returns: None.
//
// Comments: Supports only SCS commands with a single byte parameter.
//
//***************************************************************************
void monCmdSendParam(char monCmd, unsigned char monCount, unsigned char *pmonParam)
{
	monWrite(monCmd);
	if (monCount--)
	{
		monWrite(' ');
		monWrite(*pmonParam++);
		while (monCount--)
		{
			monWrite(*pmonParam++);
		};
	}
	monWrite(0x0d);
}


//***************************************************************************
// Name: monResponse
//
// Description: Waits for a response from the Vinculum.
//
// Parameters: None.
//
// Returns: tpResp - response code from enum vResponse.
//
// Comments: If no valid response matched or a carriage return detected then
//           returns Resp_None. Always parses to a carriage return.
//
//***************************************************************************
enum vResponse monResponse()
{
	enum vResponse tpResp, tpThis;
	unsigned char romBufOffset;
	char cBuf[MAX_RESPONSE_LEN];
	unsigned char bufCnt, romCnt;
	char data;

	// initialise prompt detection
	bufCnt = 0;
	tpResp = Resp_None;

	// read single byte from SPI if one waiting
	if (monRead(&data) != XFER_RETRY)
	{
		for (bufCnt = 0; bufCnt < MAX_RESPONSE_LEN; bufCnt++)
		{
			romBufOffset = 0;
			
			// put last read byte of data in receive/compare buffer
			cBuf[bufCnt] = data;

			// find if the Message matches any of our predefined types
			while (tpResp == Resp_None)
			{
				// message code for this entry
				tpThis = pResponse[romBufOffset++];

				// number of characters for this entry
				romCnt = pResponse[romBufOffset++];

				// reached end of Message definitions
				if (romCnt == 0)
				{
					break;
				}

				// check against this Message if length is correct
				if (romCnt == bufCnt + 1)
				{
					unsigned char j;
					tpResp = tpThis;

					for (j = 0; j < bufCnt; j++)
					{
						romCnt--;
						// mis-match of data
						if (pResponse[romBufOffset++] != cBuf[j])
						{
							tpResp = Resp_None;
							break; // break from for loop
						}
					}
				}
				else if (romCnt > bufCnt)
				{
					// length of reponse too long
					break;
				}

				romBufOffset += romCnt;
			}

			if (data == 0x0d)
				break;

			if (tpResp != Resp_None)
				break;

			// read subsequent bytes from the SPI interface
			data = monReadWait();
			tpResp = Resp_None;
		}
		
		if (bufCnt == MAX_RESPONSE_LEN)
		{
			// wait for carriage return
			while (data != 0x0d)
			{
				data = monReadWait();
			}
		}
	}

	return tpResp;
}

//***************************************************************************
// Name: monPrompt
//
// Description: Calls monResponse until a prompt is received.
//
// Parameters: None.
//
// Returns: tpResp - response code from enum vResponse.
//
// Comments: Waits for monResponse to return a non-message value which is
//           less than Resp_Prompt_End.
//
//***************************************************************************
enum vResponse monPrompt()
{
	enum vResponse tpResp;

	do
	{
		tpResp = monResponse();
	} while (tpResp > Resp_Prompt_End);

	return tpResp;
}
