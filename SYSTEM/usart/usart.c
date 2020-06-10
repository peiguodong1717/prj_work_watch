#include "sys.h"
#include "usart.h"	
////////////////////////////////////////////////////////////////////////////////// 	 
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos Ê¹ÓÃ	  
#endif
//////////////////////////////////////////////////////////////////////////////////	   
 
#define USE_USART_DMA_RX 1
//////////////////////////////////////////////////////////////////  
#if 1
#pragma import(__use_no_semihosting)                             
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;        
_sys_exit(int x) 
{ 
	x = x; 
} 

int fputc(int ch, FILE *f)
{ 
	if(USART_Fpt == 1){
		while((USART1->SR&0X40)==0){};  
		USART1->DR = (u8) ch; 
	}
	else if(USART_Fpt == 2){
		while((USART2->SR&0X40)==0){};  
		USART2->DR = (u8) ch; 
	}
	else{
		while((USART3->SR&0X40)==0){};  
		USART3->DR = (u8) ch; 
	}
	return ch;
}
#endif 


u8 USART1_RX_BUF[USART1_REC_LEN];     
u8 USART2_RX_BUF[USART2_REC_LEN]; 
u8 USART3_RX_BUF[USART3_REC_LEN];
u8 USART1_CRC;
u8 USART2_CRC;

u8 USART_Fpt = 1;

u16 USART1_RX_STA=0;       
u16 USART2_RX_STA=0;       
u16 USART3_RX_STA=0;


#if USE_USART_DMA_RX

void uart1dma_init(u32 bound)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO|RCC_APB2Periph_USART1, ENABLE);	

 	USART_DeInit(USART1);  
	DMA_DeInit(DMA1_Channel5); 
	
//	GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);
	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
////	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);


//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);  
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART1_RX_BUF;  
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
	DMA_InitStructure.DMA_BufferSize = USART1_REC_LEN;  
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
	DMA_Init(DMA1_Channel5,&DMA_InitStructure);  
  DMA_Cmd(DMA1_Channel5,ENABLE); 


  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Init(USART1, &USART_InitStructure);
	
	USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);
	USART_ITConfig(USART1,USART_IT_TC,DISABLE); 
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);  
	
	USART_Cmd(USART1, ENABLE);                    
	USART_ClearFlag(USART1, USART_FLAG_TC);

}
void uart2dma_init(u32 bound)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

 	USART_DeInit(USART2);  
	DMA_DeInit(DMA1_Channel6); 
 //USART2_TX   PA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
 
	//USART2_RX	  PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);  
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART2_RX_BUF;  
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
	DMA_InitStructure.DMA_BufferSize = USART2_REC_LEN;  
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
	DMA_Init(DMA1_Channel6,&DMA_InitStructure);  
  DMA_Cmd(DMA1_Channel6,ENABLE); 


  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Init(USART2, &USART_InitStructure); 
	
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);
	USART_ITConfig(USART2,USART_IT_TC,DISABLE); 
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);  
	
	USART_Cmd(USART2, ENABLE);                   
	USART_ClearFlag(USART2, USART_FLAG_TC);
}

void uart3dma_init(u32 bound)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

 	USART_DeInit(USART3);  
	DMA_DeInit(DMA1_Channel3); 
 //USART2_TX   PB.10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
 
	//USART2_RX	  PB.11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);  
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART3_RX_BUF;  
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
	DMA_InitStructure.DMA_BufferSize = USART3_REC_LEN;  
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
	DMA_Init(DMA1_Channel3,&DMA_InitStructure);  
  DMA_Cmd(DMA1_Channel3,ENABLE); 


  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Init(USART3, &USART_InitStructure); 
	
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
	USART_ITConfig(USART3,USART_IT_IDLE,ENABLE);
	USART_ITConfig(USART3,USART_IT_TC,DISABLE); 
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);  
	
	USART_Cmd(USART3, ENABLE);                   
	USART_ClearFlag(USART3, USART_FLAG_TC);
}

void UART1_Send(u8 len,u8 buf[])
{ 
	u8 i;
	for(i=0;i<len;i++){
		while((USART1->SR&0X40)==0){};  
		USART1->DR = buf[i]; 
	}
}

