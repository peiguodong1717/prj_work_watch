#include "pn532_hus.h"
#include "delay.h"	
#include "usart.h"
#include "includes.h"  

extern OS_EVENT * rfid_user;

uint8_t flag_rev_finish;  //1为接收完成
uint8_t IF_UID[4]; //存储 UID

uint8_t IF_Start_Find=0;

uint8_t pn532_packetbuffer[PN532_PACKBUFFSIZ];

uint8_t block_1 = 48;
uint8_t block_2 = 49;

////55 55 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF 03 FD D4 14 01 17 00  
uint8_t nfc_WakeUp(void)
{
	uint8_t data[24];
	uint8_t i;
	uint8_t CheckCode=0; //数据校验码
	uint8_t temp=0;
	
	while(1)
	{ 
	//  flag_rev_finish=0;
		data[0]=0x55;
		data[1]=0x55;
		data[2]=0x00;
		data[3]=0x00;
		data[4]=0x00;
		data[5]=0x00;
		data[6]=0x00;
		data[7]=0x00;
		data[8]=0x00;
		data[9]=0x00;
		data[10]=0x00;
		data[11]=0x00;
		data[12]=0x00;
		data[13]=0x00;
		data[14]=0x00;
		data[15]=0x00;
		
		data[16]=0xFF;
		data[17]=0x03; //包 长度
		data[18]=0xFD; //包 长度 校验  0x100-data[3]
		data[19]=0xD4; //命令标识码
		data[20]=0x14; //命令标识码
		data[21]=0x01;
		data[22]=0x17; //数据 校验   0x100-
		data[23]=0x00;
		
		UART1_Send(sizeof(data),data);     
		//00 00 FF 00 FF 00 00 00 FF 02 FE D5 15 16 00    
		temp=0;
		delay_ms(500); 
		for(i=11;i<13;i++)
		{
				temp+=USART1_RX_BUF[i];
		}
		CheckCode=0x100-temp;
		if(CheckCode==USART1_RX_BUF[13])
		{
			CleanBuffer(USART1_RX_BUF,20);//清除 串口接收缓冲区前30 个字节数据
			return 0;
		}
		else 
			return 1;
	}
}

/**
    Sends one or more commands to the NFC module.
    @param command The array of commands.
    @param cmdlen The number of commands.
    @param addr The address to be read from.
*/
void sendCommand(uint8_t command[], int8_t cmdlen)
{
    unsigned int checksum;
		unsigned int cmdlen_1;
		unsigned int checksum_1;
		unsigned char messages[128];
    uint16_t size;
	  uint8_t i,j;
    
		size = 8 + cmdlen;
    cmdlen++;
    checksum = PN532_PREAMBLE + PN532_STARTCODE1 + PN532_STARTCODE2;
	
    // Preampel and Startcode
    messages[0] = PN532_PREAMBLE;
    messages[1] = PN532_STARTCODE1;
    messages[2] = PN532_STARTCODE2;
    
    // Length = TFI + PD0 to PDn
    messages[3] = cmdlen;

    // Length Checksum LCS = Lower byte of [LEN + LCS] = 00
    cmdlen_1 = ~cmdlen + 1;
    messages[4] = cmdlen_1;
    
    // TFI
    messages[5] = PN532_HOSTTOPN532;
    checksum += PN532_HOSTTOPN532;

    j = 6;
    for (i = 0; i < cmdlen - 1; i++) {
        messages[j] = command[i];
        checksum += command[i];
        j++;
    }

    // Data Checksum DCS = Lower byte of [TFI + PD0 + PD1 + ? + PDn + DCS] = 00
    checksum_1 = ~checksum;
    messages[j] = checksum_1;
    
    // Postampel
    messages[j + 1] = PN532_POSTAMBLE;
		UART1_Send(size,messages);
}

