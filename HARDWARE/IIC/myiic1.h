#ifndef __MYIIC1_H
#define __MYIIC1_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
								  
//////////////////////////////////////////////////////////////////////////////////

//IO方向设置
#define SDA_EE_IN()  {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=8<<16;}
#define SDA_EE_OUT() {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=3<<16;}

//IO操作函数	 
#define IIC_EE_SCL    PAout(5) //SCL
#define IIC_EE_SDA    PAout(4) //SDA	 
#define READ_EE_SDA   PAin(4)  //输入SDA 

//IIC所有操作函数
void IIC_EE_Init(void);                //初始化IIC的IO口				 
void IIC_EE_Start(void);				//发送IIC开始信号
void IIC_EE_Stop(void);	  			//发送IIC停止信号
void IIC_EE_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_EE_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_EE_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_EE_Ack(void);					//IIC发送ACK信号
void IIC_EE_NAck(void);				//IIC不发送ACK信号

void IIC_EE_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_EE_Read_One_Byte(u8 daddr,u8 addr);	  
#endif

