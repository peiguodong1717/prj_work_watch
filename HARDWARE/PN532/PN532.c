/*
    The NFC driver.
*/

#include "usart.h"
#include "myiic.h"
#include "delay.h"
#include "string.h"

#define PN532_PACKBUFFSIZ 64
uint8_t pn532_packetbuffer[PN532_PACKBUFFSIZ];

uint8_t block_1 = 48;
uint8_t block_2 = 49;

int8_t pn532ack[] = {0x01, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};


/**
    Initializes the NFC module.
*/
void PN532_init(void)
{
    RF_IIC_Start();
		RF_IIC_Send_Byte(PN532_DEVICE_ADDR);
		if(RF_IIC_Wait_Ack())
			return;	
		RF_IIC_Stop();
		delay_ms(500);
    // send dummy command to wake board up
    getFirmwareVersion();
    setPassiveActivationRetries(0xFF);

//    // configure board to read RFID tags
    SAMConfig();
}
/**
    Writes on the I2C bus.
    @param buf The buffer to be written.
    @param count How many times to try.
*/
int i2c_Write(unsigned char *buf, unsigned count)
{
    while (count--) {
			RF_IIC_Send_Byte(*buf++);
			if(RF_IIC_Wait_Ack())
				RF_IIC_Stop();		 
    }
    return 0;
}
void PN532_test(uint8_t addr)
{
	  uint8_t buffer[13] = {0x00,0x00,0xff,0x05,0xfb,0xd4,0x00,0x00,0x00,0x00,0x2c,0x00,0x00};
		uint8_t statusbyte[70];
		uint8_t i;
		
    RF_IIC_Start();
		RF_IIC_Send_Byte(PN532_DEVICE_ADDR);
		if(RF_IIC_Wait_Ack())
			return;	
		RF_IIC_Stop();
		
		delay_ms(500); 
		
    RF_IIC_Start();
		RF_IIC_Send_Byte(PN532_DEVICE_ADDR);
		if(RF_IIC_Wait_Ack())
			return;	
	  i2c_Write(buffer, 13);
    RF_IIC_Stop();
		
		delay_ms(2);
		
		readACKFrame(PN532_DEVICE_ADDR+1);
		RF_IIC_Stop();
		delay_ms(20);
		readResponseFrames(PN532_DEVICE_ADDR+1, statusbyte);
		delay_ms(20);
}
/**
    Reads an ACK (acknowledge) frame.
    @param addr The address to be read from.
*/
uint8_t readACKFrame(int8_t addr)
{
    char ack_readbytes[6];
    uint8_t i = 0,startbyte,readACK_Flag;
	  const char ack_frame[6] = {0x00,0x00,0xff,0x00,0xff,0x00};

		RF_IIC_Start();
		RF_IIC_Send_Byte(addr);
		RF_IIC_Wait_Ack();
		
		readACK_Flag = 0;
		
		startbyte = RF_IIC_Read_Byte(IIC_Ask);
		if(startbyte==0x01)
		{
			for(i=0;i<5;i++) {
				ack_readbytes[i] = RF_IIC_Read_Byte(IIC_Ask);
			}	
		}
		else
			readACK_Flag = 1;
		ack_readbytes[i] = RF_IIC_Read_Byte(IIC_NoAsk);
		if(strcmp(ack_readbytes, ack_frame))
			readACK_Flag = 1;
    RF_IIC_Stop();
		
		return readACK_Flag;	
}


