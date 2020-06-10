#ifndef __PN532_HUS_H
#define __PN532_HUS_H

#include "sys.h"	

#define PN532_PACKBUFFSIZ 64

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


#define PN532_MIFARE_ISO14443A     0x00
#define PN532_MIFARE_ACTIVE        0x01
#define PN532_MIFARE_FELICA_COMMUN 0x10
#define PN532_MIFARE_ISO_IEC       0x11

#define PN532_COMMAND_RFCONFIGURATION 0x32


extern uint8_t IF_Start_Find;
extern uint8_t IF_UID[4];

void CleanBuffer(uint8_t *buf,uint16_t num);
uint8_t nfc_WakeUp(void);
void  nfc_InListPassiveTarget(void);
void  nfc_PsdVerifyKeyA(void);
void  nfc_write(uint8_t buf[16]);
void  nfc_read(void);
uint16_t getFirmwareVersion(void);
void  readPassiveTargetID(uint8_t cardbaudrate);
#endif /* __PN532_H__ */

