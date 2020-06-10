#include "cc2500_hpd.h"
#include "gt21l.h"

unsigned char RFRxBuf[30];
unsigned char RFTxBuf[30];
const unsigned char PaTabel[8] = {0xFF ,0xFF ,0xA9 ,0x7F ,0x6E ,0x97 ,0xC6 ,0x8D};
unsigned char LinkStatus=0;
unsigned char  ModeFlag=0;

void InitRFTxBuf(void)
{
	unsigned char i;
		for(i=0;i<30;i++)
		{
			RFTxBuf[i]=0x31+i;
		}
}	

//����������SPIд�Ĵ���
void halRfWriteReg(unsigned char addr, unsigned char value)
{
    SpiSetCsLow();
    SPI2_ReadWriteByte(addr);		     //д��ַ
    SPI2_ReadWriteByte(value);			 //д������
    SpiSetCsHigh();
}

//����������SPI����д���üĴ���
void halRfWriteBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count) 
{
    unsigned char i, temp;
		temp = addr | WRITE_BURST;
    SpiSetCsLow();
    SPI2_ReadWriteByte(temp);
    for (i = 0; i < count; i++)
 	{
        SPI2_ReadWriteByte(buffer[i]);
    }
	SpiSetCsHigh();
}

//����������SPIд����
void halRfStrobe(unsigned char strobe) 
{
    SpiSetCsLow();
    SPI2_ReadWriteByte(strobe);		//д������
    SpiSetCsHigh();
}

//����������SPI���Ĵ���
unsigned char halRfReadReg(unsigned char addr)
{
	unsigned char temp, value;
  temp = addr|READ_SINGLE;//���Ĵ�������
	SpiSetCsLow();
	SPI2_ReadWriteByte(temp);
	value = SPI2_ReadWriteByte(0XFF);
	SpiSetCsHigh();
	return value;
}

//����������SPI����д���üĴ���
void halRfReadBurstReg(unsigned char addr, unsigned char *buffer, unsigned char count) 
{
    unsigned char i,temp;
		temp = addr | READ_BURST;		//д��Ҫ�������üĴ�����ַ�Ͷ�����
    SpiSetCsLow();
	  SPI2_ReadWriteByte(temp);   
    for (i = 0; i < count; i++) 
	{
        buffer[i] = SPI2_ReadWriteByte(0XFF);
    }
    SpiSetCsHigh();
}

//����������SPI��״̬�Ĵ���
unsigned char halRfReadStatus(unsigned char addr) 
{
  unsigned char value,temp;
	temp = addr | READ_BURST;		//д��Ҫ����״̬�Ĵ����ĵ�ַͬʱд�������
  SpiSetCsLow();
  SPI2_ReadWriteByte(temp);
	value = SPI2_ReadWriteByte(0XFF);
	SpiSetCsHigh();
	return value;
}

//������������λRfTx
void RESET_RfRx(void) 
{
	SpiSetCsLow(); 
	SPI2_ReadWriteByte(SRES); 		//д�븴λ����
	SpiSetCsHigh();
	 
}

//�����������ϵ縴λRfTx
void POWER_UP_RESET_RfRx(void) 
{
	SpiSetCsHigh(); 
	delay_ms(100); 				
	SpiSetCsLow(); 
	delay_ms(100); 
	SpiSetCsHigh(); 
	delay_ms(100); 
	RESET_RfRx();   		//��λRfTx
}

unsigned char Temp1;
//��������������RfTx�ļĴ���


#define Mode_carry  0
#define Mode_Package 1
#define Mode_TestRevice 2

void halRfWriteRfSettings(void) 
{
	// FREQ = 2450m  	datarate = 2.4K  RX BW = 203k  DEVIATION = 38K 2FSK

	halRfWriteReg(IOCFG0,0x06);        //GDO0Output Pin Configuration 
	halRfWriteReg(FIFOTHR,0x07);       //RX FIFO and TX FIFO Thresholds
	halRfWriteReg(SYNC1,0xD3);         //Sync Word, High Byte 
	halRfWriteReg(SYNC0,0x91);         //Sync Word, Low Byte 
	halRfWriteReg(PKTLEN,0xFF);        //Packet Length 
	halRfWriteReg(PKTCTRL1,0x04);      //Packet Automation Control
	halRfWriteReg(PKTCTRL0,0x05);      //Packet Automation Control
	halRfWriteReg(ADDR,0x00);          //Device Address 
	halRfWriteReg(CHANNR,0x00);        //Channel Number 
	halRfWriteReg(FSCTRL1,0x08);       //Frequency Synthesizer Control 
	halRfWriteReg(FSCTRL0,0x00);       //Frequency Synthesizer Control 
	halRfWriteReg(FREQ2,0x5E);         //Frequency Control Word, High Byte 
	halRfWriteReg(FREQ1,0x3B);         //Frequency Control Word, Middle Byte 
	halRfWriteReg(FREQ0,0x13);         //Frequency Control Word, Low Byte 
	halRfWriteReg(MDMCFG4,0x86);       //Modem Configuration 
	halRfWriteReg(MDMCFG3,0x83);       //Modem Configuration 
	halRfWriteReg(MDMCFG2,0x03);       //Modem Configuration
	halRfWriteReg(MDMCFG1,0x22);       //Modem Configuration
	halRfWriteReg(MDMCFG0,0xF8);       //Modem Configuration 
	halRfWriteReg(DEVIATN,0x44);       //Modem Deviation Setting 
	halRfWriteReg(MCSM2,0x07);         //Main Radio Control State Machine Configuration 
	halRfWriteReg(MCSM1,0x30);         //Main Radio Control State Machine Configuration
	halRfWriteReg(MCSM0,0x18);         //Main Radio Control State Machine Configuration 
	halRfWriteReg(FOCCFG,0x16);        //Frequency Offset Compensation Configuration
	halRfWriteReg(BSCFG,0x6C);         //Bit Synchronization Configuration
	halRfWriteReg(AGCCTRL2,0x03);      //AGC Control
	halRfWriteReg(AGCCTRL1,0x40);      //AGC Control
	halRfWriteReg(AGCCTRL0,0x91);      //AGC Control
	halRfWriteReg(WOREVT1,0x87);       //High Byte Event0 Timeout 
	halRfWriteReg(WOREVT0,0x6B);       //Low Byte Event0 Timeout 
	halRfWriteReg(WORCTRL,0xF8);       //Wake On Radio Control
	halRfWriteReg(FREND1,0x56);        //Front End RX Configuration 
	halRfWriteReg(FREND0,0x10);        //Front End TX configuration 
	halRfWriteReg(FSCAL3,0xA9);        //Frequency Synthesizer Calibration 
	halRfWriteReg(FSCAL2,0x0A);        //Frequency Synthesizer Calibration 
	halRfWriteReg(FSCAL1,0x00);        //Frequency Synthesizer Calibration 
	halRfWriteReg(FSCAL0,0x11);        //Frequency Synthesizer Calibration 
	halRfWriteReg(RCCTRL1,0x41);       //RC Oscillator Configuration 
	halRfWriteReg(RCCTRL0,0x00);       //RC Oscillator Configuration 

}


