#ifndef __CC2500_HPD_H
#define __CC2500_HPD_H	 
#include "sys.h"

#define 	WRITE_BURST     		0x40						//连续写入
#define 	READ_SINGLE     		0x80						//读
#define 	READ_BURST      		0xC0						//连续读
#define 	BYTES_IN_RXFIFO			0x7F  					//接收缓冲区的有效字节数
#define 	CRC_OK							0x80 						//CRC校验通过位标志

//------------------------------------------------------------------------------------------------------
// CC2500/RfTx STROBE, CONTROL AND STATUS REGSITER
#define IOCFG2       0x00        // GDO2 output pin configuration
#define IOCFG1       0x01        // GDO1 output pin configuration
#define IOCFG0       0x02        // GDO0 output pin configuration
#define FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define SYNC1        0x04        // Sync word, high INT8U
#define SYNC0        0x05        // Sync word, low INT8U
#define PKTLEN       0x06        // Packet length
#define PKTCTRL1     0x07        // Packet automation control
#define PKTCTRL0     0x08        // Packet automation control
#define ADDR         0x09        // Device address
#define CHANNR       0x0A        // Channel number
#define FSCTRL1      0x0B        // Frequency synthesizer control
#define FSCTRL0      0x0C        // Frequency synthesizer control
#define FREQ2        0x0D        // Frequency control word, high INT8U
#define FREQ1        0x0E        // Frequency control word, middle INT8U
#define FREQ0        0x0F        // Frequency control word, low INT8U
#define MDMCFG4      0x10        // Modem configuration
#define MDMCFG3      0x11        // Modem configuration
#define MDMCFG2      0x12        // Modem configuration
#define MDMCFG1      0x13        // Modem configuration
#define MDMCFG0      0x14        // Modem configuration
#define DEVIATN      0x15        // Modem deviation setting
#define MCSM2        0x16        // Main Radio Control State Machine configuration
#define MCSM1        0x17        // Main Radio Control State Machine configuration
#define MCSM0        0x18        // Main Radio Control State Machine configuration
#define FOCCFG       0x19        // Frequency Offset Compensation configuration
#define BSCFG        0x1A        // Bit Synchronization configuration
#define AGCCTRL2     0x1B        // AGC control
#define AGCCTRL1     0x1C        // AGC control
#define AGCCTRL0     0x1D        // AGC control
#define WOREVT1      0x1E        // High INT8U Event 0 timeout
#define WOREVT0      0x1F        // Low INT8U Event 0 timeout
#define WORCTRL      0x20        // Wake On Radio control
#define FREND1       0x21        // Front end RX configuration
#define FREND0       0x22        // Front end TX configuration
#define FSCAL3       0x23        // Frequency synthesizer calibration
#define FSCAL2       0x24        // Frequency synthesizer calibration
#define FSCAL1       0x25        // Frequency synthesizer calibration
#define FSCAL0       0x26        // Frequency synthesizer calibration
#define RCCTRL1      0x27        // RC oscillator configuration
#define RCCTRL0      0x28        // RC oscillator configuration
#define FSTEST       0x29        // Frequency synthesizer calibration control
#define PTEST        0x2A        // Production test
#define AGCTEST      0x2B        // AGC test
#define TEST2        0x2C        // Various test settings
#define TEST1        0x2D        // Various test settings
#define TEST0        0x2E        // Various test settings

// Strobe commands
#define SRES         0x30        // Reset chip.
#define SFSTXON      0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1).
                                 // If in RX/TX: Go to a wait state where only the synthesizer is
                                 // running (for quick RX / TX turnaround).
#define SXOFF        0x32        // Turn off crystal oscillator.
#define SCAL         0x33        // Calibrate frequency synthesizer and turn it off
                                 // (enables quick start).
#define SRX          0x34        // Enable RX. Perform calibration first if coming from IDLE and
                                 // MCSM0.FS_AUTOCAL=1.
#define STX          0x35        // In IDLE state: Enable TX. Perform calibration first if
                                 // MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:
                                 // Only go to TX if channel is clear.
#define SIDLE        0x36        // Exit RX / TX, turn off frequency synthesizer and exit
                                 // Wake-On-Radio mode if applicable.
#define SAFC         0x37        // Perform AFC adjustment of the frequency synthesizer
#define SWOR         0x38        // Start automatic RX polling sequence (Wake-on-Radio)
#define SPWD         0x39        // Enter power down mode when CSn goes high.
#define SFRX         0x3A        // Flush the RX FIFO buffer.
#define SFTX         0x3B        // Flush the TX FIFO buffer.
#define SWORRST      0x3C        // Reset real time clock.
#define SNOP         0x3D        // No operation. May be used to pad strobe commands to two
                                 // INT8Us for simpler software.

#define PARTNUM      0x30
#define VERSION      0x31
#define FREQEST      0x32
#define LQI          0x33
#define RSSI         0x34
#define MARCSTATE    0x35
#define WORTIME1     0x36
#define WORTIME0     0x37
#define PKTSTATUS    0x38
#define VCO_VC_DAC   0x39
#define TXBYTES      0x3A
#define RXBYTES      0x3B

#define PATABLE      0x3E
#define TXFIFO       0x3F
#define RXFIFO       0x3F

/****************************************************************************************/
//全局变量定义
/****************************************************************************************/



//#define CS_PORT       GPIOA
//#define SCK_PORT      GPIOB
//#define SDI_PORT      GPIOA
//#define SDO_PORT      GPIOA
//#define IRQ_PORT      GPIOB

//#define CS_PIN				GPIO_Pin_10	
//#define SCK_PIN				GPIO_Pin_3	
//#define SDI_PIN				GPIO_Pin_11	
//#define SDO_PIN				GPIO_Pin_12	
//#define IRQ_PIN				GPIO_Pin_4	

//#define CS_Pnum					10	
//#define SCK_Pnum				3	
//#define SDI_Pnum				11	
//#define SDO_Pnum				12	
//#define IRQ_Pnum				4	

//#define CC2500_CS      PAout(15)
//#define CC2500_GDO0    PAin(12)
//#define CC2500_GDO1    PAin(11)

#define	SpiSetCsHigh()			GPIO_SetBits(GPIOA,GPIO_Pin_15)		
#define	SpiSetCsLow()		  GPIO_ResetBits(GPIOA,GPIO_Pin_15)


#define	SpiGetGdo0()		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12)		

void Init_RfRx(void);
void CC2500RxProc( void );
void CC2500TxProc( void );

#endif



