#include "myiic.h"
#include "delay.h"
/*////////////////////////////////////////////////////////////////////////////////	 
I2C写流程

写寄存器的标准流程为：
1.    Master发起START
2.    Master发送I2C addr（7bit）和w操作0（1bit），等待ACK
3.    Slave发送ACK
4.    Master发送reg addr（8bit），等待ACK
5.    Slave发送ACK
6.   Master发送data（8bit），即要写入寄存器中的数据，等待ACK
7.    Slave发送ACK
8.    第6步和第7步可以重复多次，即顺序写多个寄存器
9.    Master发起STOP

I2C读流程

读寄存器的标准流程为：
1.    Master发送I2Caddr（7bit）和 W操作1（1bit），等待ACK
2.    Slave发送ACK
3.    Master发送reg addr（8bit），等待ACK
4.    Slave发送ACK
5.   Master发起START
6.    Master发送I2C addr（7bit）和 R操作1（1bit），等待ACK
7.    Slave发送ACK
8.   Slave发送data（8bit），即寄存器里的值
9.   Master发送ACK
10.    第8步和第9步可以重复多次，即顺序读多个寄存器
*//////////////////////////////////////////////////////////////////////////////////

void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1);
}
void IIC_Delay(uint16_t times)
{
	uint16_t i;
	for(i=0;i<times;i++)
		__nop();
}

//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(2);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(2);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(2);
	IIC_SCL=1; 
	delay_us(2);
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(2);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA=1;delay_us(1);	   
	IIC_SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//时钟输出0 	   

	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
					//IIC_SDA=(txd&0x80)>>7;
			if((txd&0x80)>>7)
				IIC_SDA=1;
			else
				IIC_SDA=0;
			txd<<=1; 	  
			delay_us(2);   //对TEA5767这三个延时都是必须的
			IIC_SCL=1;
			delay_us(2); 
			IIC_SCL=0;	
			delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
  for(i=0;i<8;i++ )
	{
		IIC_SCL=0; 
		delay_us(2);
		IIC_SCL=1;
		receive<<=1;
		if(READ_SDA)receive++;   
		delay_us(2); 
  }					 
	if (!ack)
			IIC_NAck();//发送nACK
	else
			IIC_Ack(); //发送ACK   
	return receive;
}