//��ʼ�� RFģ��
void Init_RfRx(void)
{

	GPIO_InitTypeDef  GPIO_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11|GPIO_Pin_12;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_15;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);

  SpiSetCsHigh();

 	POWER_UP_RESET_RfRx();
	
	
	 halRfWriteRfSettings();
	 halRfWriteBurstReg(PATABLE, (unsigned char*)PaTabel, 8);
}

//���� channel
void halRfSetChannel(unsigned char channel)
{
		halRfStrobe(SIDLE);		//������
    halRfWriteReg(CHANNR, channel);
}

//CC2500 ����Ϊ����ģʽ
void halRfSetRxMode(void)
{	
	halRfStrobe(SRX); 		//д�븴λ����
}



extern unsigned char TimeFlag;

void CC2500RxProc( void )
{
  unsigned char Rx_Length;
	unsigned char i;
	unsigned char flag;
		
//	Init_RfRx();
	
	halRfStrobe(SIDLE);				//������������Ҫ
	halRfWriteReg(PKTLEN, 0xff); 		//�������ݰ�����
	halRfStrobe(SFRX);	     				//��ϴ���ջ�����
	halRfStrobe(SIDLE);						//������
	halRfStrobe(SRX);						//�������״̬
	
	LinkStatus =1;
	
			
	Rx_Length = halRfReadReg(RXFIFO);   
	if(Rx_Length)
	{
		halRfReadBurstReg(RXFIFO, RFRxBuf, Rx_Length); 
	}

	flag = 0;
	if(Rx_Length != 6)	flag = 1;
	for(i=0;i<6;i++)
	{
		if(RFRxBuf[i]!=0x31+i)
		flag =1;
	} 
	if (flag ==0)
		LinkStatus ^=1;


	halRfStrobe(SFRX);	 	
	halRfStrobe(SIDLE);						//������
	halRfStrobe(SRX);						//�������״̬		 



	halRfStrobe(SIDLE);						//������ 
	 
}


extern unsigned char TimeFlag;

void CC2500TxProc( void )
{
	unsigned int iCnt;
	

	
	InitRFTxBuf();
	
	halRfStrobe(SIDLE);
//	for( iCnt = 0; iCnt < 250; iCnt++ );
	
	LinkStatus =1;
	

	halRfWriteReg(PKTLEN, 6); 
	halRfStrobe(SFTX);				//ˢ�·��ͻ�����
	halRfStrobe(SIDLE); 			////������
	halRfWriteReg(TXFIFO,6);	
	
	while(SpiGetGdo0());		//wait gpio1 low
	
	SpiSetCsLow();
  SPI2_ReadWriteByte(TXFIFO | WRITE_BURST);		     //д��ַ
	for( iCnt = 0; iCnt < 6; iCnt++ ) {			//д�� txbuf
		SPI2_ReadWriteByte( RFTxBuf[iCnt]);
	}
	SpiSetCsHigh();		

	halRfStrobe(STX);		//���뷢��ģʽ��������

	while( !SpiGetGdo0()); // wait ���
	while(SpiGetGdo0());  //wait ���

	LinkStatus ^=1;
	
	halRfStrobe(SIDLE); 			////������
}


void CC2500Sleep(void)
{
    Init_RfRx();
		halRfStrobe(SIDLE); 			////������
		halRfStrobe(SPWD);


	halRfStrobe(SIDLE); 			////������
	
}	

////---------------------------------------------
//void CC2500test(void)
//{
//	
//	
//	while(1)
//	{
//	if(ModeFlag&b_changemode)
//	 {
//	  ModeFlag &= ~b_changemode; 		//change mode
//	  
//	   if(FuntionMode==Funtion_PackSend)
//					CC2500TxProc();
//	  else if(FuntionMode==Funtion_PackRev)
//					CC2500RxProc();
//	  else if(FuntionMode==Funtion_Sleep)
//					CC2500Sleep();
//	  }	
//  }
//}



