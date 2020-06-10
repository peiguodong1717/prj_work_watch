#include "cc2500.h"
#include "delay.h"
#include "includes.h" 
#include "timer.h"
#include "24cxx.h"
#include "cc2500.h"  
#include "includes.h"  

uint8_t RFTimeOut=0;//RFID TimeOut;
uint8_t SerOK[2],SerLen[2],SerData[2][MAXLEN];// = {0x0c,1,2,3,4,5,6,7,8,9};
//无线相关参数
uint8_t Lqi[2];
uint8_t bCrcOk[2],Rssi[2],RfChkT,bRfChk;
uint8_t RfRunNo;
uint8_t  bRcvOk[2],bSendOk[2],IconRX,IconTX;       
int Sync0,Sync1;

uint8_t  PAMAX;//无线发射功率，CC1101和CC2500两个有差异;在主程序需初始化;
//!!必须注明volatile，否则优化，CCVer的值不对


//管理卡特征码RFID四个字节中的高个字节需是0xFF才是员工，否则为特料;
uint8_t   PeopleIdent=0xFF;//四字节中高位为0xFF的即为员工卡;
uint8_t   RfidType=0;//卡类别标识，0为物料卡，1为员工卡，2为管理卡;
//注册部分
uint8_t  CC2500Mode=0;                       //无线模式
uint8_t  Lqi16[MAXPOINT];
uint8_t  Bpoint_Value[MAXPOINT]; //32个心跳包中B频点                   //32个频点的信号强度。每次循环判断前置0
//************************************************************************************

void HWSPI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;        //EXTI初始化结构定义
  NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);	 //使能PB,D,G端口时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 , ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PA9
 	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				 //PB12上拉 防止W25X的干扰
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	//初始化指定IO
 	GPIO_SetBits(GPIOA,GPIO_Pin_7);//上拉		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 //PB12上拉 防止W25X的干扰
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	//初始化指定IO
 	GPIO_SetBits(GPIOB,GPIO_Pin_3);//上拉		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5;				 //PB12上拉 防止W25X的干扰
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	//初始化指定IO	
	
	RFCS_A=1;			//SPI片选取消 
  RFCS_B=1;			//SPI片选取消 	

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//SPI主机
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//时钟悬空低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//数据捕获于第1个时钟沿
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由软件控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;		//定义波特率预分频的值:波特率预分频值为16
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 

  //使能GPIO中断功能
  EXTI_ClearITPendingBit(EXTI_Line0);//清除中断标志
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);//管脚选择
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//事件选择
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//触发模式;上升沿:EXTI_Trigger_Rising;下降沿:EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_Line = EXTI_Line0; //线路选择
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//启动中断
  EXTI_Init(&EXTI_InitStructure);//初始化
  //使能GPIO中断功能
  EXTI_ClearITPendingBit(EXTI_Line4);//清除中断标志
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource4);//管脚选择
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//事件选择
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//触发模式;上升沿:EXTI_Trigger_Rising;下降沿:EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_Line = EXTI_Line4; //线路选择
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//启动中断
  EXTI_Init(&EXTI_InitStructure);//初始化
  //中断设置
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;       //通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//占先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;   //响应级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      //启动
  NVIC_Init(&NVIC_InitStructure);              //初始化	

  //中断设置
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;       //通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//占先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;   //响应级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      //启动
  NVIC_Init(&NVIC_InitStructure);              //初始化	
	
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
//写命令码
uint8_t CC_Cmd(uint8_t Cmd,uint8_t channel)
{
	uint8_t Status;
  if(channel)  RFCS_A = 0;//RF433_CS_DN;
	else         RFCS_B = 0;//RF433_CS_DN;
	delay_us(10);	
	Status=SPI2_ReadWriteByte(Cmd);
	RFCS_A = 1;//RF433_CS_UP;
	RFCS_B = 1;//RF433_CS_UP;
  delay_us(10);	
	return Status;
}
 void Clean_Rx_buffer(uint8_t net,uint8_t channel)
{
	uint8_t i;
	SerLen[channel]=0;
	for(i=0;i<net;i++)
		SerData[channel][i] = 0;
}
//*************************************************************************************
//指定地址，写配置字
uint8_t CC_WrReg(uint8_t Addr,uint8_t Data,uint8_t channel)
{
	uint8_t Status;

  if(channel)  RFCS_A = 0;//RF433_CS_DN;
	else         RFCS_B = 0;//RF433_CS_DN;
	delay_us(20);	
	Status=SPI2_ReadWriteByte(Addr);
	SPI2_ReadWriteByte(Data);
	RFCS_A = 1;//RF433_CS_UP;
	RFCS_B = 1;//RF433_CS_UP;
	delay_us(20);
	return Status;
}
//************************************************************************************
//指定地址，连续写配置
uint8_t CC_WrRegs(uint8_t Addr,uint8_t *Buf,uint8_t Count,uint8_t channel)
{
	uint8_t Status,i;
  if(channel)  RFCS_A = 0;//RF433_CS_DN;
	else         RFCS_B = 0;//RF433_CS_DN;
	delay_us(20);
	Status=SPI2_ReadWriteByte(Addr|0x40);
	for (i=0;i<Count;i++)
	{ 
	   SPI2_ReadWriteByte(Buf[i]);
	}
	RFCS_A = 1;//RF433_CS_UP;
	RFCS_B = 1;//RF433_CS_UP;
	delay_us(20);
	return Status;
}

