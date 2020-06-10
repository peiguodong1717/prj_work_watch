#ifndef __PCF8563_H
#define __PCF8563_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
#define PCF8563_Write                            (unsigned char)0xa2  //写命令
#define PCF8563_Read                             (unsigned char)0xa3  //读命令
			  
////////////////////////////////////////////////////////////////////////////////// 

/******************************************************************************
                             参数寄存器地址宏定义
******************************************************************************/
 
#define PCF8563_Address_Control_Status_1         (unsigned char)0x00  //控制/状态寄存器1
#define PCF8563_Address_Control_Status_2         (unsigned char)0x01  //控制/状态寄存器2
 
#define PCF8563_Address_CLKOUT                   (unsigned char)0x0d  //CLKOUT频率寄存器
#define PCF8563_Address_Timer                    (unsigned char)0x0e  //定时器控制寄存器
#define PCF8563_Address_Timer_VAL                (unsigned char)0x0f  //定时器倒计数寄存器
 
#define PCF8563_Address_Years                    (unsigned char)0x08  //年
#define PCF8563_Address_Months                   (unsigned char)0x07  //月
#define PCF8563_Address_Days                     (unsigned char)0x05  //日
#define PCF8563_Address_WeekDays                 (unsigned char)0x06  //星期
#define PCF8563_Address_Hours                    (unsigned char)0x04  //小时
#define PCF8563_Address_Minutes                  (unsigned char)0x03  //分钟
#define PCF8563_Address_Seconds                  (unsigned char)0x02  //秒
 
#define PCF8563_Alarm_Minutes                    (unsigned char)0x09  //分钟报警
#define PCF8563_Alarm_Hours                      (unsigned char)0x0a  //小时报警
#define PCF8563_Alarm_Days                       (unsigned char)0x0b  //日报警
#define PCF8563_Alarm_WeekDays                   (unsigned char)0x0c  //星期报警

extern uint8_t SYS_time[7];



void PCF8563_init(void);
unsigned char RTC_Bcd2ToBin(unsigned char BCDValue);
unsigned char RTC_BinToBcd2(unsigned char BINValue);
unsigned char PCF8563_Read_Byte(unsigned char REG_ADD);
unsigned char bcd_to_hex(unsigned char data);	
void PCF8563_Write_Byte(unsigned char REG_ADD, unsigned char dat);

void PCF8563_Get_Time(uint8_t *timebuf);
void PCF8563_Set_Time(uint8_t *timebuf);
void PCF8563_Get_Reg(uint8_t *timebuf);	
uint8_t PCF8563_Get_Year(void);
 				    
#endif
