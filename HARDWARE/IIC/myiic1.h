#ifndef __MYIIC1_H
#define __MYIIC1_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
								  
//////////////////////////////////////////////////////////////////////////////////

//IO��������
#define SDA_EE_IN()  {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=8<<16;}
#define SDA_EE_OUT() {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=3<<16;}

//IO��������	 
#define IIC_EE_SCL    PAout(5) //SCL
#define IIC_EE_SDA    PAout(4) //SDA	 
#define READ_EE_SDA   PAin(4)  //����SDA 

//IIC���в�������
void IIC_EE_Init(void);                //��ʼ��IIC��IO��				 
void IIC_EE_Start(void);				//����IIC��ʼ�ź�
void IIC_EE_Stop(void);	  			//����IICֹͣ�ź�
void IIC_EE_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_EE_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_EE_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_EE_Ack(void);					//IIC����ACK�ź�
void IIC_EE_NAck(void);				//IIC������ACK�ź�

void IIC_EE_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_EE_Read_One_Byte(u8 daddr,u8 addr);	  
#endif

