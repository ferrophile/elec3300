#include "lcd_main.h"
#include "lcd_font.h"

u16 curr_bg_color = BLACK;
u16 curr_text_color = BLACK;
u16 curr_text_color_sp = BLACK;

static u8 col_shift;
static u8 row_shift;
	
u8 tft_orientation = 0, tft_enabled = 1;
u8 tft_width = 0, tft_height = 0;
u8 tft_y_index = 0;
u8 char_max_x, char_max_y;

u8 cur_screen, next_screen;
u8 pointer_to_curr_screen = 0;
char text_buf[2][160];
u16 text_color_buf[2][160];
u16 bg_color_buf[2][160];
u16 print_pos = 0;

/**
  * @brief  Initialization of SPI for TFT
  * @param  None
  * @retval None
  */
void tft_spi_init(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	SPI_InitTypeDef SPI_InitStruct;
	SPI_I2S_DeInit(SPI1);
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI1, &SPI_InitStruct);
  SPI_Cmd(SPI1, ENABLE);
}

/**
  * @brief  Sending a command
  * @param  command: one byte command to be sent
  * @retval None
  */
static inline void tft_write_command(u8 command) {
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, command);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

/**
  * @brief  Sending a data
  * @param  data: one byte data to be sent
  * @retval None
  */
static inline void tft_write_data(u8 data) {
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	GPIO_SetBits(GPIOA, GPIO_Pin_6);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	SPI_I2S_SendData(SPI1, data);
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	SPI_I2S_ReceiveData(SPI1);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

/**
  * @brief  Initialization of TFT
  * @param  orientation: default orientation
  * @param  in_bg_color: default background color
  * @param  in_text_color: default text color
  * @param  in_text_color_sp: default special text color
  * @retval None
  */
void tft_init(TFT_ORIENTATION orientation, u16 in_bg_color, u16 in_text_color, u16 in_text_color_sp) {
	tft_y_index = 0;
	cur_screen = 0;
	next_screen = 1;
	tft_orientation = orientation;

	col_shift = (tft_orientation % 2) ? 1 : 2;
	row_shift = (tft_orientation % 2) ? 2 : 1;
	
	tft_spi_init();
	
	//Hardware reset
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);
	_delay_ms(100);
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
	_delay_ms(100);
	
	//LCD Init For 1.44Inch LCD Panel with ST7735R.
	tft_write_command(0x11);//Sleep exit
	_delay_ms(120);
		
	//ST7735R Frame Rate
	tft_write_command(0xB1);
	tft_write_data(0x01);
	tft_write_data(0x2C);
	tft_write_data(0x2D);

	tft_write_command(0xB2); 
	tft_write_data(0x01);
	tft_write_data(0x2C);
	tft_write_data(0x2D);

	tft_write_command(0xB3);
	tft_write_data(0x01);
	tft_write_data(0x2C);
	tft_write_data(0x2D);
	tft_write_data(0x01);
	tft_write_data(0x2C);
	tft_write_data(0x2D);
	
	tft_write_command(0xB4); //Column inversion
	tft_write_data(0x07); 
	
	//ST7735R Power Sequence
	tft_write_command(0xC0);
	tft_write_data(0xA2);
	tft_write_data(0x02);
	tft_write_data(0x84);
	tft_write_command(0xC1);
	tft_write_data(0xC5);

	tft_write_command(0xC2);
	tft_write_data(0x0A);
	tft_write_data(0x00);

	tft_write_command(0xC3);
	tft_write_data(0x8A);
	tft_write_data(0x2A);
	tft_write_command(0xC4);
	tft_write_data(0x8A);
	tft_write_data(0xEE);
	
	tft_write_command(0xC5); //VCOM
	tft_write_data(0x0E);

	tft_write_command(0x36); //MX, MY, RGB mode
	switch (tft_orientation) {
		case 0: tft_write_data(0xC0); break;
		case 1: tft_write_data(0xA0); break;
		case 2: tft_write_data(0x00); break;
		case 3: tft_write_data(0x60); break;
	}
	
	//ST7735R Gamma Sequence
	tft_write_command(0xe0);
	tft_write_data(0x0f);
	tft_write_data(0x1a);
	tft_write_data(0x0f);
	tft_write_data(0x18);
	tft_write_data(0x2f);
	tft_write_data(0x28);
	tft_write_data(0x20);
	tft_write_data(0x22);
	tft_write_data(0x1f);
	tft_write_data(0x1b);
	tft_write_data(0x23);
	tft_write_data(0x37);
	tft_write_data(0x00);
	tft_write_data(0x07);
	tft_write_data(0x02);
	tft_write_data(0x10);

	tft_write_command(0xe1);
	tft_write_data(0x0f);
	tft_write_data(0x1b);
	tft_write_data(0x0f);
	tft_write_data(0x17);
	tft_write_data(0x33);
	tft_write_data(0x2c);
	tft_write_data(0x29);
	tft_write_data(0x2e);
	tft_write_data(0x30);
	tft_write_data(0x30);
	tft_write_data(0x39);
	tft_write_data(0x3f);
	tft_write_data(0x00);
	tft_write_data(0x07);
	tft_write_data(0x03);
	tft_write_data(0x10);
	
	tft_write_command(0x2a);
	tft_write_data(0x00);
	tft_write_data(0x00);
	tft_write_data(0x00);
	tft_write_data(0x7f);

	tft_write_command(0x2b);
	tft_write_data(0x00);
	tft_write_data(0x00);
	tft_write_data(0x00);
	tft_write_data(0x9f);

	tft_write_command(0xF0); //Enable test command
	tft_write_data(0x01);
	tft_write_command(0xF6); //Disable ram power save mode
	tft_write_data(0x00);
	
	tft_write_command(0x3A); //65k mode
	tft_write_data(0x05);
	
	tft_write_command(0x29);//Display on
	
	tft_write_command(0x2C);
	
	tft_set_bg_color(in_bg_color);
	tft_set_text_color(in_text_color);
	tft_set_special_color(in_text_color_sp);
	
	tft_fill_color(in_bg_color);
	
	char_max_x = (orientation % 2) ? CHAR_MAX_X_HORIZONTAL : CHAR_MAX_X_VERTICAL;
	char_max_y = (orientation % 2) ? CHAR_MAX_Y_HORIZONTAL : CHAR_MAX_Y_VERTICAL;

	tft_clear();
	tft_update();
}

