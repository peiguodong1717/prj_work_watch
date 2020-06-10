#include "led.h" 
#include "delay.h" 





void LED_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure;        //EXTI��ʼ���ṹ����
//  NVIC_InitTypeDef NVIC_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);
	
//  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  AFIO->MAPR&=0XF8FFFFFF;
  AFIO->MAPR|=2<<24;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);                //��ʼ��

	GPIO_SetBits(GPIOA,GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6);
	GPIO_SetBits(GPIOB,GPIO_Pin_8);
  GPIO_ResetBits(GPIOB,GPIO_Pin_9);

//  //ʹ��GPIO�жϹ���
//  EXTI_ClearITPendingBit(EXTI_Line0);//����жϱ�־
//  GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);//�ܽ�ѡ��
//  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�¼�ѡ��
//  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//����ģʽ
//  EXTI_InitStructure.EXTI_Line = EXTI_Line0; //��·ѡ��
//  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//�����ж�
//  EXTI_Init(&EXTI_InitStructure);//��ʼ��

//  //�ж�����
//  NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;       //ͨ��
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//ռ�ȼ�
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;   //��Ӧ��
//  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      //����
//  NVIC_Init(&NVIC_InitStructure);              //��ʼ��	

}

void Buzzer(uint8_t time)
{
	uint8 i;
	Beep = 1;
	for (i=0;i<time;i++) delay_us(1000);
  Beep = 0;
}




//void EXTI0_IRQHandler(void)
//{
//#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
//	OSIntEnter();    
//#endif
//  if(EXTI_GetITStatus(EXTI_Line4)!= RESET)
//	{
//    DispString(0,0,"EXTI0_IRQHandler",0);
//		EXTI_ClearFlag(EXTI_Line0);   
//		EXTI_ClearITPendingBit(EXTI_Line0);
//	}
//#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
//	OSIntExit();  											 
//#endif  
//}
