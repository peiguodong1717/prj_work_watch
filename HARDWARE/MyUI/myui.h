#ifndef __MYUI_H
#define __MYUI_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
					  
////////////////////////////////////////////////////////////////////////////////// 


#define   LED_ALARM  (1<<10)
#define   LED_NEWS	 (1<<23)

//LCD
#define   LCD_RESET  PAout(4)
#define   LCD_SI	   PAout(7)
#define   LCD_SCK	   PAout(6)
#define   LCD_A0	   PAout(5)
#define   LCD_CS	   PAout(1)

extern uint8_t logo[];

void DispString(uint8_t hx,uint8_t hy,char *str,uint8_t style);

void Display(uint8_t hx,uint8_t hy,uint8_t charstyle,uint8_t *data,uint8_t style);

void ClearScreen(void);

void Display_Image(uint8_t page[]);

void Write_Command(uint8_t command);

void RefreshScreen(void);

void Write_Data(uint8_t data1);

void Set_column_addr(uint8_t add);

void Set_row_addr(uint8_t row);

void Lcd_Set(void);

void Myui_Init(void);

void set_Contrast(uint8_t contrast);

void Lcd_Test(void);

void Display_Clear(uint8_t data1,uint8_t data2);

uint32_t  GB2312addr(uint16_t GBcode);

void HexToStr(uint8_t Hex[],char Str[] , uint8_t len);

void ByteHexToStr(uint8_t Hex,char Str[]);

void DisplayLine(uint8_t sx,uint8_t sy, uint8_t ex,uint8_t ey ,uint8_t style);

#endif