/**
    Writes a data block.
    @param blockNumber The block number.
    @param data The data to be written.
*/
uint8_t writeDataBlock (uint8_t blockNumber, uint8_t data[])
{
		uint8_t i,flag;
    // Prepare the first command
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      // Card number
    pn532_packetbuffer[2] = PN532_MIFARE_WRITE;     // Mifare Write command = 0xA0
    pn532_packetbuffer[3] = blockNumber;            // Block Number (0..63 for 1K, 0..255 for 4K)
    
    for (i = 0; i < 16; i++) {
        pn532_packetbuffer[i + 4] = data[i];
    }

    sendCommand(pn532_packetbuffer, 20);
		return flag;
}
/**
    Reads a data block.
    @param blockNumber The block number.
    @param data The data to be read. This is written to be the function.
*/
void readDataBlock(uint8_t blockNumber, uint8_t *data)
{
    // Prepare the command
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      // Target number
    pn532_packetbuffer[2] = PN532_MIFARE_READ;      // Mifare Read command = 0x30
    pn532_packetbuffer[3] = blockNumber;            // Block Number (0..63 for 1K, 0..255 for 4K)

    sendCommand(pn532_packetbuffer, 4);
}
/**
    Authenticates a block.
    @param uid The array of UIDs.
    @param uidLen The number of UIDs.
    @param blockNumber The block number.
    @param keyNumber The key number.
    @param keyData The key data.
*/
int8_t authenticateBlock (uint8_t uid[], uint8_t uidLen, uint8_t blockNumber, uint8_t keyNumber, uint8_t keyData[])
{
    uint8_t i;
//		uint8_t result[16];
	
    // Prepare the authentication command
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;   // Data Exchange Header
    pn532_packetbuffer[1] = 1;                      // Max card numbers
    pn532_packetbuffer[2] = (keyNumber) ? PN532_AUTH_WITH_KEYB : PN532_AUTH_WITH_KEYA;
    pn532_packetbuffer[3] = blockNumber;            // Block Number (1K = 0..63, 4K = 0..255)
    pn532_packetbuffer[4] = keyData[0];
    pn532_packetbuffer[5] = keyData[1];
    pn532_packetbuffer[6] = keyData[2];
    pn532_packetbuffer[7] = keyData[3];
    pn532_packetbuffer[8] = keyData[4];
    pn532_packetbuffer[9] = keyData[5];

    for (i = 0; i < uidLen; i++) {
        pn532_packetbuffer[10 + i] = uid[i]; // 4 byte card ID
    }

    sendCommand(pn532_packetbuffer, 10 + uidLen);

//    if (result[0] == 0x14 || result[0] == 0x27) {
//        return 0;
//    }
    return 0;
}
/**
    Gets the firmware version.
*/
uint16_t getFirmwareVersion(void)
{
		uint16_t ver = 0;

    pn532_packetbuffer[0] = PN532_FIRMWAREVERSION;
    sendCommand(pn532_packetbuffer, 1);
		delay_ms(100);
		
		if((USART1_RX_BUF[5] == 0xd5)&&(USART1_RX_BUF[7] == 0x32))
		{
			  ver =  (USART1_RX_BUF[8]<<8)|USART1_RX_BUF[9];
				return ver;
		}
		else
			  return 0;
}
/**
    Finds and reads all available NFC tags.
    @param cardbaudrate The BAUD rate.
					PN532_MIFARE_ISO14443A     0x00
					PN532_MIFARE_ACTIVE        0x01
					PN532_MIFARE_FELICA_COMMUN 0x10
					PN532_MIFARE_ISO_IEC       0x11
    @param uid The UID of the NFC tag.
*/    
void readPassiveTargetID(uint8_t cardbaudrate)
{
    if(!IF_Start_Find){
			pn532_packetbuffer[0] = PN532_INLISTPASSIVETARGET;
			pn532_packetbuffer[1] = 1; // max 1 cards at once
			pn532_packetbuffer[2] = cardbaudrate;
			
				sendCommand(pn532_packetbuffer, 3);
				delay_ms(10);
				IF_Start_Find = 1; 
    }
    else;
     //发送信号量 开始查询； 
//    uid[0] = data[13];
//    uid[1] = data[14];
//    uid[2] = data[15];
//    uid[3] = data[16];
}
/**
    Finds and authenticates an NFC target.
    @param block The block to authenticate.
*/
int8_t findTargetAndAuth(uint8_t block)
{
    uint8_t uidLength = 4;
    uint8_t uid[] = {0x0, 0x0, 0x0, 0x0};
		uint8_t key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
		
    readPassiveTargetID(PN532_MIFARE_ISO14443A);
    

    return authenticateBlock(uid, uidLength, block, 0, key);
}
/**
    Writes a string to the NFC tag.
    @param The string to be written.
*/
uint8_t writeStringToNFC(char string[])
{
		uint8_t data[16];
		int8_t i;
	
    if (findTargetAndAuth(block_1)) {
        
        for (i = 0; i < 16; i++) {
            data[i] = string[i];
        }
        writeDataBlock (block_1, data);
        if (findTargetAndAuth(block_2)) {
            
            for (i = 16; i < 32; i++) {
                data[i - 16] = string[i];
            }
            writeDataBlock (block_2, data);
            return 1;
        }
    }
    return 0;
}
////配置 106 kbps type A (ISO/IEC14443 Type A),
////00 00 FF 04 FC D4 4A 01 00 E1 00
//void  nfc_InListPassiveTarget(void)
//{
//    uint8_t data[11];
//    uint8_t i;
//    uint8_t CheckCode=0; //数据校验码
//    uint8_t temp=0;
//    while(1)
//    {   
//        data[0]=0x00;
//        data[1]=0x00;
//        data[2]=0xFF;
//        data[3]=0x04; //包 长度
//        data[4]=0xFC; //包 长度 校验  0x100-data[3]
//        data[5]=0xD4; //命令标识码
//        data[6]=0x4A; //命令标识码
//        data[7]=0x01;
//        data[8]=0x00;
//        data[9]=0xE1; //数据 校验   0x100-
//        data[10]=0x00;
//        			
//			  UART1_Send(sizeof(data),data);
//					
//        delay_ms(680); 
//        
//        temp =0;
//        for(i=11;i<23;i++)
//				{
//						temp+=USART1_RX_BUF[i];
//				}
//				CheckCode=0x100-temp;
//				if(CheckCode==USART1_RX_BUF[23])
//				{
//					UID[0]=USART1_RX_BUF[19];
//					UID[1]=USART1_RX_BUF[20];
//					UID[2]=USART1_RX_BUF[21];
//					UID[3]=USART1_RX_BUF[22];  
//					
//					printf("UID:");
//					for(i=0;i<4;i++)
//						printf("%02x ",UID[i] ); 
//					printf("\r\n");
//					
//					CleanBuffer(40);//清除 串口接收缓冲区前30 个字节数据
//					RxCounter=0;  // wenxue 
//					break;
//				}
//    }
//}
//// 密码授权，验证KeyA
////00 00 FF 0F F1 D4 40 01 60 03 FF FF FF FF FF FF UID1 UID2 UID3 UID4 2A 00
////00 00 FF 0F F1 D4 40 01 60 03 FF FF FF FF FF FF 94 8A 3B 0B 2A 00
//void  nfc_PsdVerifyKeyA(void)
//{
//    uint8_t data[22];
//    uint8_t temp=0; 
//    uint8_t i;
//    uint8_t CheckCode=0; //数据校验码
//   
//    data[0]=0x00;
//    data[1]=0x00;
//    data[2]=0xFF;
//    
//    data[3]=0x0F; //包 长度
//    data[4]=0xF1; //包 长度 校验  0x100-data[3]
//    
//    data[5]=0xD4; //命令标识码
//    data[6]=0x40; //命令标识码
//    
//    data[7]=0x01;
//    data[8]=0x60;
//    data[9]=0x03; 
//    
//    data[10]=0xFF; //KEY A 密码 FF FF FF FF FF FF
//    data[11]=0xFF;
//    data[12]=0xFF;
//    data[13]=0xFF;
//    data[14]=0xFF;
//    data[15]=0xFF;
//    
//    data[16]=UID[0];
//    data[17]=UID[1];
//    data[18]=UID[2];
//    data[19]=UID[3];
//    
//    for(i=5;i<20;i++)
//    {
//        temp+=data[i];
//    }

