#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 

////////////////////////////////////////////////////////////////////////////////// 	
#define USART1_REC_LEN  			128  	
#define USART2_REC_LEN  			128  	
#define USART3_REC_LEN  			128

extern u8  USART_Fpt;
extern u8  USART1_RX_BUF[USART1_REC_LEN]; 
extern u8  USART2_RX_BUF[USART2_REC_LEN]; 
extern u16 USART1_RX_STA;         		
extern u16 USART2_RX_STA;

void uart1_init(u32 bound);
void uart1dma_init(u32 bound);
void uart2_init(u32 bound);
void uart2dma_init(u32 bound);
void uart3dma_init(u32 bound);
void UART1_Send(u8 len,u8 buf[]);
void UART2_Send(u8 len,u8 buf[]);
void UART3_Send(u8 len,u8 buf[]);
unsigned short CRC_CHECK(unsigned char *Buf, unsigned char CRC_CNT);
#endif