//*************************************************************************************
//读状态寄存器
uint8_t CC_RdStatus(uint8_t Addr,uint8_t channel)
{
	uint8_t Data=0;   
  if(channel)  RFCS_A = 0;//RF433_CS_DN;
	else         RFCS_B = 0;//RF433_CS_DN;
	delay_us(20);
	Data=SPI2_ReadWriteByte(Addr|0xC0);//
	Data=SPI2_ReadWriteByte(0);
	RFCS_A = 1;//RF433_CS_UP;
	RFCS_B = 1;//RF433_CS_UP;
	delay_us(20);//WAIT;
	return Data;
}
//*****************************************************************************************
//读配置
uint8_t CC_RdReg(uint8_t Addr,uint8_t channel)
{
	uint8_t Data;   
    
  if(channel)  RFCS_A = 0;//RF433_CS_DN;
	else         RFCS_B = 0;//RF433_CS_DN;
	delay_us(20);
	Data=SPI2_ReadWriteByte(Addr|0x80);
	Data=SPI2_ReadWriteByte(0);
	RFCS_A = 1;//RF433_CS_UP;
	RFCS_B = 1;//RF433_CS_UP;
	delay_us(20);//WAIT;
	return Data;
}

//*******************************************************************************************
//连续读配置字
uint8_t CC_RdRegs(uint8_t Addr,uint8_t *Buf,uint8_t Count,uint8_t channel)
{
	uint8_t Status,i;

  if(channel)  RFCS_A = 0;//RF433_CS_DN;
	else         RFCS_B = 0;//RF433_CS_DN;
	delay_us(20);
	Status=SPI2_ReadWriteByte(Addr|0xC0);//******
	for (i=0;i<Count;i++)
	{ 
	 Buf[i]=SPI2_ReadWriteByte(0); //******
	}
	RFCS_A = 1;//RF433_CS_UP;
	RFCS_B = 1;//RF433_CS_UP;
	delay_us(20);//WAIT;
	return Status;

}
//******************************************************************************************
//读状态
uint8_t CC_GetStatus(uint8_t channel)
{
  return CC_Cmd(CCxxx0_SNOP,channel);
}
//**********************************************************************************
//芯片上电复位
void CC_RESET(uint8_t channel)
{
  RFCS_A = 1;//RF433_CS_UP;
	RFCS_B = 1;//RF433_CS_UP;
  if(channel)  RFCS_A = 0;//RF433_CS_DN;
	else         RFCS_B = 0;//RF433_CS_DN;
  RFCS_A = 1;//RF433_CS_UP;
	RFCS_B = 1;//RF433_CS_UP;
	delay_us(1000);//>40us
	CC_Cmd(CCxxx0_SRES,channel);
}
//***********************************************************************************
//配置芯片特性和无线参数
void CC_RfConfig(RF_SETTINGS *pRfSettings,uint8_t channel) 
{    // Write register settings
    CC_WrReg(CCxxx0_FSCTRL1,  pRfSettings->FSCTRL1,channel);
    CC_WrReg(CCxxx0_FSCTRL0,  pRfSettings->FSCTRL0,channel);
    CC_WrReg(CCxxx0_FREQ2,    pRfSettings->FREQ2,channel);
    CC_WrReg(CCxxx0_FREQ1,    pRfSettings->FREQ1,channel);
    CC_WrReg(CCxxx0_FREQ0,    pRfSettings->FREQ0,channel);
    CC_WrReg(CCxxx0_MDMCFG4,  pRfSettings->MDMCFG4,channel);
    CC_WrReg(CCxxx0_MDMCFG3,  pRfSettings->MDMCFG3,channel);
    CC_WrReg(CCxxx0_MDMCFG2,  pRfSettings->MDMCFG2,channel);
    CC_WrReg(CCxxx0_MDMCFG1,  pRfSettings->MDMCFG1,channel);
    CC_WrReg(CCxxx0_MDMCFG0,  pRfSettings->MDMCFG0,channel);
    CC_WrReg(CCxxx0_CHANNR,   pRfSettings->CHANNR,channel);
    CC_WrReg(CCxxx0_DEVIATN,  pRfSettings->DEVIATN,channel);
    CC_WrReg(CCxxx0_FREND1,   pRfSettings->FREND1,channel);
    CC_WrReg(CCxxx0_FREND0,   pRfSettings->FREND0,channel);
    CC_WrReg(CCxxx0_MCSM0 ,   pRfSettings->MCSM0 ,channel);
    CC_WrReg(CCxxx0_FOCCFG,   pRfSettings->FOCCFG,channel);
    CC_WrReg(CCxxx0_BSCFG,    pRfSettings->BSCFG,channel);
    CC_WrReg(CCxxx0_AGCCTRL2, pRfSettings->AGCCTRL2,channel);
	  CC_WrReg(CCxxx0_AGCCTRL1, pRfSettings->AGCCTRL1,channel);
    CC_WrReg(CCxxx0_AGCCTRL0, pRfSettings->AGCCTRL0,channel);
    CC_WrReg(CCxxx0_FSCAL3,   pRfSettings->FSCAL3,channel);
    CC_WrReg(CCxxx0_FSCAL2,   pRfSettings->FSCAL2,channel);
	  CC_WrReg(CCxxx0_FSCAL1,   pRfSettings->FSCAL1,channel);
    CC_WrReg(CCxxx0_FSCAL0,   pRfSettings->FSCAL0,channel);
    CC_WrReg(CCxxx0_FSTEST,   pRfSettings->FSTEST,channel);
    CC_WrReg(CCxxx0_TEST2,    pRfSettings->TEST2,channel);
    CC_WrReg(CCxxx0_TEST1,    pRfSettings->TEST1,channel);
    CC_WrReg(CCxxx0_TEST0,    pRfSettings->TEST0,channel);
    CC_WrReg(CCxxx0_IOCFG2,   pRfSettings->IOCFG2,channel);
    CC_WrReg(CCxxx0_IOCFG0,   pRfSettings->IOCFG0,channel);    
    CC_WrReg(CCxxx0_PKTCTRL1, pRfSettings->PKTCTRL1,channel);
    CC_WrReg(CCxxx0_PKTCTRL0, pRfSettings->PKTCTRL0,channel);
    CC_WrReg(CCxxx0_ADDR,     pRfSettings->DADDR,channel);
    CC_WrReg(CCxxx0_PKTLEN,   pRfSettings->PKTLEN,channel);
		
}
//****************************************************************************************
//设置射频功率
void CC_PaTable(uint8_t paTable,uint8_t channel)
{
	CC_Cmd(CCxxx0_SIDLE,channel);//为方便
  CC_WrReg(CCxxx0_PATABLE, paTable,channel);
	delay_us(1000);
}
//****************************************************************************************
//设置频道
void CC_Chan(uint8_t Chan,uint8_t channel)
{
	Chan=Chan*2-1;
	CC_Cmd(CCxxx0_SIDLE,channel);
	CC_FEC(1,channel);
	CC_Cmd(CCxxx0_SIDLE,channel);
  CC_WrReg(CCxxx0_CHANNR,Chan,channel);
	delay_us(100);
	CC_Cmd(CCxxx0_SIDLE,channel);
	delay_us(100);
  CC_Cmd(CCxxx0_SAFC,channel);//设置自动补偿;
	delay_us(100);
	CC_Cmd(CCxxx0_SIDLE,channel);
	delay_us(100);
	CC_Cmd(CCxxx0_SFSTXON,channel);
	delay_us(100);
	CC_Cmd(CCxxx0_SIDLE,channel);
	delay_us(100);
	CC2500_RxOn(channel);
}