//    data[20]=0x100-temp;   //数据 校验   0x100-
//    
//    data[21]=0x00;    
//     
//		UART1_Send(sizeof(data),data);  
//    
//    
//    delay_ms(180);
//   
//    temp=0;
//    for(i=11;i<14;i++)
//    {
//        temp+=USART1_RX_BUF[i];
//    }
//    CheckCode=0x100-temp;
//    if(CheckCode==USART1_RX_BUF[14])
//    {
//       if(USART1_RX_BUF[13]==0x00)
//       {
//         CleanBuffer(40);//清除 串口接收缓冲区前40 个字节数据
//         RxCounter=0;  // wenxue 
//				 printf("Verify success!\r\n");

//       }
//    }
//}




////默认 读 02区的16个字节
////00 00 FF 05 FB D4 40 01 30 02 B9 00
//void  nfc_read()
//{
//    uint8_t data[12];
//    uint8_t temp=0; 
//    uint8_t i;
//    uint8_t CheckCode=0; //数据校验码

//   
//    data[0]=0x00;
//    data[1]=0x00;
//    data[2]=0xFF;
//    
//    data[3]=0x05; //包 长度
//    data[4]=0xFB; //包 长度 校验  0x100-data[3]
//    
//    data[5]=0xD4; //命令标识码
//    data[6]=0x40; //命令标识码
//    
//    data[7]=0x01;
//    data[8]=0x30;
//    data[9]=0x02; //读第二块的16字节数据 
//    
//    data[10]=0xB9; //数据校验
//    data[11]=0x00;

