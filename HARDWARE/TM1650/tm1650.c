#include "tm1650.h"
#include "myiic.h"
#include "delay.h"
#include "led.h"
#include "myui.h"
//设置显示值
void TmDisplayOn(uint8_t addr,uint8_t val)
{
  IIC_Start();
  IIC_Send_Byte(0x48); //发送写显示缓冲区命令
  IIC_Send_Byte((DIS_LEVEL<<4)|0X09);//7段显示打开
  IIC_Stop();
  
  IIC_Start();
  IIC_Send_Byte(addr);
  IIC_Send_Byte(val);
  IIC_Stop();
}

//关闭显示
void TmDisplayOff(void)
{
  IIC_Start();
  IIC_Send_Byte(0x48);
  IIC_Send_Byte(0x71);
  IIC_Stop();
}

//读按键码
uint8_t TmReadKey(void)
{
  uint8_t key;
  
  IIC_Start();
  IIC_Send_Byte(0x49);//命令
	if(IIC_Wait_Ack()) return 0;
  key=IIC_Read_Byte(IIC_Ask);//按键码
  IIC_Stop();
  return key;
}

void Tm1650Init(void)
{	
	delay_ms(50);
	TmDisplayOn(0x02,0x00);
	delay_ms(50);
}
/*
//////////////按键值对应表/////////////
--------------------------------------------
s1  0x77 | s2  0x76 | s3  0x75 | s4  0x6f |
s5  0x6e | s6  0x6d | s7  0x67 | s8  0x66 |
s9  0x65 | s10 0x5f | s11 0x5e | s12 0x5d |
s13 0x57 | s14 0x56 | s15 0x4f | s16 0x4e |
s17 0x47 | s18 0x46 |
--------------------------------------------
*/
void TimeKey(void)
{
	uint8_t key_val;
	key_val = TmReadKey();
//  Beep = 1;
	switch(key_val)	
	{
		 case 0x77:
			DispString(0,0,"<<<<<<KEY1>>>>>>",1);
			break;
		 case 0x76:
			DispString(0,0,"<<<<<<KEY2>>>>>>",1);
			break;
		 case 0x75:
			DispString(0,0,"<<<<<<KEY3>>>>>>",1);
			break;
		 case 0x6f:
			DispString(0,0,"<<<<<<KEY4>>>>>>",1);
			break;
		 case 0x6e:
			DispString(0,0,"<<<<<<KEY5>>>>>>",1);
			break;
		 case 0x6d:
			DispString(0,0,"<<<<<<KEY6>>>>>>",1);
			break;
		 case 0x67:
			DispString(0,0,"<<<<<<KEY7>>>>>>",1);
			break;
		 case 0x66:
			DispString(0,0,"<<<<<<KEY8>>>>>>",1);
			break;
		 case 0x65:
			DispString(0,0,"<<<<<<KEY9>>>>>>",1);
			break;
		 case 0x5f:
			DispString(0,0,"<<<<<<KEY10>>>>>",1);
			break;
		 case 0x5e:
			DispString(0,0,"<<<<<<KEY11>>>>>",1);
			break;
		 case 0x5d:
			DispString(0,0,"<<<<<<KEY12>>>>>",1);
			break;
		 case 0x57:
			DispString(0,0,"<<<<<<KEY13>>>>>",1);
			break;
		 case 0x56:
			DispString(0,0,"<<<<<<KEY14>>>>>",1);
			break;
		 case 0x4f:
			DispString(0,0,"<<<<<<KEY15>>>>>",1);
			break;
		 case 0x4e:
			DispString(0,0,"<<<<<<KEY16>>>>>",1);
			break;
		 case 0x47:
			DispString(0,0,"<<<<<<KEY17>>>>>",1);
			break;
		 case 0x46:
			DispString(0,0,"<<<<<<KEY18>>>>>",1);
			break;
		 default:
//			Beep = 0;
			break;
	}
}