//热重启
void CC_HotReset(uint8_t channel){
	CC_Cmd(CCxxx0_SIDLE,channel);//为方便
	CC_RfConfig(&rfCC2500Settings76800,channel);
	CC_Cmd(CCxxx0_SIDLE,channel);//为方便
  CC_WrReg(CCxxx0_MCSM1,0x00,channel);//0x0f取消CCA，收发总回到RX 不能，否则不能自动校正频率
	CC_Cmd(CCxxx0_SIDLE,channel);//为方便
  CC_FEC(1,channel);
}


//****************************************************************************************
//打开或关闭FEC前向纠错
void CC_FEC(uint8_t On,uint8_t channel)
{
	uint8_t Reg;
  Reg=CC_RdReg(CCxxx0_MDMCFG1,channel);
  if (On==1) 
     CC_WrReg(CCxxx0_MDMCFG1, (Reg | 0x80),channel);
  else
  	 CC_WrReg(CCxxx0_MDMCFG1, (Reg & 0x7F),channel);
}
//*****************************************************************************************
//打开或关闭WHITE数据功能（使数据01均衡）
void CC_WHITE(uint8_t On,uint8_t channel)
{
	uint8_t Reg;
  Reg=CC_RdReg(CCxxx0_PKTCTRL0,channel);
  if (On==1) 
     CC_WrReg(CCxxx0_PKTCTRL0, (Reg | 0x40),channel);
  else
  	 CC_WrReg(CCxxx0_PKTCTRL0, (Reg & 0xBF),channel);
}


