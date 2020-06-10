#include "cc2500.h"
#include "delay.h"
#include "includes.h" 
#include "timer.h"
#include "24cxx.h"
#include "cc2500.h"  
#include "includes.h"  

uint8_t RFTimeOut=0;//RFID TimeOut;
uint8_t SerOK[2],SerLen[2],SerData[2][MAXLEN];// = {0x0c,1,2,3,4,5,6,7,8,9};
//������ز���
uint8_t Lqi[2];
uint8_t bCrcOk[2],Rssi[2],RfChkT,bRfChk;
uint8_t RfRunNo;
uint8_t  bRcvOk[2],bSendOk[2],IconRX,IconTX;       
int Sync0,Sync1;

uint8_t  PAMAX;//���߷��书�ʣ�CC1101��CC2500�����в���;�����������ʼ��;
//!!����ע��volatile�������Ż���CCVer��ֵ����


//����������RFID�ĸ��ֽ��еĸ߸��ֽ�����0xFF����Ա��������Ϊ����;
uint8_t   PeopleIdent=0xFF;//���ֽ��и�λΪ0xFF�ļ�ΪԱ����;
uint8_t   RfidType=0;//������ʶ��0Ϊ���Ͽ���1ΪԱ������2Ϊ����;
//ע�Ჿ��
uint8_t  CC2500Mode=0;                       //����ģʽ
uint8_t  Lqi16[MAXPOINT];
uint8_t  Bpoint_Value[MAXPOINT]; //32����������BƵ��                   //32��Ƶ����ź�ǿ�ȡ�ÿ��ѭ���ж�ǰ��0
//************************************************************************************

void HWSPI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;        //EXTI��ʼ���ṹ����
  NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);	 //ʹ��PB,D,G�˿�ʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 , ENABLE);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PA9
 	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				 //PB12���� ��ֹW25X�ĸ���
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	//��ʼ��ָ��IO
 	GPIO_SetBits(GPIOA,GPIO_Pin_7);//����		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 //PB12���� ��ֹW25X�ĸ���
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	//��ʼ��ָ��IO
 	GPIO_SetBits(GPIOB,GPIO_Pin_3);//����		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5;				 //PB12���� ��ֹW25X�ĸ���
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);	//��ʼ��ָ��IO	
	
	RFCS_A=1;			//SPIƬѡȡ�� 
  RFCS_B=1;			//SPIƬѡȡ�� 	

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//SPI����
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//ʱ�����յ�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//���ݲ����ڵ�1��ʱ����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź����������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ16
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	SPI_Init(SPI2, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
 

  //ʹ��GPIO�жϹ���
  EXTI_ClearITPendingBit(EXTI_Line0);//����жϱ�־
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);//�ܽ�ѡ��
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�¼�ѡ��
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//����ģʽ;������:EXTI_Trigger_Rising;�½���:EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_Line = EXTI_Line0; //��·ѡ��
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//�����ж�
  EXTI_Init(&EXTI_InitStructure);//��ʼ��
  //ʹ��GPIO�жϹ���
  EXTI_ClearITPendingBit(EXTI_Line4);//����жϱ�־
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource4);//�ܽ�ѡ��
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�¼�ѡ��
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//����ģʽ;������:EXTI_Trigger_Rising;�½���:EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_Line = EXTI_Line4; //��·ѡ��
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//�����ж�
  EXTI_Init(&EXTI_InitStructure);//��ʼ��
  //�ж�����
  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;       //ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//ռ�ȼ�
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;   //��Ӧ��
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      //����
  NVIC_Init(&NVIC_InitStructure);              //��ʼ��	

  //�ж�����
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;       //ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//ռ�ȼ�
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;   //��Ӧ��
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      //����
  NVIC_Init(&NVIC_InitStructure);              //��ʼ��	
	
	SPI_Cmd(SPI2, ENABLE); //ʹ��SPI���� 

}

//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   
//SPI_BaudRatePrescaler_8   8��Ƶ   
//SPI_BaudRatePrescaler_16  16��Ƶ  
//SPI_BaudRatePrescaler_256 256��Ƶ 
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//�ж���Ч��
	SPI2->CR1&=0XFFC7;//λ3-5���㣬�������ò�����
	SPI2->CR1|=SPI_BaudRatePrescaler;	//����SPI1�ٶ� 
	SPI_Cmd(SPI2,ENABLE); //ʹ��SPI1
}
//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}			  
	SPI_I2S_SendData(SPI2, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}	  						    
	return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����					    
}

