#ifndef __MYIIC_H
#define __MYIIC_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
#define IIC_Ask   0x01	
#define IIC_NoAsk 0x00
//////////////////////////////////////////////////////////////////////////////////

//IO
#define SDA_IN()  {GPIOA->CRL&=0XFFFFFF0F;GPIOA->CRL|=8<<4;}
#define SDA_OUT() {GPIOA->CRL&=0XFFFFFF0F;GPIOA->CRL|=3<<4;}

//IO
#define IIC_SCL    PAout(0) //SCL
#define IIC_SDA    PAout(1) //SDA	 
#define READ_SDA   PAin(1)  

//IIC
void IIC_HAL_Init(void);
void IIC_Init(void);               		 
void IIC_Start(void);				
void IIC_Stop(void);	  			
void IIC_Send_Byte(u8 txd);			
u8 IIC_Read_Byte(unsigned char ack);
u8 IIC_Wait_Ack(void); 				
void IIC_Ack(void);					
void IIC_NAck(void);	

//void RF_IIC_Start(void);				
//void RF_IIC_Stop(void);	  			
//void RF_IIC_Send_Byte(u8 txd);			
//u8 RF_IIC_Read_Byte(unsigned char ack);
//u8 RF_IIC_Wait_Ack(void); 				
//void RF_IIC_Ack(void);					
//void RF_IIC_NAck(void);
#endif
