//****************************************************************************************
//清除接收缓冲区和接收错误相关标志
void CC_ClrRx(uint8_t channel) 
{
  CC_Cmd(CCxxx0_SIDLE,channel);//!!必须在Idle状态
	CC_Cmd(CCxxx0_SFRX,channel);
}	  

//**********************************************************************************************
//清除发送缓冲区和发送错误相关标志
void  CC_ClrTx(uint8_t channel) 	
{
  CC_Cmd(CCxxx0_SIDLE,channel);//!!必须在Idle状态
  CC_Cmd(CCxxx0_SFTX,channel);
}
//**********************************************************************************
//无线模块初始化
uint8_t CC_Init(uint8_t channel)
{
  volatile uint8_t i=0;
  CC_RESET(channel);
  delay_ms(1000);
  //!!必须注明volatile，否则优化，i的值不对
  i=CC_RdStatus(CCxxx0_VERSION,channel);//0x03  
	//CC2500  
	PAMAX=0xFF;
	CC_RfConfig(&rfCC2500Settings76800,channel);
  CC_PaTable(PAMAX,channel);
	CC_Cmd(CCxxx0_SIDLE,channel);
  CC_WrReg(CCxxx0_MCSM1,0x00,channel);//0x0f取消CCA，收发总回到RX 不能，否则不能自动校正频率
  CC_FEC(1,channel);
  delay_ms(1000);
  CC_Cmd(CCxxx0_SRX,channel);
	if(CC_RdStatus(CCxxx0_VERSION,channel)==0x03)  return 0;
  else                                           return 1;

}
//**************************************************************************************
//发送数据包，缓冲区中第1字节是长度
void CC_SendPacket(uint8_t *txBuffer, uint8_t size,uint8_t channel) 
{
	uint8 usCount=0x00;
	if(channel) EXTI->IMR&=0x000ffffe;
	else        EXTI->IMR&=0x000fffef;	
	//拉高PA;
	if(channel) GDO2_A=1;
	else GDO2_B=1;
	delay_us(200);
	CC_Cmd(CCxxx0_SIDLE,channel);
 	CC_ClrTx(channel);//v1.1保证TxBYTES无以前字节
	CC_Cmd(CCxxx0_SIDLE,channel);
  CC_WrReg(CCxxx0_TXFIFO, size, channel);//len
	CC_Cmd(CCxxx0_SIDLE,channel);
	CC_WrRegs(CCxxx0_TXFIFO, txBuffer, size, channel);//***
	CC_Cmd(CCxxx0_SIDLE,channel);
	CC_Cmd(CCxxx0_STX, channel);
	//延时等发送完;
  if(channel) {
		 while((!GDO0_A)&&(usCount<0xFF)){
			 usCount++;
			 delay_us(100);
		 }
	 }
	 else {
		 while((!GDO0_B)&&(usCount<0xFF)){
			 usCount++;
			 delay_us(100);
		 }
	 };
	 usCount=0x00;
   if(channel) {
		 while(GDO0_A&&(usCount<0xFF)){
			 usCount++;
			 delay_us(100);
		 }
	 }
	 else {
		 while(GDO0_B&&(usCount<0xFF)){
			 usCount++;
			 delay_us(100);
		 }
	 };
	//置接收模式;
	CC2500_RxOn(channel); 
  if(channel)  EXTI->IMR|=1<<0;
  else         EXTI->IMR|=1<<4;
	
//	ResetCC2500();
}
//*************************************************************************************************
//读出接收的数据，和LQI,RSSI，并判断CrcOk
uint8_t CC_RdPacket(uint8_t channel)
{
   uint8_t iLen,usCount;	
	 if(channel) {
		 while(GDO0_A&&(usCount<0xFF)){
			 usCount++;
			 delay_us(100);
		 }
	 }
	 else {
		 while(GDO0_B&&(usCount<0xFF)){
			 usCount++;
			 delay_us(100);
		 }
	 };
	 delay_us(200);
 	 iLen= CC_RdReg(CCxxx0_RXFIFO,channel);//第一字节是Len 数据包长度
   if (iLen==0)
      goto RXERR; 
	 if (iLen>MAXLEN) 
	    goto RXERR;//iLen=MAXLEN;
     // Read data from RX FIFO and store in rxBuffer
	 SerLen[channel]=iLen;    //数据包长度
   CC_RdRegs(CCxxx0_RXFIFO, SerData[channel],iLen,channel); //读数据
   // Read the 2 appended status bytes (status[0] = RSSI, status[1] = LQI)
   Rssi[channel]=CC_RdReg(CCxxx0_RXFIFO,channel); 
   Lqi[channel]= CC_RdReg(CCxxx0_RXFIFO,channel);
	 if (Lqi[channel]>0x80) //80
	 {  
	    bCrcOk[channel]=1;// MSB of LQI is the CRC_OK bit ***
	    bRcvOk[channel]=TRUE;//IconRX=1;
	 }
	 else 
		 bCrcOk[channel]=0;  
	 iLen=CC_RdStatus(CCxxx0_RXBYTES,channel) & 0x7f;
	 CC2500_RxOn(channel);//接收状态;
	 if (iLen==0) return 0;
	 
RXERR:	 //应该为0，不为0就不对，要清除RxFiFo
	 CC_ClrRx(channel);
	 //iLen=CC_RdStatus(CCxxx0_RXBYTES) & 0x7f;
	 bRcvOk[channel]=0;
	 bCrcOk[channel]=0;
	 CC2500_RxOn(channel);//接收状态;
   return 1;

	 
}		
//*********************************************************************************************
//把信号强度RSSI值转换成dB值
uint8_t CC_RssiCh(uint8_t rssi)
{//输出值是正值，但都是负的dBm，例如返回值是55是-55dBm
  if (rssi>=128)
  { 
	  return (128+RSSI0-(rssi>>1));
  }
  else
  { 
	  return (RSSI0-(rssi>>1));
  }
}
//*************************************************************************************
//读信号强度RSSI值
uint8_t CC_Rssi(uint8_t channel)
{
  return CC_RdStatus(CCxxx0_RSSI,channel);
}