//��ָ��λ�ö���ָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ 
u8 SPI_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	  u8 status,u8_ctr;	       
  	status=SPI2_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
 	  for(u8_ctr=0;u8_ctr<len;u8_ctr++)pBuf[u8_ctr]=SPI2_ReadWriteByte(0XFF);//��������
  	return status;        //���ض�����״ֵ̬
}
//��ָ��λ��дָ�����ȵ�����
//reg:�Ĵ���(λ��)
//*pBuf:����ָ��
//len:���ݳ���
//����ֵ,�˴ζ�����״̬�Ĵ���ֵ
u8 SPI_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	  u8 status,u8_ctr;	    
  	status = SPI2_ReadWriteByte(reg);//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++)SPI2_ReadWriteByte(*pBuf++); //д������	 
  	return status;          //���ض�����״ֵ̬
}
//д������
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
//ָ����ַ��д������
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
//ָ����ַ������д����
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
//��״̬�Ĵ���
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
//������
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
//������������
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
//��״̬
uint8_t CC_GetStatus(uint8_t channel)
{
  return CC_Cmd(CCxxx0_SNOP,channel);
}
//**********************************************************************************
//оƬ�ϵ縴λ
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
//����оƬ���Ժ����߲���
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
//������Ƶ����
void CC_PaTable(uint8_t paTable,uint8_t channel)
{
	CC_Cmd(CCxxx0_SIDLE,channel);//Ϊ����
  CC_WrReg(CCxxx0_PATABLE, paTable,channel);
	delay_us(1000);
}
//****************************************************************************************
//����Ƶ��
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
  CC_Cmd(CCxxx0_SAFC,channel);//�����Զ�����;
	delay_us(100);
	CC_Cmd(CCxxx0_SIDLE,channel);
	delay_us(100);
	CC_Cmd(CCxxx0_SFSTXON,channel);
	delay_us(100);
	CC_Cmd(CCxxx0_SIDLE,channel);
	delay_us(100);
	CC2500_RxOn(channel);
}

//������
void CC_HotReset(uint8_t channel){
	CC_Cmd(CCxxx0_SIDLE,channel);//Ϊ����
	CC_RfConfig(&rfCC2500Settings76800,channel);
	CC_Cmd(CCxxx0_SIDLE,channel);//Ϊ����
  CC_WrReg(CCxxx0_MCSM1,0x00,channel);//0x0fȡ��CCA���շ��ܻص�RX ���ܣ��������Զ�У��Ƶ��
	CC_Cmd(CCxxx0_SIDLE,channel);//Ϊ����
  CC_FEC(1,channel);
}


//****************************************************************************************
//�򿪻�ر�FECǰ�����
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
//�򿪻�ر�WHITE���ݹ��ܣ�ʹ����01���⣩
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
//������ջ������ͽ��մ�����ر�־
void CC_ClrRx(uint8_t channel) 
{
  CC_Cmd(CCxxx0_SIDLE,channel);//!!������Idle״̬
	CC_Cmd(CCxxx0_SFRX,channel);
}	  

//**********************************************************************************************
//������ͻ������ͷ��ʹ�����ر�־
void  CC_ClrTx(uint8_t channel) 	
{
  CC_Cmd(CCxxx0_SIDLE,channel);//!!������Idle״̬
  CC_Cmd(CCxxx0_SFTX,channel);
}
//**********************************************************************************
//����ģ���ʼ��
uint8_t CC_Init(uint8_t channel)
{
  volatile uint8_t i=0;
  CC_RESET(channel);
  delay_ms(1000);
  //!!����ע��volatile�������Ż���i��ֵ����
  i=CC_RdStatus(CCxxx0_VERSION,channel);//0x03  
	//CC2500  
	PAMAX=0xFF;
	CC_RfConfig(&rfCC2500Settings76800,channel);
  CC_PaTable(PAMAX,channel);
	CC_Cmd(CCxxx0_SIDLE,channel);
  CC_WrReg(CCxxx0_MCSM1,0x00,channel);//0x0fȡ��CCA���շ��ܻص�RX ���ܣ��������Զ�У��Ƶ��
  CC_FEC(1,channel);
  delay_ms(1000);
  CC_Cmd(CCxxx0_SRX,channel);
	if(CC_RdStatus(CCxxx0_VERSION,channel)==0x03)  return 0;
  else                                           return 1;

}
//**************************************************************************************
//�������ݰ����������е�1�ֽ��ǳ���
void CC_SendPacket(uint8_t *txBuffer, uint8_t size,uint8_t channel) 
{
	uint8 usCount=0x00;
	if(channel) EXTI->IMR&=0x000ffffe;
	else        EXTI->IMR&=0x000fffef;	
	//����PA;
	if(channel) GDO2_A=1;
	else GDO2_B=1;
	delay_us(200);
	CC_Cmd(CCxxx0_SIDLE,channel);
 	CC_ClrTx(channel);//v1.1��֤TxBYTES����ǰ�ֽ�
	CC_Cmd(CCxxx0_SIDLE,channel);
  CC_WrReg(CCxxx0_TXFIFO, size, channel);//len
	CC_Cmd(CCxxx0_SIDLE,channel);
	CC_WrRegs(CCxxx0_TXFIFO, txBuffer, size, channel);//***
	CC_Cmd(CCxxx0_SIDLE,channel);
	CC_Cmd(CCxxx0_STX, channel);
	//��ʱ�ȷ�����;
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
	//�ý���ģʽ;
	CC2500_RxOn(channel); 
  if(channel)  EXTI->IMR|=1<<0;
  else         EXTI->IMR|=1<<4;
	
//	ResetCC2500();
}
//*************************************************************************************************
//�������յ����ݣ���LQI,RSSI�����ж�CrcOk
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
 	 iLen= CC_RdReg(CCxxx0_RXFIFO,channel);//��һ�ֽ���Len ���ݰ�����
   if (iLen==0)
      goto RXERR; 
	 if (iLen>MAXLEN) 
	    goto RXERR;//iLen=MAXLEN;
     // Read data from RX FIFO and store in rxBuffer
	 SerLen[channel]=iLen;    //���ݰ�����
   CC_RdRegs(CCxxx0_RXFIFO, SerData[channel],iLen,channel); //������
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
	 CC2500_RxOn(channel);//����״̬;
	 if (iLen==0) return 0;
	 