//		
//		UART1_Send(sizeof(data),data);  
//    
//    delay_ms(680);
//  
//    temp=0;
//    for(i=11;i<30;i++)
//    {
//        temp+=USART1_RX_BUF[i];
//    }
//    CheckCode=0x100-temp;
//    if(CheckCode==USART1_RX_BUF[30])
//    {       

//         printf("Read Data:");				  
//			   for(i=14;i<30;i++)
//				  	printf("%02x ",USART1_RX_BUF[i] ); 
//				 printf("\r\n");
//			
//         CleanBuffer(40);//清除 串口接收缓冲区前40 个字节数据
//		   	RxCounter=0;  // wenxue 
//    }
//}



////默认往 02区写 16个字节的第一个字节
////00 00 FF 15 EB D4 40 01 A0 02 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F D1 00
//void  nfc_write(uint8_t buf[16])
//{
//    uint8_t data[28];
//    uint8_t temp=0;
//    uint8_t i;
//    uint8_t CheckCode=0;
//    
//    data[0]=0x00;
//    data[1]=0x00;
//    data[2]=0xFF;
//    
//    data[3]=0x15; //包 长度
//    data[4]=0xEB; //包 长度 校验  0x100-data[3]
//    
//    data[5]=0xD4; //命令标识码
//    data[6]=0x40; //命令标识码
//    
//    data[7]=0x01; //读写大于6字节 就置1，看手册
//    data[8]=0xA0; //写
//    data[9]=0x02; //写第二块的16字节数据 
//    
//    data[10]=buf[0]; //第 1 字节 数据
//    data[11]=buf[1];
//    data[12]=buf[2]; //第 3 字节 数据
//    data[13]=buf[3];
//    data[14]=buf[4]; //第 5 字节 数据
//    data[15]=buf[5];
//    data[16]=buf[6]; //第 7 字节 数据
//    data[17]=buf[7];
//    data[18]=buf[8]; //第 9 字节 数据
//    data[19]=buf[9];
//    data[20]=buf[10]; //第 11 字节 数据
//    data[21]=buf[11];
//    data[22]=buf[12]; //第 13 字节 数据
//    data[23]=buf[13];
//    data[24]=buf[14]; //第 15 字节 数据
//    data[25]=buf[15];
//    
//		 temp=0;
//    for(i=5;i<26;i++)
//    {
//        temp+=data[i];
//    }
//    data[26]=0x100-temp; //数据 校验码
//    data[27]=0x00;

//    
//		UART1_Send(sizeof(data),data);  
//		
//    delay_ms(680);

//  //00 00 FF 00 FF 00 00 00 FF 03 FD D5 41 00 EA 00
//    temp=0;
//    for(i=11;i<14;i++)
//    {
//        temp+=USART1_RX_BUF[i];
//    }
//    CheckCode=0x100-temp;
//    if(CheckCode==USART1_RX_BUF[14])
//    {
//        
//       printf("Write Data success!\r\n");		
//			 CleanBuffer(40);//清除 串口接收缓冲区前40 个字节数据
//			 RxCounter=0;  // wenxue 
//         

//    }
//}


void CleanBuffer(uint8_t *buf,uint16_t num)//清零 前 多少个字节的数据
{
    uint16_t i=0;
    
    for(i=0;i<num;i++)
      buf[i]=0x00;
}