//**************************************************************************************
//读打包状态寄存器
uint8_t CC_PackStatus(uint8_t channel)
{
  return CC_RdStatus(CCxxx0_PKTSTATUS,channel);
  //bit0-bit7 GDO0,GDO1,GDO2,SYNC,  CCA,PQT,CS,CRCOK
  //如果MCSM1.CCA＝0没有使用CCA的话，CCA指示位总为1，
  //如果使用CCA，CCA和CS位就相反
}
//***********************************************************************************
//用CCA判断是否有信道冲突
uint8_t CC_CheckCCA(uint8_t channel)
{
	uint8_t i;  
	i=CC_RdStatus(CCxxx0_PKTSTATUS,channel);
  //bit0-bit7 GDO0,GDO1,GDO2,SYNC,  CCA,PQT,CS,CRCOK
  //如果MCSM1.CCA＝0没有使用CCA的话，CCA指示位总为1－－必须使能CCA!!
  //如果使用CCA，CCA和CS位就相反
  //if (CHK(i,6)==0)//使用CS指示，效果相同
//  if CHK(i,4) //使用CCA指示
  if ((i & 4) ==4)
    return 1;
  else
    return 0;
}

//==============================================================================
void RfOff(uint8_t channel)
{
  CC_Cmd(0x36,channel);//IDLE 必须先IDLE才能进入SLEEP
  CC_Cmd(0x39,channel);//Rf PwrDown
  
}