/**
  * @brief  Enable using TFT
  * @param  None
  * @retval None
  */
void tft_enable(void) {
	tft_enabled = 1;
}

/**
  * @brief  Disable using TFT
  * @param  None
  * @retval None
  */
void tft_disable(void) {
	tft_enabled = 0;
}

/**
  * @brief  Set the current background color
  * @param  None
  * @retval None
  */
void tft_set_bg_color(u16 in_bg_color) {
	curr_bg_color = in_bg_color;
}

/**
  * @brief  Set the current text color
  * @param  None
  * @retval None
  */
void tft_set_text_color(u16 in_text_color) {
	curr_text_color = in_text_color;
}

/**
  * @brief  Set the current special text color
  * @param  None	
  * @retval None
  */
void tft_set_special_color(u16 text_color_sp) {
	curr_text_color_sp = text_color_sp;
}

u8 tft_get_orientation() {
	return tft_orientation;
}

static inline void tft_set_region(u16 x, u16 y, u16 w, u16 h) {
	//tft_write_data(0x00); //Dummy
	tft_write_command(0x2a);
	tft_write_data(0x00);
	tft_write_data(x+col_shift);
	tft_write_data(0x00);
	tft_write_data(x+w+col_shift);

	tft_write_command(0x2b);
	tft_write_data(0x00);
	tft_write_data(y+row_shift);
	tft_write_data(0x00);
	tft_write_data(y+h+row_shift);
	
	tft_write_command(0x2c);
}

/**
  * @brief  Clear one line in the text buffer
  * @param  line: the line to be cleared
  * @retval None
  */
void tft_clear_line(u8 line){
	for (u8 x = 0; x < char_max_x; x++) {
		text_buf[cur_screen][line * char_max_x + x] = ' ';
		text_color_buf[cur_screen][line * char_max_x + x] = curr_text_color;
		bg_color_buf[cur_screen][line * char_max_x + x] = curr_bg_color;
	}
}

/**
  * @brief  Clear the whole text buffer
  * @param  None
  * @retval None
  */
void tft_clear(void){
	tft_y_index = 0;
	for(u8 y = 0; y < char_max_y; y++){
		tft_clear_line(y);
	}
}

/**
  * @brief  Print a single pixel on screen
  * @param  x: x-coordinate
  * @param  y: y-coordinate
  * @param  color: color of the pixel
  * @retval None
  */
void tft_put_pixel(u8 x, u8 y, u16 color) {
	tft_set_region(x, y, 1, 1);
	tft_write_data(color >> 8);
	tft_write_data(color);
}

/**
  * @brief  Fill the whole screen with a color
  * @param  color: color to be filled with
  * @retval None
  */