/**
    Sends one or more commands to the NFC module.
    @param command The array of commands.
    @param cmdlen The number of commands.
    @param addr The address to be read from.
*/
void sendCommand(uint8_t command[], int8_t cmdlen, int8_t addr)
{
    unsigned int checksum;
		unsigned int cmdlen_1;
		unsigned int checksum_1;
		unsigned char i2c_messages[128];
//		unsigned char statusbyte;
    uint16_t size;
	  uint8_t i,j;
    
		size = 8 + cmdlen;
    cmdlen++;
    checksum = PN532_PREAMBLE + PN532_STARTCODE1 + PN532_STARTCODE2;
	
    // Preampel and Startcode
    i2c_messages[0] = PN532_PREAMBLE;
    i2c_messages[1] = PN532_STARTCODE1;
    i2c_messages[2] = PN532_STARTCODE2;
    
    // Length = TFI + PD0 to PDn
    i2c_messages[3] = cmdlen;

    // Length Checksum LCS = Lower byte of [LEN + LCS] = 00
    cmdlen_1 = ~cmdlen + 1;
    i2c_messages[4] = cmdlen_1;
    
    // TFI
    i2c_messages[5] = PN532_HOSTTOPN532;
    checksum += PN532_HOSTTOPN532;

    j = 6;
    for (i = 0; i < cmdlen - 1; i++) {
        i2c_messages[j] = command[i];
        checksum += command[i];
        j++;
    }

    // Data Checksum DCS = Lower byte of [TFI + PD0 + PD1 + ? + PDn + DCS] = 00
    checksum_1 = ~checksum;
    i2c_messages[j] = checksum_1;
    
    // Postampel
    i2c_messages[j + 1] = PN532_POSTAMBLE;

	  RF_IIC_Start();
		RF_IIC_Send_Byte(addr);
		RF_IIC_Wait_Ack();
    i2c_Write(i2c_messages, size);
    RF_IIC_Stop();
		
		delay_ms(2);
		
//    RF_IIC_Start();
//		RF_IIC_Send_Byte(PN532_DEVICE_ADDR + 1);
//		RF_IIC_Wait_Ack();
//		RF_IIC_Read_Byte(IIC_NoAsk);
//    RF_IIC_Wait_Ack();
//    RF_IIC_Stop();
}


/**
    Pauses execution for a bit.
    This function is called to delay the program execution to give the NFC module time to... do things.
*/
void NFC_delay(void)
{
    int i,x = 0;
    for (i = 0; i < 59999; i++) {
        x++;
    }
}

///**
//    Handles timeouts and interrupts.
//*/
//static void wait_for_SI(void)
//{
//    uint16_t timeout = 16000;
//    while (timeout-- && (PBin(4)));
//}
/**
    Reads response frames.
    @param addr The address to be read from.
    @param result The result. This is written to by the function.
*/
void readResponseFrames(uint8_t addr, uint8_t *result)
{
		uint8_t response_1[8];
		uint8_t response_2[256];
		uint16_t i = 0;
		uint16_t j = 0;
	
    NFC_delay();
		RF_IIC_Start();
    RF_IIC_Send_Byte(addr);
		if(RF_IIC_Wait_Ack()) 
			return;

 
    while (i < 8) {
			response_1[i] = RF_IIC_Read_Byte(IIC_Ask);
        i++;
    }

    if (response_1[1] != 0 && response_1[2] != 0) {
        return;
    }

    //response_2[response_1[4]];
    
		if(response_1[4]>=40) return;
		
    i = 0;
    while(i < response_1[4] - 1) {
        response_2[i] = RF_IIC_Read_Byte(IIC_Ask);
        if (i != response_1[4] - 2) {
            result[j++] = response_2[i];
        }
        i++;
    }

    response_2[i] = RF_IIC_Read_Byte(IIC_NoAsk);
    RF_IIC_Stop();
}


