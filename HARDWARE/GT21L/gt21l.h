#ifndef __GT21L_H
#define __GT21L_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
#define RFCS_A     PAout(7)
#define RFCS_B     PBout(3)
#define GDO0_A     PBout(0)
#define GDO0_B     PBout(4)
#define GDO2_A     PBout(1)
#define GDO2_B     PBout(5)
//  在GT21L16S2Y芯片中数据的地址
//------------------------------------------------------
#define ASC0808D2HZ_ADDR  ( 0x66c0 ) 		//7*8 ascii code
#define ASC0812M2ZF_ADDR  ( 0x66d40 )   		//6*12 ascii code
#define GBEX0816ZF_ADDR   243648   	   	//8*16 ascii code

#define ZF1112B2ZF_ADDR ( 0x3cf80 )	   	//12*12 12点字符
#define HZ1112B2HZ_ADDR  ( 0x3cf80+376*24 )	//12*12 12点汉字


#define CUTS1516ZF_ADDR  0x00  				//16*16 16点字符
#define JFLS1516HZ_ADDR  27072  			//16*16 16点汉字
////////////////////////////////////////////////////////////////////////////////// 
void GT21_Init(void);

u8 WriteGT21Data(u8 value);
	
u8 ReadGT21Data(void);

u8 ReadGTDot(u32 addr,u8 *dotdata,u8 len);

void GT21_HWSPI_Init(void) ;

u8 SPI2_ReadWriteByte(u8 TxData);

u8 SPI_Read_Buf(u8 reg,u8 *pBuf,u8 len);

u8 SPI_Write_Buf(u8 reg, u8 *pBuf, u8 len);

void TestGT21(void);
#endif
