#ifndef __24CXX_H
#define __24CXX_H
#include "myiic.h"   				  
//////////////////////////////////////////////////////////////////////////////////

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	  8191
#define AT24C128	16383
#define AT24C256	32767  

#define EE_TYPE  AT24C256

//存储器分配
#define    SETBASE    30000    //max:32768
#define    AT24CTEST	SETBASE		//测试存储读写是否正常的地址;			
#define    APOINT     AT24CTEST+1      //A频点
#define    BPOINT     APOINT+1  //B频点
#define    H_BAG      BPOINT+1  //心跳包
#define    PASSWORD   H_BAG+1   //密码位置
#define    UNITCOUNT  PASSWORD+6 //注册设备数量
#define    UNITLIST   UNITCOUNT+1 //注册列表信息  地址+时间+启用状态			 
#define    CIRCLE     UNITLIST+1  //周期
#define    MINSIGNAL  CIRCLE+1    //最小信号值   
#define    AUTOASK 	  MINSIGNAL+1 //自动询终端	  
#define    ASKDELAY	  AUTOASK+1 //延时等终端返回;
#define    DEBUGMODE	  ASKDELAY+1//调试模式;
#define    COMPORT	  DEBUGMODE+1//用于通讯的默认串口(哪个串口调试基站，就保存哪个串口);
#define    ASKTERMLIST	COMPORT+1//终端列表,128*3个字节;
#define		 AUTOCLEAR		ASKTERMLIST+390 //自动清理不在线终端;
#define		 BLANCERUNTYPE	AUTOCLEAR+1  //平衡系统运行模式 0无1半自动2全自动;
#define		 BLANCESECOND		BLANCERUNTYPE+1 //平衡系统时长2字节;
#define    BLANCEPLACECOUNT		BLANCESECOND+2 //需在线位置数;
#define		 POWERA		BLANCEPLACECOUNT+1 //A模块功率;  
#define		 POWERB		POWERA+1 //B模块功率;  
#define		 OTHERSTATE		POWERB+1 //其他状态;  
	
/* Variables' number */
#define NumbOfVar               ((uint8_t)10)

u8 AT24CXX_ReadOneByte(u16 ReadAddr);							
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);					
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead); 

u8 AT24CXX_Check(void);  
void AT24CXX_Init(void); 
#endif
















