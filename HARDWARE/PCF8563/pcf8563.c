#include "pcf8563.h"
#include "myiic.h"
#include "delay.h"

uint8_t SYS_time[7]={0,0,12,1,1,1,18};     //设置时间1秒，2分钟，3小时，4号，周5,6月，7年

void PCF8563_init(void)
{
	//默认时间设置
	PCF8563_Write_Byte(0x00,0x00);  //Control_Status_1
	PCF8563_Write_Byte(0x01,0x02);  //Control_Status_2
	PCF8563_Write_Byte(0x02,RTC_BinToBcd2(SYS_time[0]));  //Seconds
	PCF8563_Write_Byte(0x03,RTC_BinToBcd2(SYS_time[1]));  //Minutes
	PCF8563_Write_Byte(0x04,RTC_BinToBcd2(SYS_time[2]));  //Hours
	PCF8563_Write_Byte(0x05,RTC_BinToBcd2(SYS_time[3]));  //Days
	PCF8563_Write_Byte(0x06,RTC_BinToBcd2(SYS_time[4]));  //WeekDays
	PCF8563_Write_Byte(0x07,RTC_BinToBcd2(SYS_time[5]));  //Months
	PCF8563_Write_Byte(0x08,RTC_BinToBcd2(SYS_time[6]));  //Years
	//默认闹钟设置
	PCF8563_Write_Byte(0x09,0x01);  //Minute_Alarm
	PCF8563_Write_Byte(0x0a,0x02);  //Hour_Alarm
	PCF8563_Write_Byte(0x0b,0x03);  //WeekDays_Alarm
	PCF8563_Write_Byte(0x0c,0x04);  //Day_Alarm
	//定时器默认设置
	PCF8563_Write_Byte(0x0d,0x03);  //CLKOUT_Frequency
	PCF8563_Write_Byte(0x0e,0x00);  //Timer_Control
	PCF8563_Write_Byte(0x0f,0x03);  //Timer_Countdown_Value		
}


void PCF8563_Get_Time(uint8_t *timebuf)
{
	uint8_t time[7];
	uint8_t i;
	time[0] = PCF8563_Read_Byte(2)&0x7f;
	time[1] = PCF8563_Read_Byte(3)&0x7f;
	time[2] = PCF8563_Read_Byte(4)&0x3f;
	time[3] = PCF8563_Read_Byte(5)&0x3f;
	time[4] = PCF8563_Read_Byte(6)&0x07;
	time[5] = PCF8563_Read_Byte(7)&0x1f;
	time[6] = PCF8563_Read_Byte(8);
	for(i=0;i<7;i++){
		timebuf[i] = bcd_to_hex(time[i]);
	}
}
uint8_t PCF8563_Get_Year(void)
{
	uint8_t year,temp;
	
	temp = PCF8563_Read_Byte(8);
	year = bcd_to_hex(temp);

	return  year;
} 

void PCF8563_Get_Reg(uint8_t *timebuf)
{
	uint8_t i;
	for(i=0;i<16;i++){
		timebuf[i] = PCF8563_Read_Byte(i);
	}
}

//设置时间timebuf（byte0 -   byte6）分别表示 秒，分钟，小时，天， 周，月，年
void PCF8563_Set_Time(uint8_t *timebuf)
{
	uint8_t i;
	for(i=0;i<7;i++){
		PCF8563_Write_Byte(i+2,RTC_BinToBcd2(timebuf[i]));
	}
} 

void PCF8563_Write_Byte(unsigned char REG_ADD, unsigned char dat)
{
	IIC_Start();
	IIC_Send_Byte(PCF8563_Write);//发送写命令并检查应答位
	while(IIC_Wait_Ack());
	IIC_Send_Byte(REG_ADD);
	IIC_Wait_Ack();
	IIC_Send_Byte(dat);	//发送数据	
	IIC_Wait_Ack();
	IIC_Stop();
	delay_us(2);
} 

unsigned char PCF8563_Read_Byte(unsigned char REG_ADD)
{
	u8 ReData;
	IIC_Start( );
	IIC_Send_Byte(PCF8563_Write);	//发送写命令并检查应答位
	while(IIC_Wait_Ack( ));
	IIC_Send_Byte(REG_ADD);	//确定要操作的寄存器
	IIC_Wait_Ack();
	IIC_Start();	//重启总线
	IIC_Send_Byte(PCF8563_Read);	//发送读取命令
	IIC_Wait_Ack();
	ReData = IIC_Read_Byte(0);	//读取数据,加发送非应答
	IIC_Stop();
	delay_us(2);
	return ReData;
}



unsigned char RTC_BinToBcd2(unsigned char BINValue)
{
	unsigned char bcdhigh = 0;
	
	while (BINValue >= 10)
	{
		bcdhigh++;
		BINValue -= 10;
	}
	
	return ((unsigned char)(bcdhigh << 4) | BINValue);
}

unsigned char bcd_to_hex(unsigned char data)
{
    unsigned char temp;

    temp = ((data>>4)*10 + (data&0x0f));
    return temp;
}

unsigned char RTC_Bcd2ToBin(unsigned char BCDValue)
{
	unsigned char tmp = 0;
	
	tmp = ((unsigned char)(BCDValue & (unsigned char)0xF0) >> (unsigned char)0x04) * 10;
	return (tmp + (BCDValue & (unsigned char)0x0F));
}