RXERR:	 //Ӧ��Ϊ0����Ϊ0�Ͳ��ԣ�Ҫ���RxFiFo
	 CC_ClrRx(channel);
	 //iLen=CC_RdStatus(CCxxx0_RXBYTES) & 0x7f;
	 bRcvOk[channel]=0;
	 bCrcOk[channel]=0;
	 CC2500_RxOn(channel);//����״̬;
   return 1;

	 
}		
//*********************************************************************************************
//���ź�ǿ��RSSIֵת����dBֵ
uint8_t CC_RssiCh(uint8_t rssi)
{//���ֵ����ֵ�������Ǹ���dBm�����緵��ֵ��55��-55dBm
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
//���ź�ǿ��RSSIֵ
uint8_t CC_Rssi(uint8_t channel)
{
  return CC_RdStatus(CCxxx0_RSSI,channel);
}

//**************************************************************************************
//�����״̬�Ĵ���
uint8_t CC_PackStatus(uint8_t channel)
{
  return CC_RdStatus(CCxxx0_PKTSTATUS,channel);
  //bit0-bit7 GDO0,GDO1,GDO2,SYNC,  CCA,PQT,CS,CRCOK
  //���MCSM1.CCA��0û��ʹ��CCA�Ļ���CCAָʾλ��Ϊ1��
  //���ʹ��CCA��CCA��CSλ���෴
}
//***********************************************************************************
//��CCA�ж��Ƿ����ŵ���ͻ
uint8_t CC_CheckCCA(uint8_t channel)
{
	uint8_t i;  
	i=CC_RdStatus(CCxxx0_PKTSTATUS,channel);
  //bit0-bit7 GDO0,GDO1,GDO2,SYNC,  CCA,PQT,CS,CRCOK
  //���MCSM1.CCA��0û��ʹ��CCA�Ļ���CCAָʾλ��Ϊ1��������ʹ��CCA!!
  //���ʹ��CCA��CCA��CSλ���෴
  //if (CHK(i,6)==0)//ʹ��CSָʾ��Ч����ͬ
//  if CHK(i,4) //ʹ��CCAָʾ
  if ((i & 4) ==4)
    return 1;
  else
    return 0;
}

//==============================================================================
void RfOff(uint8_t channel)
{
  CC_Cmd(0x36,channel);//IDLE ������IDLE���ܽ���SLEEP
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
//����ģ�鹦�ܲ��ԣ�Ӳ��������
void CC_Test(uint8_t channel)
{
  volatile uint8_t i=0;
  //!!����ע��volatile�������Ż���i��ֵ����
  i=CC_RdStatus(CCxxx0_VERSION, channel);//0x03
  //sprintf(code,"�汾:%d",i);
  //News(code);
  i=CC_RdReg(CCxxx0_PKTCTRL0, channel);//0x05
  //sprintf(code,"�Ĵ���1:%d",i);
  //News(code);
  i=CC_RdReg(CCxxx0_PKTLEN, channel);//0xff
  //sprintf(code,"�Ĵ���2:%d",i);
  //News(code);
  i=CC_GetStatus(channel);
   CC_Cmd(CCxxx0_SIDLE,channel);
  i=CC_GetStatus(channel);
  CC_Cmd(CCxxx0_SRX,channel);
}