void UART2_Send(u8 len,u8 buf[])
{ 
	u8 i;
	for(i=0;i<len;i++){
		while((USART2->SR&0X40)==0){};  
		USART2->DR = buf[i]; 
	}
}

void UART3_Send(u8 len,u8 buf[])
{ 
	u8 i;
	for(i=0;i<len;i++){
		while((USART3->SR&0X40)==0){};  
		USART3->DR = buf[i]; 
	}
}

void USART1_IRQHandler(void)                                 
{      
	uint16_t buff_length;
#ifdef OS_TICKS_PER_SEC	 	//Èç¹ûÊ±ÖÓ½ÚÅÄÊı¶¨ÒåÁË,ËµÃ÷ÒªÊ¹ÓÃucosIIÁË.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  
	{  
			DMA_Cmd(DMA1_Channel5,DISABLE); 
			/* æ¸…DMAæ ‡å¿—ä½ */
			DMA_ClearFlag( DMA1_FLAG_TC5 );  
			/* è·å–æ¥æ”¶åˆ°çš„æ•°æ®é•¿åº¦ å•ä½ä¸ºå­—èŠ*/
      buff_length = USART1_REC_LEN - DMA_GetCurrDataCounter(DMA1_Channel5);
		  USART1_RX_STA = buff_length;
			/* é‡æ–°èµ‹å€¼è®¡æ•°å€¼ï¼Œå¿…é¡»å¤§äºç­‰äºæœ€å¤§å¯èƒ½æ¥æ”¶åˆ°çš„æ•°æ®å¸§æ•°ç›® */
			DMA_SetCurrDataCounter(DMA1_Channel5,USART1_REC_LEN);
			//printf("%s",USART1_RX_BUF);
      //printf("len:%d:%s\n",buff_length,USART1_RX_BUF);
		  USART_ReceiveData(USART1); 
			USART_ClearFlag(USART1,USART_IT_IDLE);	
			DMA_Cmd(DMA1_Channel5,ENABLE);  
	}
	__nop(); 
#ifdef OS_TICKS_PER_SEC	 	//Èç¹ûÊ±ÖÓ½ÚÅÄÊı¶¨ÒåÁË,ËµÃ÷ÒªÊ¹ÓÃucosIIÁË.
	OSIntExit();  											 
#endif  
}   

