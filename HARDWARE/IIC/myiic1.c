#include "myiic1.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
							  
//////////////////////////////////////////////////////////////////////////////////
 
//初始化IIC
void IIC_EE_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_4|GPIO_Pin_5); 	//PB10,PB11 输出高
}
//产生IIC起始信号
void IIC_EE_Start(void)
{
	SDA_EE_OUT();     //sda线输出
	IIC_EE_SDA=1;	  	  
	IIC_EE_SCL=1;
	delay_us(4);
 	IIC_EE_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_EE_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_EE_Stop(void)
{
	SDA_EE_OUT();//sda线输出
	IIC_EE_SCL=0;
	IIC_EE_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_EE_SCL=1; 
	IIC_EE_SDA=1;//发送I2C总线结束信号
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_EE_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_EE_IN();      //SDA设置为输入  
	IIC_EE_SDA=1;delay_us(1);	   
	IIC_EE_SCL=1;delay_us(1);	 
	while(READ_EE_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_EE_Stop();
			return 1;
		}
	}
	IIC_EE_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void IIC_EE_Ack(void)
{
	IIC_EE_SCL=0;
	SDA_EE_OUT();
	IIC_EE_SDA=0;
	delay_us(2);
	IIC_EE_SCL=1;
	delay_us(2);
	IIC_EE_SCL=0;
}
//不产生ACK应答		    
void IIC_EE_NAck(void)
{
	IIC_EE_SCL=0;
	SDA_EE_OUT();
	IIC_EE_SDA=1;
	delay_us(2);
	IIC_EE_SCL=1;
	delay_us(2);
	IIC_EE_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_EE_Send_Byte(u8 txd)
{                        
    u8 t;   
		SDA_EE_OUT(); 	    
    IIC_EE_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        //IIC_SDA=(txd&0x80)>>7;
		if((txd&0x80)>>7)
			IIC_EE_SDA=1;
		else
			IIC_EE_SDA=0;
		txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_EE_SCL=1;
		delay_us(2); 
		IIC_EE_SCL=0;	
		delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_EE_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_EE_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_EE_SCL=0; 
        delay_us(2);
		IIC_EE_SCL=1;
        receive<<=1;
        if(READ_EE_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_EE_NAck();//发送nACK
    else
        IIC_EE_Ack(); //发送ACK   
    return receive;
}




