#include "usb_driver.h"

volatile static u8 syncCount = 0;

Response responseList[RESPONSE_NO] = {
	{Resp_Prompt_OK, 2, ">"},
	{Resp_Prompt_UE, 2, "E"},
	{Resp_Prompt_LE, 2, "e"},
	{Resp_Prompt_ND, 3, "ND"},
	{Resp_Prompt_CF, 3, "CF"},
	{Resp_Prompt_BC, 3, "BC"},
	{Resp_Prompt_DF, 3, "DF"},
	{Resp_Prompt_FI, 3, "FI"},
	{Resp_Prompt_RO, 3, "RO"},
	{Resp_Prompt_FO, 3, "FO"},
	{Resp_Prompt_NE, 3, "NE"},
	{Resp_Prompt_FN, 3, "FN"},
	{Resp_Message_NU, 3, "NU"},
	{Resp_Message_DD1, 4, "DD1"},
	{Resp_Message_DD2, 4, "DD2"},
	{Resp_Message_DR1, 4, "DR1"},
	{Resp_Message_DR2, 4, "DR2"}
};

static void usb_driver_gpio_init() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;//GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_USART2);
	//GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);
}

static void usb_driver_nvic_init() {
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 5;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

static void usb_driver_usart_init() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART2, &USART_InitStruct);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
}

static void usb_driver_write(u8 data) {
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, data);
}

// Blocking. May need implement timeout
static u8 usb_driver_get_response() {
	u8 resp = Resp_None;
	u8 buffer[MAX_RESPONSE_LEN];
	u8 bufCnt;
	
	for (bufCnt = 0; bufCnt < MAX_RESPONSE_LEN; bufCnt++) {
		while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
		buffer[bufCnt] = USART_ReceiveData(USART2);
		
		if (buffer[bufCnt] == 0x0D)
			break;
	}
	
	if (bufCnt < MAX_RESPONSE_LEN) {
		bufCnt++;
		for (u8 i = 0; i < RESPONSE_NO; i++) {
			// Compare if correct length
			if (responseList[i].length == bufCnt) {
				resp = responseList[i].id;
				for (u8 j = 0; j < bufCnt - 1; j++) {
					if (responseList[i].msg[j] != buffer[j]) {
						resp = Resp_None;
						break;
					}
				}
				// If found match, break and return it
				if (resp != Resp_None)
					break;
			}
		}
	}
	
	return resp;
}

// Blocking. May need implement timeout
static u8 usb_driver_get_prompt() {
	u8 resp;
	
	do {
		resp = usb_driver_get_response();
	} while (resp > Resp_Prompt_End);
	
	return resp;
}

void usb_driver_init() {
	usb_driver_gpio_init();
	usb_driver_nvic_init();
	usb_driver_usart_init();
	
	/*
	usb_driver_write('E');
	usb_driver_write(CMD_CR);
	while (usb_driver_get_response() != Resp_Prompt_UE);
	usb_driver_write('e');
	usb_driver_write(CMD_CR);
	while (usb_driver_get_response() != Resp_Prompt_LE);
	usb_driver_write(CMD_SCS);
	usb_driver_write(CMD_CR);
	while (usb_driver_get_prompt() != Resp_Prompt_OK);
	*/
}

void USART2_IRQHandler () {
	if (USART_GetITStatus(USART2, USART_IT_RXNE)) {
		u8 data = USART_ReceiveData(USART2);
		syncCount++;
		
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
}