void tft_fill_color(u16 color) {
	u16 i;
	if (tft_orientation % 2)
		tft_set_region(0, 0, MAX_HEIGHT, MAX_WIDTH);
	else
		tft_set_region(0, 0, MAX_WIDTH, MAX_HEIGHT);
	
	for (i = 0; i < MAX_WIDTH*MAX_HEIGHT; i++) {
		tft_write_data(color >> 8);
		tft_write_data(color);
	}
}

/**
  * @brief  Clear every pixel on screen
  * @param  None
  * @retval None
  */
void tft_force_clear(void) {
	for (u8 i = 0; i < char_max_x * char_max_y; i++) {
		text_buf[0][i] = ' ';
		text_buf[1][i] = ' ';
	}
	tft_fill_color(curr_bg_color);
}

static void tft_set_buffer(u8 x, u8 y, u8 * fp) {
	u8 i = y * char_max_x + x;
	u8 is_special = 0;
	
	while (*fp && i < char_max_x * char_max_y) {
		switch(*fp) {
			case '\r':
			case '\n':
				i = (i / char_max_x + 1) * char_max_x + x;
				break;
			case '[':
			case ']':
				is_special = (*fp == '[');
				break;
			default:
				text_buf[cur_screen][i] = *fp;
				text_color_buf[cur_screen][i] = is_special ? curr_text_color_sp : curr_text_color;
				bg_color_buf[cur_screen][i++] = curr_bg_color;
				break;
		}
		fp++;
	}
}

char tft_get_buffer_char(u8 x, u8 y) {
	return text_buf[cur_screen][y * char_max_x + x];
}

u16 tft_get_buffer_color(u8 x, u8 y) {
	return text_color_buf[cur_screen][y * char_max_x + x];
}

/**
  * @brief  Print a string at certain position, use [] to indicate special words
  * @param  x: starting x-coordinate
  * @param  y: starting y-coordinate
  * @param  pstr: string to be printed
  */
void tft_prints(u8 x, u8 y, const char * pstr, ...){
	if (x >= char_max_x || y >= char_max_y)
		return;
	
	u8 buf[256];
	va_list arglist;
	va_start(arglist, pstr);
	vsprintf((char*)buf, (const char*)pstr, arglist);
	va_end(arglist);
	
	tft_set_buffer(x, y, buf);
}

/**
	* Append a line to the tft screen. Not affected by tft_prints
	* @param pstr: Stuff to be printed
	*/
void tft_println(const char * pstr, ...){
	u8 buf[256];
	va_list arglist;
	va_start(arglist, pstr);
	vsprintf((char*)buf, (const char*)pstr, arglist);
	va_end(arglist);
	
	tft_set_buffer(0, tft_y_index, buf);
	
	tft_y_index++;
	if (tft_y_index >= char_max_y) tft_y_index = 0;
}

static inline bool tft_char_is_changed(u8 i){
	return !((text_buf[cur_screen][i] == text_buf[next_screen][i])
						&& (text_color_buf[cur_screen][i] == text_color_buf[next_screen][i])
						&& (bg_color_buf[cur_screen][i] == bg_color_buf[next_screen][i]));
}

/**
  * @brief  Refresh the whole screen
  * @param  None
  * @retval None
  */
void tft_update(void) {
	u8 * char_ptr;
	s16 adj_cnt = 0;
	u16 color;
	u8 char_cnt = 0;
	u8 y = 0;
	
	if (!tft_enabled)
		return;
	
	for (int i = 0; i < char_max_x * char_max_y; i++) {
		if (tft_char_is_changed(i)) {
			adj_cnt = 1;
			while (((i % char_max_x) + adj_cnt < char_max_x) && tft_char_is_changed(i + (adj_cnt++))); //Search for diff char
			tft_set_region((i % char_max_x)*CHAR_WIDTH, (i / char_max_x)*CHAR_HEIGHT, adj_cnt*CHAR_WIDTH-1, CHAR_HEIGHT-1);
			
			y = 0;
			while (y < CHAR_HEIGHT) {
				char_ptr = CHAR_PTR(text_buf[cur_screen][i + char_cnt]);
				for (u8 x = CHAR_WIDTH; x > 0; x--) {
					color = (char_ptr[y] >> (x - 1) & 0x01) ? text_color_buf[cur_screen][i + char_cnt] : bg_color_buf[cur_screen][i + char_cnt];
					tft_write_data(color >> 8);
					tft_write_data(color);
				}
				if (++char_cnt == adj_cnt) {
					char_cnt = 0;
					y++;
				}
			}
			i += adj_cnt-1;
		}
	}
	
	// Swap pointers
	cur_screen = (cur_screen == 0);
	next_screen = (next_screen == 0);
}