void USART2_IRQHandler(void)                                 
{ 
	uint16_t buff_length;	
#ifdef OS_TICKS_PER_SEC	 	//Èç¹ûÊ±ÖÓ½ÚÅÄÊı¶¨ÒåÁË,ËµÃ÷ÒªÊ¹ÓÃucosIIÁË.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)  
	{  		
			DMA_Cmd(DMA1_Channel6,DISABLE); 
			/* æ¸…DMAæ ‡å¿—ä½ */
			DMA_ClearFlag( DMA1_FLAG_TC5 );  
			/*·å–æ¥æ”¶åˆ°çš„æ•°æ®é•¿åº¦ å•ä½ä¸*/
      buff_length = USART2_REC_LEN - DMA_GetCurrDataCounter(DMA1_Channel6);
			USART2_RX_STA = buff_length;
		  /* é‡æ–°èµ‹å€¼è®¡æ•°å€¼ï¼Œå¿…é¡»å¤§äºç­‰äºæœ€å¤§å¯èƒ½æ¥æ”¶åˆ°çš„æ•°æ®å¸§æ•°ç›® */
			DMA_SetCurrDataCounter(DMA1_Channel6,USART2_REC_LEN);
			//printf("%s",USART1_RX_BUF);
			//printf("len:%d:%s\n",buff_length,USART2_RX_BUF);
		  USART_ReceiveData(USART2); 
			USART_ClearFlag(USART2,USART_IT_IDLE);	
			DMA_Cmd(DMA1_Channel6,ENABLE); 
	}         
	__nop();
#ifdef OS_TICKS_PER_SEC	 	//Èç¹ûÊ±ÖÓ½ÚÅÄÊı¶¨ÒåÁË,ËµÃ÷ÒªÊ¹ÓÃucosIIÁË.
	OSIntExit();  											 
#endif   
}
void USART3_IRQHandler(void)                                 
{      
	uint16_t buff_length;
#ifdef OS_TICKS_PER_SEC	 	//Èç¹ûÊ±ÖÓ½ÚÅÄÊı¶¨ÒåÁË,ËµÃ÷ÒªÊ¹ÓÃucosIIÁË.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)  
	{  
			DMA_Cmd(DMA1_Channel3,DISABLE); 
			/* æ¸…DMAæ ‡å¿—ä½ */
			DMA_ClearFlag( DMA1_FLAG_TC5 );  
      /*  */
      buff_length = USART3_REC_LEN - DMA_GetCurrDataCounter(DMA1_Channel3);
		  USART3_RX_STA = buff_length;
			/* é‡æ–°èµ‹å€¼è®¡æ•°å€¼ï¼Œå¿…é¡»å¤§äºç­‰äºæœ€å¤§å¯èƒ½æ¥æ”¶åˆ°çš„æ•°æ®å¸§æ•°ç›® */
			DMA_SetCurrDataCounter(DMA1_Channel3,USART3_REC_LEN);
			//printf("%s",USART1_RX_BUF);
			//printf("len:%d:%s\n",buff_length,USART3_RX_BUF);
		  USART_ReceiveData(USART3); 
			USART_ClearFlag(USART3,USART_IT_IDLE);	
			DMA_Cmd(DMA1_Channel3,ENABLE); 
	}         
	__nop(); 
#ifdef OS_TICKS_PER_SEC	 	//Èç¹ûÊ±ÖÓ½ÚÅÄÊı¶¨ÒåÁË,ËµÃ÷ÒªÊ¹ÓÃucosIIÁË.
	OSIntExit();  											 
#endif  
}
#else
//void uart1_init(u32 bound)
//{
//    
//  GPIO_InitTypeDef GPIO_InitStructure;
//	USART_InitTypeDef USART_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1, ENABLE);	

// 	USART_DeInit(USART1);  
// //USART1_TX   PA.2
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
//	GPIO_Init(GPIOA, &GPIO_InitStructure); 
// 
//	//USART1_RX	  PA.3
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);  

//	USART_InitStructure.USART_BaudRate = bound;
//	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//	USART_InitStructure.USART_StopBits = USART_StopBits_1;
//	USART_InitStructure.USART_Parity = USART_Parity_No;
//	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

//  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
//	NVIC_Init(&NVIC_InitStructure);
//	
//	USART_Init(USART1, &USART_InitStructure);
//	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//	USART_Cmd(USART1, ENABLE);                   
//	USART_ClearFlag(USART1, USART_FLAG_TC);

//}

//void uart2_init(u32 bound)
//{
//   
//  GPIO_InitTypeDef GPIO_InitStructure;
//	USART_InitTypeDef USART_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
//  
// 	USART_DeInit(USART2); 
//	 //USART1_TX   PA.2
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
// 
//	//USART1_RX	  PA.3
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);  

//	USART_InitStructure.USART_BaudRate = bound;
//	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//	USART_InitStructure.USART_StopBits = USART_StopBits_1;
//	USART_InitStructure.USART_Parity = USART_Parity_No;
//	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	

//  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
//	NVIC_Init(&NVIC_InitStructure);	
//	
//	USART_Init(USART2, &USART_InitStructure);
//	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
//	USART_Cmd(USART2, ENABLE);                  
//	USART_ClearFlag(USART2, USART_FLAG_TC);
//}
#endif

unsigned short CRC_CHECK(unsigned char *Buf, unsigned char CRC_CNT)
{
    unsigned short CRC_Temp;
    unsigned char i,j;
    CRC_Temp = 0xffff;

    for (i=0;i<CRC_CNT; i++)
	 {      
        CRC_Temp ^= Buf[i];
        for (j=0;j<8;j++) 
			  {
            if (CRC_Temp & 0x01)
                CRC_Temp = (CRC_Temp >>1 ) ^ 0xa001;
            else
                CRC_Temp = CRC_Temp >> 1;
        }
    }
    return(CRC_Temp);
}
 