/**
    Finds and reads all available NFC tags.
    @param cardbaudrate The BAUD rate.
    @param uid The UID of the NFC tag.
*/    
void readPassiveTargetID(uint8_t cardbaudrate, uint8_t *uid)
{
		uint8_t data[20];
	
    pn532_packetbuffer[0] = PN532_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1; // max 1 cards at once
    pn532_packetbuffer[2] = cardbaudrate;

    sendCommand(pn532_packetbuffer, 3, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    
    readResponseFrames(PN532_DEVICE_ADDR + 1, data);
    uid[0] = data[6];
    uid[1] = data[7];
    uid[2] = data[8];
    uid[3] = data[9];
}


/**
    Sets passive activation entries.
    @param maxRetries The meximum number of retries.
*/
void setPassiveActivationRetries(uint8_t maxRetries)
{
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
    pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
    pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
    pn532_packetbuffer[4] = maxRetries;

    sendCommand(pn532_packetbuffer, 5, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
}


/**
    Configures the NFC module as an NFC reader.
*/
void SAMConfig(void)
{
    pn532_packetbuffer[0] = PN532_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01; // normal mode;
    pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01; // use IRQ pin!

    sendCommand(pn532_packetbuffer, 4, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
}


/**
    Reads one reponse frame.
    @param addr The address to be read from.
*/
void readResponseFrame(uint8_t addr)
{
    uint8_t result[40];
    readResponseFrames(addr, result);
}


/**
    Writes a data block.
    @param blockNumber The block number.
    @param data The data to be written.
*/
void writeDataBlock (uint8_t blockNumber, uint8_t data[])
{
		uint8_t i;
    // Prepare the first command
    pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                      // Card number
    pn532_packetbuffer[2] = PN532_MIFARE_WRITE;     // Mifare Write command = 0xA0
    pn532_packetbuffer[3] = blockNumber;            // Block Number (0..63 for 1K, 0..255 for 4K)
    
    for (i = 0; i < 16; i++) {
        pn532_packetbuffer[i + 4] = data[i];
    }

    sendCommand(pn532_packetbuffer, 20, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
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

    sendCommand(pn532_packetbuffer, 4, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);
    readResponseFrames(PN532_DEVICE_ADDR + 1, data);
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
		uint8_t result[16];
	
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

    sendCommand(pn532_packetbuffer, 10 + uidLen, PN532_DEVICE_ADDR);
    readACKFrame(PN532_DEVICE_ADDR + 1);

    readResponseFrames(PN532_DEVICE_ADDR + 1, result);

    if (result[0] == 0x14 || result[0] == 0x27) {
        return 0;
    }
    return 1;
}


/**
    Gets the firmware version.
*/
void getFirmwareVersion(void)
{
    pn532_packetbuffer[0] = PN532_FIRMWAREVERSION;
    sendCommand(pn532_packetbuffer, 1, PN532_DEVICE_ADDR);
//	  delay_ms(2);
    readACKFrame(PN532_DEVICE_ADDR + 1);
//	  delay_ms(2);
    readResponseFrame(PN532_DEVICE_ADDR + 1);
}


/**
    Writes a string to the NFC tag.
    @param The string to be written.
*/
int8_t writeStringToNFC(char string[])
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


/**
    Reads a string from the NFC tag.
    @param string The string to be read. This is written to by the function.
*/
int8_t readStringFromNFC(char *string)
{
    uint8_t data[17];

    if (findTargetAndAuth(block_1)) {
        readDataBlock(block_1, data);
        convertDataToString(data, string, 0);
        if (findTargetAndAuth(block_2)) {
            readDataBlock(block_2, data);
            convertDataToString(data, string, 16);
            return 1;
        }
    }
    return 0;
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
		
    readPassiveTargetID(PN532_MIFARE_ISO14443A, uid);
    

    return authenticateBlock(uid, uidLength, block, 0, key);
}


/**
    Converts data to a string.
    @param data The data to be converted.
    @param string The string that contains the result. This is written to by the function.
    @param start Where to start converting.
*/
void convertDataToString(uint8_t data[], char *string, int8_t start)
{
		int8_t i;
	
    for (i = start; i < 16 + start; i++) {
        string[i] = data[i + 1 - start];
    }
}


////KEY_INT中断(PA15)处理函数
//void EXTI4_IRQHandler(void)
//{
//   
//   EXTI_ClearITPendingBit(GPIO_PinSource4);//清除中断标志
//}
