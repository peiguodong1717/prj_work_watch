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

//�洢������
#define    SETBASE    30000    //max:32768
#define    AT24CTEST	SETBASE		//���Դ洢��д�Ƿ������ĵ�ַ;			
#define    APOINT     AT24CTEST+1      //AƵ��
#define    BPOINT     APOINT+1  //BƵ��
#define    H_BAG      BPOINT+1  //������
#define    PASSWORD   H_BAG+1   //����λ��
#define    UNITCOUNT  PASSWORD+6 //ע���豸����
#define    UNITLIST   UNITCOUNT+1 //ע���б���Ϣ  ��ַ+ʱ��+����״̬			 
#define    CIRCLE     UNITLIST+1  //����
#define    MINSIGNAL  CIRCLE+1    //��С�ź�ֵ   
#define    AUTOASK 	  MINSIGNAL+1 //�Զ�ѯ�ն�	  
#define    ASKDELAY	  AUTOASK+1 //��ʱ���ն˷���;
#define    DEBUGMODE	  ASKDELAY+1//����ģʽ;
#define    COMPORT	  DEBUGMODE+1//����ͨѶ��Ĭ�ϴ���(�ĸ����ڵ��Ի�վ���ͱ����ĸ�����);
#define    ASKTERMLIST	COMPORT+1//�ն��б�,128*3���ֽ�;
#define		 AUTOCLEAR		ASKTERMLIST+390 //�Զ����������ն�;
#define		 BLANCERUNTYPE	AUTOCLEAR+1  //ƽ��ϵͳ����ģʽ 0��1���Զ�2ȫ�Զ�;
#define		 BLANCESECOND		BLANCERUNTYPE+1 //ƽ��ϵͳʱ��2�ֽ�;
#define    BLANCEPLACECOUNT		BLANCESECOND+2 //������λ����;
#define		 POWERA		BLANCEPLACECOUNT+1 //Aģ�鹦��;  
#define		 POWERB		POWERA+1 //Bģ�鹦��;  
#define		 OTHERSTATE		POWERB+1 //����״̬;  
	
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
















