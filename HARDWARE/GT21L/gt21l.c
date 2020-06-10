#include "gt21l.h"
#include "delay.h"
#include "myui.h"
//#include "includes.h"  

//extern OS_EVENT * spi2_user;
/*******************************************************************************
* Function Name  : HC595_Init
* Description    : 初始化HC595
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GT21_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);  // 使能GT21L的时钟  

    //SPI2_SCK、SPI2_MOSI 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 ;       
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       // 管脚频率为50MHZ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         // 输出模式为复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);                  // 初始化寄存器   
    //SPI2_SCK、SPI2_MOSI 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;       
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       // 管脚频率为50MHZ
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         // 输出模式为复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);                  // 初始化寄存器   
    //SPI2_MISO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;       
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   // 输入模式为浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);        // 初始化寄存器   
	
	  //SPI2_CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;       
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // 输入模式为浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);        // 初始化寄存器   


}

void GT21_HWSPI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB,D,G端口时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 , ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PA9
 	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //PB12上拉 防止W25X的干扰
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	//初始化指定IO
 	GPIO_SetBits(GPIOA,GPIO_Pin_8);//上拉		
	
	GT21_CS=1;			//SPI片选取消 
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//SPI主机
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//时钟悬空低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第1个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由软件控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为16
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	SPI_Cmd(SPI2, ENABLE); //使能SPI外设 

}

//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频 
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
	SPI2->CR1&=0XFFC7;//位3-5清零，用来设置波特率
	SPI2->CR1|=SPI_BaudRatePrescaler;	//设置SPI1速度 
	SPI_Cmd(SPI2,ENABLE); //使能SPI1
}
//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个数据
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
	{
		retry++;
		if(retry>200)return 0;
	}	  						    
	return SPI_I2S_ReceiveData(SPI2); //返回通过SPIx最近接收的数据					    
}

//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
u8 SPI_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	  u8 status,u8_ctr;	       
  	status=SPI2_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值   	   
 	  for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI2_ReadWriteByte(0XFF);//读出数据
  	return status;        //返回读到的状态值
}
//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
u8 SPI_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	  u8 status,u8_ctr;	    
  	status = SPI2_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI2_ReadWriteByte(*pBuf++); //写入数据	 
  	return status;          //返回读到的状态值
}				   

//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
u8 WriteGT21Data(u8 value)
{
  uint8_t i,j=0x80;
  for(i=0;i<8;i++)
  {
    GT21_SCK = 0;
//		__nop();
    if(value&j)
      GT21_SI = 1;
    else
      GT21_SI = 0;
//    __nop(); 
    GT21_SCK = 1;
//		__nop();
    j=j>>1;
  }
	return 0;
}
//读取SPI寄存器值
//reg:要读的寄存器
u8 ReadGT21Data(void)
{
   uint8_t i,j,m;
	
   m=0;
   j=0x80;
   for(i=0;i<8;i++)
   {
      GT21_SCK=1;
//			__nop();
      GT21_SCK=0;
//		  __nop();
      if(GT21_SO)  
				m=m|j;
//			__nop();
      j=j>>1; 			
   } 
   return m;
}	
//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
u8 ReadGTDot(u32 addr,u8 *dotdata,u8 len)
{
  uint8_t i;
//  uint8_t err;

//  OSSemPend(spi2_user,0,&err); //请求信号量

  GT21_CS = 0;
  SPI2_ReadWriteByte(0x03);
  SPI2_ReadWriteByte(addr>>16);
  SPI2_ReadWriteByte(addr>>8);
  SPI2_ReadWriteByte(addr);
  for(i=0;i<len;i++)
	{
		dotdata[i]=SPI2_ReadWriteByte(0XFF);
	}
	GT21_CS =1;
	return 0;
//	OSSemPost(spi2_user);  //发送信号量
}
	   
void TestGT21(void)
{
	 uint8_t dot[32];
	 ReadGTDot(0x00AAC1,dot,16);
	 delay_ms(1);
	 DispString(0,0,(char*)dot,1);
}
