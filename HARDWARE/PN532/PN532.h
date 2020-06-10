/*
    The NFC driver.
*/
#include "sys.h"

#ifndef _PN532_H
#define _PN532_H

#define PN532_PREAMBLE 0x00
#define PN532_STARTCODE1 0x00
#define PN532_STARTCODE2 0xFF
#define PN532_POSTAMBLE 0x00

#define PN532_HOSTTOPN532 0xD4

#define PN532_FIRMWAREVERSION 0x02
#define PN532_POWERDOWN 0x16
#define PN532_GETGENERALSTATUS 0x04
#define PN532_SAMCONFIGURATION  0x14
#define PN532_INLISTPASSIVETARGET 0x4A
#define PN532_INDATAEXCHANGE 0x40
#define PN532_MIFARE_READ 0x30
#define PN532_MIFARE_WRITE 0xA0

#define PN532_AUTH_WITH_KEYA 0x60
#define PN532_AUTH_WITH_KEYB 0x61


#define PN532_WAKEUP 0x55

#define PN532_SPI_STATREAD 0x02
#define PN532_SPI_DATAWRITE 0x01
#define PN532_SPI_DATAREAD 0x03
#define PN532_SPI_READY 0x01

#define PN532_DEVICE_ADDR  0x48


#define PN532_MIFARE_ISO14443A     0x00
#define PN532_MIFARE_ACTIVE        0x01
#define PN532_MIFARE_FELICA_COMMUN 0x10
#define PN532_MIFARE_ISO_IEC       0x11

#define PN532_COMMAND_RFCONFIGURATION 0x32


void PN532_test(uint8_t addr);


void PN532_init(void);

uint8_t readACKFrame(int8_t addr);

void sendCommand(uint8_t command[], int8_t cmdlen, int8_t addr);

void NFC_delay(void);

void readResponseFrames(uint8_t addr, uint8_t *result);
    
void readPassiveTargetID(uint8_t cardbaudrate, uint8_t *uid);

void setPassiveActivationRetries(uint8_t maxRetries);

void SAMConfig(void);

void readResponseFrame(uint8_t addr);

void writeDataBlock (uint8_t blockNumber, uint8_t data[]);

void readDataBlock(uint8_t blockNumber, uint8_t *data);

int8_t authenticateBlock (uint8_t uid[], uint8_t uidLen,
    uint8_t blockNumber, uint8_t keyNumber, uint8_t keyData[]);

void getFirmwareVersion(void);

int8_t writeStringToNFC(char string[]);

int8_t readStringFromNFC(char *string);

int8_t findTargetAndAuth(uint8_t block);

void convertDataToString(uint8_t data[], char *string, int8_t start);

int i2c_Write(unsigned char *buf, unsigned count);

#endif