void CC2500_RxOn(uint8_t channel)
{
	 CC_Cmd(CCxxx0_SIDLE,channel);
	 CC_Cmd(CCxxx0_SFRX,channel);
	 CC_Cmd(CCxxx0_SIDLE,channel);
   CC_Cmd(CCxxx0_SRX,channel);
	if(channel) GDO2_A=0x00;
	else GDO2_B=0x00;	
}

void CC2500_TxOn(uint8_t channel)
{
   CC_Cmd(CCxxx0_STX,channel);
}
//无线模块功能测试，硬件调试用
void CC_Test(uint8_t channel)
{
  volatile uint8_t i=0;
  //!!必须注明volatile，否则优化，i的值不对
  i=CC_RdStatus(CCxxx0_VERSION, channel);//0x03
  //sprintf(code,"版本:%d",i);
  //News(code);
  i=CC_RdReg(CCxxx0_PKTCTRL0, channel);//0x05
  //sprintf(code,"寄存器1:%d",i);
  //News(code);
  i=CC_RdReg(CCxxx0_PKTLEN, channel);//0xff
  //sprintf(code,"寄存器2:%d",i);
  //News(code);
  i=CC_GetStatus(channel);
   CC_Cmd(CCxxx0_SIDLE,channel);
  i=CC_GetStatus(channel);
  CC_Cmd(CCxxx0_SRX,channel);
}
