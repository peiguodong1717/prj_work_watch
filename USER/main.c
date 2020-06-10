#include "led.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "pcf8563.h"
#include "24cxx.h"
#include "cc2500.h"  
#include "includes.h"  

//����ģ�鶨��;
#define RF_A							1
#define RF_B							0
//���ȴ�10*1000us
#define MaxWaitCount      30
#define MaxCommError      32
#define BPointMaxWait			60
#define MaxTermCount			128
//����15����߳�
#define MaxWaitSecond			15
//����Ա��״̬λ;
#define READY							1
#define ONLINE 						2
#define HAVEEMP						3

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			10 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				  64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	

//������
//�����������ȼ�
#define MAIN_TASK_PRIO       			8 
//���������ջ��С
#define MAIN_STK_SIZE  					1024
//�����ջ	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//������
void main_task(void *pdata);


//free
//�����������ȼ�
#define FREE_TASK_PRIO       			7 
//���������ջ��С
#define FREE_STK_SIZE  					64
//�����ջ	
OS_STK FREE_TASK_STK[FREE_STK_SIZE];
//������
void free_task(void *pdata);

//-----------------------------------------�����Զ��忪ʼ---------------------------------------;


uint8 CC2500Data[2][MAXLEN];//����ģ����յ�����;
uint8 CC2500Count[2];//���ݳ���;

//ȫ�ֱ���;
uint8 CurUartNo=0x01;//��ǰͨѶ���ڣ�Ĭ��Ϊ����
uint8_t chark;//�����Լ�;
void ResetCC2500Data(uint8 channel);//���CC2500����;
uint8 ExplainCC2500(uint8 channel);//���ռ������;					 
void TestSetTime(void);//��������ʱ��;
void RefreshTime(void);//ˢ��ʱ��;
void WriteSetting(void);//��ȡ����;
void ReadSetting(void);//��������;
void AskTerm(void);//ѭ�ն�;
uint8 CheckPc(void);//��鲢����Э��;
int SeekTerm(uint16 paramno);//��ȡ�ն˺����;
void ClearTermCmd(int index);//�����������;
void ClearAllTermCmd(void);//������л�������;
void SendHeartBag(void);//����������;
void ClearInvalidTerm(u8 AskTermIndex);//���������ն�;
void SaveTerm(int index);//����ָ�����ն������ŵ��洢;
void DeleteTerm(void);//��������ն��б�;
void CheckBlanceState(void);//������ϵͳ;
void SendBlanceLight(int paramTermNo);//�㲥ƽ��ϵͳ��״̬;
void ClearTermStauts(int paramCount);//�������״̬;
void BlanceOver(void);//�������;
void sme1_read(void);//��ȡ����ģ������1;
void sme2_read(void);//��ȡ����ģ������2;
void SendOnLineToPC(void);//�����ն�������λ��

//-------------ȫ�ֱ���----------------//
uint16 FreeCount=0,SendTimeOut=0;//��
uint8 AskWaitOver=0,AtSendCmd=0;//ȫ�ֵĵȴ����,�жϽ��մ�����ɣ�����ý����ȴ�;					 
uint8 Apoint,Bpoint,HeartEnable,AutoAsk,Circle,MinSignal,AskDelay,DebugMode;//AutoAskΪ�Զ���ѭ��־
uint8 AutoClear=1;//�Զ�������Ч�ն�
uint8 AskTermIndex,AskTermCount,AskStart,AskPcSign,HeartBagCount;//,AskTermIndexΪ��ǰλ��,AskTermCountΪ���ն���,AskStartΪ��ͣ��ѭ��־,AskPcSign��ʱѯ��PC�Ƿ������
uint16 AskTermList[MaxTermCount];//��ѭ�б����128���ն�,AskWaitCount�ȴ�������
uint8 AskTermErrCount[MaxTermCount][2];//��ѭ����,Ҫ�߳��б�,����������;
uint8 AskTermCmd[MaxTermCount][MAXLEN];//���ϴ����48���ֽ�;
uint8 AskTermCmdLen[MaxTermCount];//���ϴ������;
uint8 AskTermStatus[MaxTermCount][4];//0:��ʾ�Ƿ���Ҫȡ״̬;1:ƽ��ϵͳ״̬;2:�ڲ�����;3:�Ƿ���Ա��;
uint8 RelayData[MAXLEN],RelayLen,RelayCount=0;//ת������;
uint8 CheckDataOK(uint8* data,uint8 len);////У�������Ƿ�׼ȷ;
//ȫ�ֱ���;
uint8 CurYear,CurMonth,CurDay,CurWeek,CurHour,CurMinute,CurSecond;//��ǰʱ����;
uint16 BlanceSecond=0x00,BlanceRunSec=0x00;//ƽ��ϵͳ����ʱ��;
uint8 BlanceRunType=0x00,NeedSendBlance=0x00,BlancePlaceCount=0x00;//Ĭ��Ϊ�ֶ�,������;
uint8 PowerA=0xFF,PowerB=0xFF;
uint16 BPointWait=0x00;
uint8	SendOnLine=0x00;

//-----------------------------------------�����Զ������---------------------------------------;
//////////////////////////////////////////////////////////////////////////////
    
//OS_EVENT * msg_key;			  //���������¼���	  
//OS_EVENT * q_msg;			    //��Ϣ����
OS_TMR   * tmr1;			    //�����ʱ��1
OS_TMR   * tmr2;			    //�����ʱ��1
OS_EVENT * RF1_user;
OS_EVENT * RF2_user;
//�����ʱ��1�Ļص�����	
//ÿ1sִ��һ��,������ʾCPUʹ���ʺ��ڴ�ʹ����		   
void tmr1_callback(OS_TMR *ptmr,void *p_arg) 
{	
	  RefreshTime();//һ��һ��;
	  //���ձ���;
	  if (BPointWait<=BPointMaxWait) {
			BPointWait++;
		}
		//���������ն�;
		if (SendOnLine<0xFF)	SendOnLine++;
	  //���Զ�����ʱ�Ŵ���
	  if ((BlanceRunType==2)&&(BlanceSecond>0)){
			BlanceRunSec++;
			if (BlanceRunSec>=BlanceSecond){
				NeedSendBlance=0x01;
				BlanceRunSec=0;
			}
		}
 	  //�������ʱ(10��û�ؾ���Ϊ��ʱ�����ط�)
	  if (AtSendCmd==0x01) SendTimeOut++;
	  if (SendTimeOut>10) {
			SendTimeOut=0x00;
			AtSendCmd=0x00;
	  };
	  //��ѭ״̬���ն�ʱ
		if (AutoAsk==0x01) {
			FreeCount++;
			//��Ҫ����վ������,��ʾ����;
	    if (FreeCount>15) {
				AskPcSign=1;
				FreeCount=0x00;
			}
		};
		//����������5��1��;
		if (HeartBagCount<0xFF) HeartBagCount++;	
}
//�����ʱ��1�Ļص�����	
//ÿ100msִ��һ��,������ʾCPUʹ���ʺ��ڴ�ʹ����		   
void tmr2_callback(OS_TMR *ptmr,void *p_arg) 
{   
    delay_ms(100);	   
} 
			


int main(void)
{	 
  u8 comport;
 	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	LED_Init();
	IIC_Init();
	uart1dma_init(115200);
	uart2dma_init(115200);
	uart3dma_init(115200);
	TIM2_Init(100,7199);
	TIM3_Init(500,7199);
	TIM4_Init(1000,7199);
  HWSPI_Init();
	Buzzer(30);//���30ms;
	//��ȫ��
	LEDA_Tx=0x00;
	LEDA_Rx=0x00;
	LEDB_Tx=0x00;
	LEDB_Rx=0x00;
  if(CC_Init(RF_A))         chark|=0x02;
	if(CC_Init(RF_B))         chark|=0x04;
	if(AT24CXX_Check())    chark|=0x08;
	if(PCF8563_Get_Year() == 18);
	else  PCF8563_init();
  util_readStm32UniqueCode();
  ReadSetting();//��ȡ����
	//���÷��书��;
	CC_PaTable(PowerA,RF_A);
	delay_ms(200);
  CC_PaTable(PowerB,RF_B);
	delay_ms(200);
	//��ȫ��
	LEDA_Tx=!LEDA_Tx;
	LEDA_Rx=!LEDA_Rx;
	LEDB_Tx=!LEDB_Tx;
	LEDB_Rx=!LEDB_Rx;
	//˫���ڣ���ȡ����ͨѶ��Ĭ�ϴ���
	comport=AT24CXX_ReadOneByte(COMPORT);
	if ((comport>=1)||(comport<=3)) {
		CurUartNo=comport;
	}
	OSInit();  	 			//��ʼ��UCOSII		  
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	


}							    
//��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;  	    
 	RF1_user=OSSemCreate(0); 	//����SPI�ź���
  RF2_user=OSSemCreate(0); 	//����RFID�ź���	  	  
	OSStatInit();					//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)     				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);
	OSTaskCreate(free_task,(void *)0,(OS_STK*)&FREE_TASK_STK[FREE_STK_SIZE-1],FREE_TASK_PRIO);				    				   
 	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}
//LED����


//������
void main_task(void *pdata)
{							 
	u8 i,err,tbyte[MAXLEN],waitSecond;
	u16 AskWaitCount=0;
	OS_CPU_SR cpu_sr=0; 
  //������ʱ��;
 	tmr1=OSTmrCreate(10,100,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,(INT8U *)"tmr1",&err);   //1sִ��һ��	
	OSTmrStart(tmr1,&err);//���������ʱ��1		
	tmr2=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr2_callback,0,(INT8U *)"tmr2",&err);	//100msִ��һ��
  //OSTmrStart(tmr2,&err);//���������ʱ��1			 
	IWDG_Init(4,625);     //���Ź���ʼ��1s;
	CC_Chan(Apoint,RF_A);
	CC_Chan(Bpoint,RF_B);	
	AskWaitCount=MaxWaitCount;
	AskWaitOver=0;
	AtSendCmd=0x00;
	FreeCount=0x00;
	//����
	ResetCC2500Data(RF_A);
	ResetCC2500Data(RF_B);
	//�����ź����
	if (DebugMode==0x01){
		tbyte[0]=0xEE;
		tbyte[1]=CC_RssiCh(Rssi[RF_A]);
		tbyte[2]=CC_RssiCh(Rssi[RF_B]);;
		tbyte[3]=GDO2_A;
		tbyte[4]=GDO2_B ;
		tbyte[5]=0xFF;
		if (CurUartNo==0x01) UART1_Send(6,tbyte);
		if (CurUartNo==0x02) UART2_Send(6,tbyte);
		delay_ms(30);
	}
	//�ý���ģʽ;
	CC2500_RxOn(RF_A);
	CC2500_RxOn(RF_B);
	//������ն���Ҫ��ѭ��ֱ�ӿ���
	if (AskTermCount>0) AskStart=0x01;
 	while(1)
	{	
		IWDG_Feed();//ι��������
		//��ʱ����յ����ͼ�������;
		if ((AskWaitCount>=MaxWaitCount)|| AskWaitOver){
			//����Ƿ���Ҫ������Ч�ն�;
			if ((AskStart==1)&&(AskPcSign!=0x01)&&(AskTermCount>0)&&(AskTermCount<=MaxTermCount)){
				OSSchedLock();
				//��Ӧ��λ
				if (AskWaitOver==0x01) {
					AskTermErrCount[AskTermIndex][0]=0x00;
					AskTermErrCount[AskTermIndex][1]=0x00;
					AskTermStatus[AskTermIndex][ONLINE]=0x00;//����;					
				}
				else {					
					//��Ӧ���ۼ�,����һ��������Ч;	
          if (AskTermErrCount[AskTermIndex][0]==0x00) {
						AskTermErrCount[AskTermIndex][1]=CurSecond;
					}
					//������������
					if (AskTermErrCount[AskTermIndex][0]<=MaxCommError) AskTermErrCount[AskTermIndex][0]++;
					//�������ж��Ƿ�ʱ
					if (AskTermErrCount[AskTermIndex][0]>MaxCommError){
						if (CurSecond<AskTermErrCount[AskTermIndex][1]) waitSecond=CurSecond+60-AskTermErrCount[AskTermIndex][1];
						else waitSecond=CurSecond-AskTermErrCount[AskTermIndex][1];
						//���������ն�;
						if (waitSecond>=MaxWaitSecond) {
							if (AutoClear==1) ClearInvalidTerm(AskTermIndex);
							else AskTermStatus[AskTermIndex][ONLINE]=0xEE;//������;
						}
					}
				};
	      OSSchedUnlock();				
			}
			//5�뷢��������,�����ź�ǿ��;
			if (HeartEnable==0x01){
				if ((HeartBagCount>5)&&(RelayLen==0)) {
					SendHeartBag();		
				}
			}
			//������ʽ��ʼ;
			delay_us(500);	//��ʱ1000us,����λ���ֵý���״̬;
			//�Ƿ�Ҫ��λBģ��;
			if (BPointWait>=BPointMaxWait){
				BPointWait=0x00;			
				CC2500_RxOn(RF_B);
			}
			OSSchedLock();
			AskWaitOver=0;
      OSSchedUnlock();
			//���û���նˣ�Ҳ����Ҫ��ʱ��;
			if (AskTermCount==0) AskWaitCount=0;
			//�������������ѭ;
			if ((CC2500Count[RF_A]==0)&&(CC2500Count[RF_B]==0)&&(RelayLen==0)) {
				//ѭ��һ�ն�
				OSSchedLock();
				AskTermIndex++;
				//�ж��Ƿ�Ϊ�б��е����һ�ն�,�������ص���һ��
				if (AskTermIndex>=AskTermCount) AskTermIndex=0;	
				OSSchedUnlock();			
				//�ն���ʱ;
				if (AskWaitCount<5) {
					err=10-AskWaitCount;
					delay_ms(err);
				};
				//��λ�ȴ�ʱ�����;
				AskWaitCount=0;				
				//ѯ�ն�����;
				AskTerm();
				//��ʱ;
		    delay_us(500);			
			}
		}			
		//����Ƿ���������Ҫ����;
		if (SerOK[RF_A]) sme1_read();
		if (SerOK[RF_B]) sme2_read();
		//�������Ҫ����;
		if (AtSendCmd==0) {
			ExplainCC2500(RF_A);
		  ExplainCC2500(RF_B);
			//�ж��Ƿ�Ҫ�㲥ƽ��ϵͳ�ĵ�״̬
			if (BlanceRunType==0x01) CheckBlanceState();
			if (NeedSendBlance==0x01){
				//���͵�״̬;
				OSSchedLock();
				NeedSendBlance=0x00;
				OSSchedUnlock();
				//���͵�״̬����
				SendBlanceLight(0);
				delay_ms(10);
				SendBlanceLight(0);
				delay_ms(10);
				SendBlanceLight(0);
				delay_ms(10);
				//�����־;
				ClearTermStauts(AskTermCount);
				//���������λ��
				BlanceOver();
			}
		}
		//�����λ��;
		if (CheckPc()==100){
       __set_FAULTMASK(1);//�ر����ж�
       NVIC_SystemReset();//����Ƭ������
		}
		//��ת��������ת��
		if (RelayLen>0){
			LEDA_Tx=0x00;
			LEDB_Tx=0x00;
			OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)   
			if ((RelayCount<5)&&(RelayLen<MAXLEN)){
				for(i=0;i<RelayLen;i++) tbyte[i]=RelayData[i];
				RelayCount++;//���ת��5��;
				CC_SendPacket(tbyte,RelayLen,RF_A);	
			} else{
				//��λ;
				RelayLen=0;
				RelayCount=0;
			}
			OS_EXIT_CRITICAL();			//�˳��ٽ���(�޷����жϴ��)   
			delay_ms(5);			
			LEDA_Tx=0x01;
			LEDB_Tx=0x01;
		}
		//������ڷ�������,���ϴ�һ�������û�;
		if ((SendOnLine>0xB4)&&(AtSendCmd==0)){
			SendOnLine=0;
			SendOnLineToPC();
		}
		//��ʱ1000us;
		delay_us(AskDelay*200);
		//�ȴ���һ��;
		OSSchedLock();
		AskWaitCount++;
		OSSchedUnlock();
	}
	
}		
//�����ն�������λ��
void SendOnLineToPC(void){
	u8 i,data[MAXLEN];
	u16 checksum;
	data[0]=0xa3;
	data[1]=0xa3;
	data[2]=0xff;
	data[3]=0xff;
	data[4]=0xf0;
	data[5]=0xAA;
	data[6]=Apoint;
	data[7]=Bpoint;
	data[8]=HeartEnable;
	data[9]=Circle;
	data[10]=MinSignal;
	data[11]=AutoAsk;
	data[12]=AskDelay;
	data[13]=AskTermCount;
	for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
	data[14]=checksum>>8;
	data[15]=checksum;
	if (CurUartNo==0x01) UART1_Send(16,data);
	if (CurUartNo==0x02) UART2_Send(16,data);
	//AtSendCmd=0x01;
	delay_ms(10);	
}
//���ƽ��ϵͳ;
void CheckBlanceState(){
	uint16 i,readyCount;
	for (readyCount=0,i=0;i<AskTermCount;i++){
    //ֻ���������豸
		if (AskTermStatus[AskTermIndex][ONLINE]!=0xEE){
			if ((AskTermStatus[i][READY]==0x01)&&(AskTermStatus[i][HAVEEMP]==0x01)) readyCount++; //����豸
		}
	}	
	//ȫ�����������;
	if (AskTermCount>0){
		if ((readyCount>=BlancePlaceCount)||(readyCount>=AskTermCount)){
			OSSchedLock();
			NeedSendBlance=0x01;
			OSSchedUnlock();
		}
	}
}

//�㲥ƽ��ϵͳ�ĵ�״̬;
void SendBlanceLight(int paramTermNo)
{
	OS_CPU_SR cpu_sr=0;
  uint8  sendBuf[20];
  uint8  i;
  uint16 checksum;
	LEDA_Tx=0x00;
	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)
	sendBuf[0]=0xB5;
	sendBuf[1]=paramTermNo>>8;
	sendBuf[2]=paramTermNo;
	sendBuf[3]=0x0D;//������;
	sendBuf[4]=0x08;//�����
	sendBuf[5]=0;//��λ״̬;
	for(checksum=0,i=0;i<6;i++)
		checksum=checksum+sendBuf[i];
	sendBuf[6]=checksum>>8;
	sendBuf[7]=checksum;
	//ͨ��AƵ����ѭ
	CC_SendPacket(sendBuf,0x08,RF_A);
	OS_EXIT_CRITICAL();			//�����ٽ���(�޷����жϴ��)
	delay_ms(5);
	LEDA_Tx=0x01;	
}

//ƽ�����ϵͳ��ʼ;
void BlanceOver(void){
	uint8 data[32],i;
	uint16 checksum;
  data[0]=0xa3;
	data[1]=0xa3;
	data[2]=0xff;
	data[3]=0xff;
	data[4]=0xf0;
	data[5]=0xFC;
	data[6]=BlanceRunType;
	data[7]=BlanceSecond>>8;
	data[8]=BlanceSecond;
	data[9]=BlanceRunSec>>8;
	data[10]=BlanceRunSec;
	data[11]=0x00;
	data[12]=0x00;
	data[13]=0x00;
	for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
	data[14]=checksum>>8;
	data[15]=checksum;
	if (CurUartNo==0x01) UART1_Send(16,data);
	if (CurUartNo==0x02) UART2_Send(16,data);
	delay_ms(30);
}

//��ȡ����A
void sme1_read(void){
	u8 i,needupload=0;
	needupload=0;//Ĭ�ϲ���Ҫ�ϴ���������;
	LEDA_Rx=0x00;
	CC_RdPacket(RF_A);
	//�����ݴ���;
	if (SerLen[RF_A]>5) 
	{		
		//У������;
		if (CheckDataOK(SerData[RF_A],SerLen[RF_A])==0x01)
		{
			//�ж��Ƿ���Ҫ���ת��״̬,����ѭӦ��ģ�һ��Ϊת��Ӧ��;
			if ((RelayLen>0)&&(SerData[RF_A][0]==0xB3)&&(SerData[RF_A][3]!=0xFF)&&(SerData[RF_A][3]!=0xFE)) RelayLen=0;
			//�������ѭ������,���ݴ������¶�Ӧ
			if (AskTermCount>0)
			{
				if ((SerData[RF_A][0]==0xB3)||(SerData[RF_A][0]==0xC3)||(SerData[RF_A][0]==0xD3))
				{
					//��ַ��ͬ,��ʾ�ն˻ش���;
					if (AskTermList[AskTermIndex]==(SerData[RF_A][1]<<8|SerData[RF_A][2]))
					{
						//�ն˷���PC��ص�����
						if (SerData[RF_A][0]==0xB3){
							//��¼���ϴ���PC������Ϊ0x21��ʾ�м�¼��0x07��ʾû��¼;
							if ((SerData[RF_A][3]==0xFF)&&(SerData[RF_A][4]==0x21)){														
								//������ϴ������У����ݴ����ֱ�Ӵ���;
								if (AtSendCmd==1){
									//�����ݴ浽��Ӧ��������
									AskTermCmdLen[AskTermIndex]=SerLen[RF_A];
									//�����ݵ��ݴ�;
									for(i=0;i<SerLen[RF_A];i++)
									{
										AskTermCmd[AskTermIndex][i]=SerData[RF_A][i];
									}
								} else needupload=1;
							} else{
								//�޼�¼ʱ������;
								if (SerData[RF_A][3]==0xFE)	{
										AskTermStatus[AskTermIndex][HAVEEMP]=SerData[RF_A][5];//Ա���������;
										AskTermStatus[AskTermIndex][READY]=SerData[RF_A][6];//������������ɲ�������Ҫ�ж�
								}
							}
						}
						//ƽ��SerDataֱ�Ӹ���״̬;
						if (SerData[RF_A][0]==0xC3){
							needupload=1;										
						}
						//�����ݻظ�����ȴ�;
						if (SerData[RF_A][0]==0xD3){
							ClearTermCmd(AskTermIndex);
						}						
					}
				}
			} else{
				//��¼���ϴ���PC,��Ĳ���Ҫ����;
				if ((SerData[RF_A][0]==0xB3)&&((SerData[RF_A][3]==0xFF)||(SerData[RF_A][3]==0xFE))) {
						needupload=1;//�ڷǲɼ�ģʽ��ֱ���ϴ���PC					
				};
				//���Ķ�ȡ�ϴ���PC,��Ĳ���Ҫ;
				if ((SerData[RF_A][0]==0xC3)&&(SerData[RF_A][3]==0xFA)) {
						needupload=1;//�ڷǲɼ�ģʽ��ֱ���ϴ���PC					
				};					
			}
			//��ŵ�ʵʱ�ϴ�����
			if (needupload){ 
				//�����ݵ��ݴ�;
				for(i=0;i<SerLen[RF_A];i++)
				{
					CC2500Data[RF_A][i]=SerData[RF_A][i];
				}	
				CC2500Count[RF_A]=SerLen[RF_A];
			}
			//��Ӧ��ɽ�����һ����ѭ;
			AskWaitOver=1;
		}
	}
	//���
	SerOK[RF_A]=0x00;
	Clean_Rx_buffer(SerLen[RF_A],RF_A);
  CC2500_RxOn(RF_A);	
	LEDA_Rx=!LEDA_Rx;	
}

//��ȡ����A
void sme2_read(void){
u8 i,cmdlen,needupload=0,returnOK=0,sendDelay=20;
	uint16 checksum;
	uint8 heartdata[32];
	LEDB_Rx=0x00;
	BPointWait=0x00;
	CC_RdPacket(RF_B);
	//�����ݴ���;
	if (SerLen[RF_B]>0) 
	{
			if (CheckDataOK(SerData[RF_B],SerLen[RF_B])==0x01)
			{
				//����Ƿ���ѯ��վ�� 0xB0 UnitAddrH,UnitAddrL,0xFF,CmdLen,CheckSumH,CheckSumL;
				if (SerData[RF_B][0]==0xB0)
				{
					cmdlen=14;//�����;
					heartdata[0]=0xA0;
					heartdata[1]=SerData[RF_B][1];
					heartdata[2]=SerData[RF_B][2];
					heartdata[3]=0xFF;//����;
					heartdata[4]=cmdlen;//����;
					heartdata[5]=Apoint; //����
					heartdata[6]=Bpoint;
					heartdata[7]=MinSignal;
					heartdata[8]=Circle;
					heartdata[9]=CC_RssiCh(Rssi[RF_B]); 
					heartdata[10]=AskTermCount;
					heartdata[11]=AutoAsk;    
					for(checksum=0,i=0;i<cmdlen-2;i++)
						 checksum=checksum+heartdata[i];
					heartdata[cmdlen-2]=checksum>>8;
					heartdata[cmdlen-1]=checksum;
					LEDB_Tx=0x00;
					delay_ms(sendDelay);
					CC_SendPacket(heartdata,cmdlen,RF_B); 
					delay_ms(5);
					CC_SendPacket(heartdata,cmdlen,RF_B); 
					LEDB_Tx=0x01;
				} else
				//����ע��� 0xB1 UnitAddrH,UnitAddrL,0xFF,CmdLen,CheckSumH,CheckSumL;
				if (SerData[RF_B][0]==0xB1)
				{
					if (AskTermCount<128){
						returnOK=1;
					} else returnOK=0;
					cmdlen=10;//�����;
					heartdata[0]=0xA1;
					heartdata[1]=SerData[RF_B][1];
					heartdata[2]=SerData[RF_B][2];
					heartdata[3]=0xFF;//����;
					heartdata[4]=cmdlen;//����;
					heartdata[5]=Apoint; //����
					heartdata[6]=Bpoint;
					heartdata[7]=returnOK;//ע���Ƿ�ɹ�; 
					for(checksum=0,i=0;i<cmdlen-2;i++)
						 checksum=checksum+heartdata[i];
					heartdata[cmdlen-2]=checksum>>8;
					heartdata[cmdlen-1]=checksum;
					LEDB_Tx=0x00;
					delay_ms(sendDelay);
					CC_SendPacket(heartdata,cmdlen,RF_B); 
					delay_ms(5);
					CC_SendPacket(heartdata,cmdlen,RF_B);
					LEDB_Tx=0x01;
					//δ�ڵ�ǰ��վע����������
					if (SeekTerm(SerData[RF_B][1]<<8|SerData[RF_B][2])<0){
						OSSchedLock();
						AskTermList[AskTermCount]=(SerData[RF_B][1]<<8|SerData[RF_B][2]);
						AskTermStatus[AskTermCount][0]=0x00;
						AskTermStatus[AskTermCount][READY]=0x00;
						AskTermStatus[AskTermCount][ONLINE]=0x00;
						AskTermStatus[AskTermCount][HAVEEMP]=0x00;
						AskTermErrCount[AskTermCount][0]=0x00;
						AskTermErrCount[AskTermCount][1]=0x00;
						ClearTermCmd(AskTermCount);
						AskTermCount++;
						if (AskTermCount<128) AskTermList[AskTermCount]=0;//���б�����һ���ն˺�Ϊ0
						if (AskTermCount>=MaxTermCount) AskTermCount=0;
						OSSchedUnlock();
					};
					Buzzer(10);
					//��ע�������������ѭ;
					AskStart=0x01;
				}
				//���������Ϣ;
				if (SerData[RF_B][0]==0xB2){
					LEDB_Tx=0x00;
					//B���ϴ�����Ϊ�����ʾ����OK������ת����PC
					if (CC2500Count[RF_B]==0) {
						returnOK=1;
						needupload=0x01;//��������Ҫ�ϴ�;
					} else returnOK=0;
					cmdlen=10;//�����;
					heartdata[0]=0xA2;
					heartdata[1]=SerData[RF_B][1];
					heartdata[2]=SerData[RF_B][2];
					heartdata[3]=0xFF;//����;
					heartdata[4]=cmdlen;//����;
					heartdata[5]=Apoint; //����
					heartdata[6]=Bpoint;
					heartdata[7]=returnOK;//ע���Ƿ�ɹ�; 
					for(checksum=0,i=0;i<cmdlen-2;i++)
						 checksum=checksum+heartdata[i];
					heartdata[cmdlen-2]=checksum>>8;
					heartdata[cmdlen-1]=checksum;
					delay_ms(sendDelay);
					CC_SendPacket(heartdata,cmdlen,RF_B); 
					LEDB_Tx=0x01;

				}
				//׼������Ҫʵʱ�ϴ���PC
				if (needupload==0x01)
				{
					needupload=0;
					//�����ݵ��ݴ�;
					for(i=0;i<=SerLen[RF_B];i++)
						CC2500Data[RF_B][i]=SerData[RF_B][i];
					CC2500Count[RF_B]=SerLen[RF_B];
				}
			}
	}
	//���
	SerOK[RF_B]=0x00;
	Clean_Rx_buffer(SerLen[RF_B],RF_B);
	CC2500_RxOn(RF_B);
	LEDB_Rx=!LEDB_Rx;
}

void free_task(void *pdata)
{							 	 
 	while(1)
  {
		  OSTimeDlyHMSM(0,0,0,100);
	}
}		   

void EXTI0_IRQHandler(void)
{
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
  if(EXTI_GetITStatus(EXTI_Line0)!= RESET)
	{
    SerOK[RF_A]=0x01;
		EXTI_ClearFlag(EXTI_Line0);   
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntExit();  											 
#endif  
}

void EXTI4_IRQHandler(void)
{
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
  if(EXTI_GetITStatus(EXTI_Line4)!= RESET)
	{
    SerOK[RF_B]=0x01;
		EXTI_ClearFlag(EXTI_Line4);   
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntExit();  											 
#endif  
}






//ˢ��ʱ��;
void RefreshTime(void) 
{
	uint8 TimeBuf[7];//��ǰʱ�ӵ�ȫ�ֱ���;,2ʱ1��0�� 6��5��3�� 4����;
	//ȡʱ��
	PCF8563_Get_Time(TimeBuf);
	CurYear=TimeBuf[6];
	CurMonth=TimeBuf[5];
	CurWeek=TimeBuf[4];
	CurDay=TimeBuf[3];
	CurHour=TimeBuf[2];
	CurMinute=TimeBuf[1];
	CurSecond=TimeBuf[0];
}	

void ReadSetting(void)
{
	  u8 i,urlh,urll,eff;
    Apoint=AT24CXX_ReadOneByte(APOINT);
    Bpoint=AT24CXX_ReadOneByte(BPOINT);
    HeartEnable=AT24CXX_ReadOneByte(H_BAG);
    Circle=AT24CXX_ReadOneByte(CIRCLE);
    MinSignal=AT24CXX_ReadOneByte(MINSIGNAL);
    AutoAsk=AT24CXX_ReadOneByte(AUTOASK);
	  AskDelay=AT24CXX_ReadOneByte(ASKDELAY);
	  if (AskDelay<0x01){
			AskDelay=0x01;
			AT24CXX_WriteOneByte(ASKDELAY,AskDelay);
		}
	  DebugMode=AT24CXX_ReadOneByte(DEBUGMODE);
	  if (DebugMode>0x01) {
			DebugMode=0x00;
			AT24CXX_WriteOneByte(DEBUGMODE,DebugMode);
		}
		//�����ն��б�
		AskTermCount=0;
		for(i=0;i<128;i++){
			urlh=AT24CXX_ReadOneByte(ASKTERMLIST+i*3);
			urll=AT24CXX_ReadOneByte(ASKTERMLIST+i*3+1);
			eff=AT24CXX_ReadOneByte(ASKTERMLIST+i*3+2);
			if (eff==0x01){
				AskTermList[AskTermCount]=(urlh<<8|urll);
				AskTermStatus[AskTermCount][0]=0x00;
				AskTermStatus[AskTermCount][READY]=0x00;
				AskTermStatus[AskTermCount][ONLINE]=0x00;
				AskTermStatus[AskTermCount][HAVEEMP]=0x00;
				AskTermErrCount[AskTermCount][0]=0x00;
				AskTermErrCount[AskTermCount][1]=0x00;
				AskTermCount++;
			} else break;
		};
		//�Զ�����������ն�
		AutoClear=AT24CXX_ReadOneByte(AUTOCLEAR);
	  if (AutoClear>0x01) {
			AutoClear=0x01;//Ĭ�����;
			AT24CXX_WriteOneByte(AUTOCLEAR,AutoClear);
		}
		//��ȡƽ��ϵͳ״̬
		BlanceRunType=AT24CXX_ReadOneByte(BLANCERUNTYPE);	
	  if (BlanceRunType>0x02) {
			BlanceRunType=0x00;//Ĭ���ֶ�;
			AT24CXX_WriteOneByte(BLANCERUNTYPE,BlanceRunType);
		}		
		//������ʱ��
		urlh=AT24CXX_ReadOneByte(BLANCESECOND);
		urll=AT24CXX_ReadOneByte(BLANCESECOND+1);
		if ((urlh==0xFF)&&(urll==0xFF)) BlanceSecond=0;
    else BlanceSecond=urlh<<8 | urll;
		BlanceRunSec=0;
		//ƽ����Ҫ���ߵ�λ��
		BlancePlaceCount=AT24CXX_ReadOneByte(BLANCEPLACECOUNT);
		//���书��;
		PowerA=AT24CXX_ReadOneByte(POWERA);
		if (PowerA<0x1F){
			PowerA=0xFF;//���ǿ��
			AT24CXX_WriteOneByte(POWERA,PowerA);			
		}
		PowerB=AT24CXX_ReadOneByte(POWERB);
		if (PowerB<0x1F){
			PowerB=0xFF;//���ǿ��
			AT24CXX_WriteOneByte(POWERB,PowerB);			
		}
}
 
void WriteSetting(void)
{
   AT24CXX_WriteOneByte(APOINT,Apoint);
   AT24CXX_WriteOneByte(BPOINT,Bpoint);
   AT24CXX_WriteOneByte(H_BAG,HeartEnable);
   AT24CXX_WriteOneByte(CIRCLE,Circle);
   AT24CXX_WriteOneByte(MINSIGNAL,MinSignal);   
   AT24CXX_WriteOneByte(AUTOASK,AutoAsk);  
	 AT24CXX_WriteOneByte(ASKDELAY,AskDelay);  
	 AT24CXX_WriteOneByte(DEBUGMODE,DebugMode);  
	 AT24CXX_WriteOneByte(BLANCEPLACECOUNT,BlancePlaceCount);  
}
 
//���CC2500����;
void ResetCC2500Data(uint8 channel)
{
	uint8 i;
	CC2500Count[channel]=0x00;
	//�����ݵ��ݴ�;
	for(i=0;i<MAXLEN;i++)
	{
		CC2500Data[channel][i]=0x00;
	}
}

//���ռ������;
uint8 ExplainCC2500(uint8 channel)
{
   if (CC2500Count[channel]>0)
	 {
		 //��ʱ;
		 delay_ms(5);
     //�÷��������־;
     AtSendCmd=0x01;
		 //��������������,��Щ���ݲ���Ҫ�����PC
		 //������PC,����ģʽ����Ҫ�����PC;
		 if (CurUartNo==0x01) UART1_Send(CC2500Count[channel],CC2500Data[channel]);
		 if (CurUartNo==0x02) UART2_Send(CC2500Count[channel],CC2500Data[channel]);
		 //�������
     ResetCC2500Data(channel);
		 delay_ms(5);
	 }
	 return 1;	 
} 
 
//��������ʱ��;
void TestSetTime(void){
	uint8 paramtime[7];
	paramtime[0]=CurSecond;//��
	paramtime[1]=CurMinute;//��
	paramtime[2]=CurHour;//ʱ
	paramtime[3]=CurDay;//��
	paramtime[4]=0;//������0-6;
	paramtime[5]=CurMonth;//��
	paramtime[6]=CurYear;//��
	//����ʱ��;
	PCF8563_Set_Time(paramtime);
}

//��ȡ�ն����;
int SeekTerm(uint16 paramno){
	uint8 i;
	for (i=0;i<AskTermCount;i++){
		if (AskTermList[i]==paramno) return i;
	}
	return -1;
}


//������ѭ�б�ѯ�ն��Ƿ���Ҫ�ϴ�;
void AskTerm(void)
{
  uint8  sendBuf[32];
  uint8  data[16];
  uint8  i;
  uint16 checksum,TmpTermNo;
  //�����־Ϊ1,��ʾ��Ҫѯ�ն�
  if (AutoAsk==1)
  {
    if ((AskStart==1)&&(AskTermCount>0)&&(AskPcSign!=1)) //��ѭ��
    {
				//����ѯ�ն�����; A5 A5 00 01 FF 05 00 00 00 00 00 00 00 00 02 4F 
				TmpTermNo=AskTermList[AskTermIndex];
				if (TmpTermNo>0)
				{ 
					//��λ����;
					FreeCount=0;
					//ѭ�ն�״̬,����ƽ��ϵͳ��ÿ������ѯ��1Ϊֹ,��������0��
					 //�����ϴ�ʵ�ֹ���0xB5;
					if (AskTermCmdLen[AskTermIndex]==0x00)//�޴��ϴ�����;
				  {
						LEDA_Tx=0x00;			
						sendBuf[0]=0xB5;
						sendBuf[1]=TmpTermNo>>8;
						sendBuf[2]=TmpTermNo;
						sendBuf[3]=0xFF;//ѭ�ϴ�����
						sendBuf[4]=0x07;//Ĭ�������Ϊ07;
						for(checksum=0,i=0;i<5;i++)
							checksum=checksum+sendBuf[i];
						sendBuf[5]=checksum>>8;
						sendBuf[6]=checksum;
						//ͨ��AƵ����ѭ
						CC_SendPacket(sendBuf,0x07,RF_A);
						LEDA_Tx=0x01;	
				  } else //��������ǲ���Ҫ�������;
					{
						 if (AtSendCmd==0x00){
							 //�÷��������־;
							 AtSendCmd=0x01;				 
							 //ͨ�����ڷ�����,�з�������0;
							 if (CurUartNo==0x01) UART1_Send(AskTermCmdLen[AskTermIndex],AskTermCmd[AskTermIndex]);
							 if (CurUartNo==0x02) UART2_Send(AskTermCmdLen[AskTermIndex],AskTermCmd[AskTermIndex]);
							 delay_ms(10);
						 }
						 //ֱ�Ӳ�ѯ��һ��������ȴ�;
						 AskWaitOver=0x01;
					}
			  }
    } else if (AskPcSign==1)
    {
      //���ڸ�λ
      USART2_RX_STA=0;
      //״̬��λ
			OSSchedLock();
      AskPcSign=0;
			FreeCount=0;
      OSSchedUnlock();
      //��������PC�Ƿ��Ѵ��������֪ͨPCĿǰ�ɼ���������״̬
      data[0]=0xa3;
      data[1]=0xa3;
      data[2]=0xff;
      data[3]=0xff;
      data[4]=0xf0;
      data[5]=0xFF;
      data[6]=AskTermCount;
      data[7]=AskTermIndex;
      data[8]=AutoAsk;
      data[9]=AskStart;
      data[10]=CC2500Count[RF_A];
      data[11]=CC2500Count[RF_B];
      data[12]=RelayLen;
      data[13]=0;
      for(checksum=0,i=0;i<14;i++)
        checksum=checksum+data[i];
      data[14]=checksum>>8;
      data[15]=checksum;
			if (CurUartNo==0x01) UART1_Send(16,data);  
			if (CurUartNo==0x02) UART2_Send(16,data);  
      delay_ms(10);			
    }
  }     
}


//���������ն�;
void ClearInvalidTerm(u8 AskTermIndex){
	u8 i,j;
	//������������λ��abort
	
	//������������;
	if (AskTermCount==0) return;
	for (i=AskTermIndex;i<AskTermCount-1;i++){
		AskTermList[i]=AskTermList[i+1];
		AskTermErrCount[i][0]=AskTermErrCount[i+1][0];
		AskTermErrCount[i][1]=AskTermErrCount[i+1][1];
		AskTermCmdLen[i]=AskTermCmdLen[i+1];
		if (AskTermCmdLen[i]>0){
			for (j=0;j<48;j++) AskTermCmd[i][j]=AskTermCmd[i+1][j];
		}
	}
	AskTermList[AskTermCount]=0x00;
	AskTermErrCount[AskTermCount][0]=0x00;
	AskTermErrCount[AskTermCount][1]=0x00;
	AskTermCmdLen[AskTermCount]=0x00;
	for (j=0;j<48;j++) AskTermCmd[AskTermCount][j]=0x00;
	//�ն���-1
	AskTermCount--;
  AskTermList[AskTermCount]=0x00;
	AskTermErrCount[AskTermCount][0]=0x00;
	AskTermErrCount[AskTermCount][1]=0x00;
	AskTermCmdLen[AskTermCount]=0x00;
	for (j=0;j<48;j++) AskTermCmd[AskTermCount][j]=0x00;
	//��ǰ�ն�-1
	if (AskTermIndex>0) AskTermIndex--;
	
}

//�����λ��
uint8 CheckPc(void)
{
   uint8  recBuf[128],recLen=0,notRestSend=0x00,data[MAXLEN],stationRtn;
	 uint16 tmpno;
   uint8  uartno,i,j,AllowAdd;//AllowAddΪ�ʼ������ն˺ű�־,�����ж��ն˺��Ƿ���Ҫ�ӵ���ѭ�б���;
	 int itemindex;
   uint16 checksum;
	 //���ڽ��������ж�
	 if (USART1_RX_STA>0x00) {		 
		   uartno=1;
		   //��������;
			 recLen=USART1_RX_STA;
       for(i=0;i<USART1_RX_STA;i++)
       {
				 if (i<128) recBuf[i]=USART1_RX_BUF[i];
       }
			 USART1_RX_STA=0x00;//���ڸ�λ;
	 }	 	
	 if (USART2_RX_STA>0x00) {		 
		   uartno=2;
		   //��������;
			 recLen=USART2_RX_STA;
       for(i=0;i<USART2_RX_STA;i++)
       {
				 if (i<128) recBuf[i]=USART2_RX_BUF[i];
       }
			 USART2_RX_STA=0x00;//���ڸ�λ;
	 }
   if ((recLen>0x00)&&(recLen<MAXLEN))
   {
			 //ֻ�б�׼��վ����Ÿ��Ķ˿�;
			 if ((recBuf[0]==0xA5)&&(recBuf[1]==0xA5)) {
				 //��ǰ�˿���ʵ�����ݽ����˿ڲ�ͬ�򱣴�˿ڹ��´�����ʹ��
				 if (CurUartNo!=uartno){
					 CurUartNo=uartno;
					 AT24CXX_WriteOneByte(COMPORT,CurUartNo);
				 }
			 }
		   if (recLen>=16){
				 if((recBuf[2]==0xff)&&(recBuf[3]==0xff))  //��ַΪ0xFFFF�ɼ���������Ϊ�ն�
				 {
					   //��վ����; 
						 if(recBuf[4]==0xf0)
						 {
							  if (recBuf[5]==0x0A) //ɶ�²����������ǻظ�Ϊ������AtSendCmd״̬;
								{									
									//Ҳ���ûظ�,һ�����ڻظ�����ָ����ջ��ϴ��ն��б����;
									notRestSend=0x00;//��λ����״̬����;		
                  AskStart=0x01;//������ѭ;		
									delay_ms(10);									
								}
								else if(recBuf[5]==0x01)  //ȡ
								{
									 data[0]=0xa3;
									 data[1]=0xa3;
									 data[2]=0xff;
									 data[3]=0xff;
									 data[4]=0xf0;
									 data[5]=recBuf[5];
									 data[6]=Apoint;
									 data[7]=Bpoint;
									 data[8]=HeartEnable;
									 data[9]=Circle;
									 data[10]=MinSignal;
									 data[11]=AutoAsk;
									 data[12]=AskDelay;
									 data[13]=DebugMode;
									 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
									 data[14]=checksum>>8;
									 data[15]=checksum;
									 if (CurUartNo==0x01) UART1_Send(16,data);
									 if (CurUartNo==0x02) UART2_Send(16,data);
									 delay_ms(10);
										 
								}
								else if(recBuf[5]==0x02)  //��
								{
									 Apoint=recBuf[6];
									 Bpoint=recBuf[7];
									 HeartEnable=recBuf[8];
									 Circle=recBuf[9];
									 MinSignal=recBuf[10];
									 AutoAsk=recBuf[11];
									 AskDelay=recBuf[12];
									 DebugMode=recBuf[13];
									 OSSchedLock(); 
									 AT24CXX_WriteOneByte(APOINT,Apoint);
									 AT24CXX_WriteOneByte(BPOINT,Bpoint);
									 AT24CXX_WriteOneByte(H_BAG,HeartEnable);
									 AT24CXX_WriteOneByte(CIRCLE,Circle);
									 AT24CXX_WriteOneByte(MINSIGNAL,MinSignal);    
									 AT24CXX_WriteOneByte(AUTOASK,AutoAsk);                                                           
									 AT24CXX_WriteOneByte(ASKDELAY,AskDelay);
									 AT24CXX_WriteOneByte(DEBUGMODE,DebugMode);
									 OSSchedUnlock();
									 CC_Chan(Apoint,RF_A);
	                 CC_Chan(Bpoint,RF_B);	
									 CC2500_RxOn(RF_A);
	                 CC2500_RxOn(RF_B);
									 data[0]=0xa3;
									 data[1]=0xa3;
									 data[2]=0xff;
									 data[3]=0xff;
									 data[4]=0xf0;
									 data[5]=recBuf[5];
									 data[6]=Apoint;
									 data[7]=Bpoint;
									 data[8]=HeartEnable;
									 data[9]=Circle;
									 data[10]=MinSignal;
									 data[11]=AutoAsk;
									 data[12]=AskDelay;
									 data[13]=DebugMode;
									 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
									 data[14]=checksum>>8;
									 data[15]=checksum;
									 if (CurUartNo==0x01) UART1_Send(16,data);
									 if (CurUartNo==0x02) UART2_Send(16,data);
									 delay_ms(10);											 
								}
								else if(recBuf[5]==0x03)  //��λ
								{
									 data[0]=0xa3;
									 data[1]=0xa3;
									 data[2]=0xff;
									 data[3]=0xff;
									 data[4]=0xf0;
									 data[5]=recBuf[5];
									 data[6]=Apoint;
									 data[7]=Bpoint;
									 data[8]=HeartEnable;
									 data[9]=Circle;
									 data[10]=MinSignal;
									 data[11]=AutoAsk;
									 data[12]=AskDelay;
									 data[13]=DebugMode;
									 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
									 data[14]=checksum>>8;
									 data[15]=checksum;
									 if (CurUartNo==0x01) UART1_Send(16,data);
									 if (CurUartNo==0x02) UART2_Send(16,data);
									 delay_ms(10);
									 return 100;	            
								}
								else if(recBuf[5]==0x04)  //������ѭ�ն�
								{    
									 OSSchedLock();
									 AskStart=0;//��ͣ��ѭ
									 AllowAdd=0;//�ó�ʼֵ,�ʼƼ���;
									 //�ж��ն˺��Ƿ����,�����ھͼ�
									 tmpno=(recBuf[6]<<8)+recBuf[7];
									 if (SeekTerm(tmpno)<0) AllowAdd=1;
									 if ((AllowAdd==1)&&(AskTermCount<128))
									 {
										 AskTermList[AskTermCount]=tmpno;//���ն˺Ŵ���б���
										 AskTermStatus[AskTermCount][0]=0x00;
										 AskTermStatus[AskTermCount][READY]=0x00;
										 AskTermStatus[AskTermCount][ONLINE]=0x00;
										 AskTermStatus[AskTermCount][HAVEEMP]=0x00;
										 AskTermErrCount[AskTermCount][0]=0x00;
										 AskTermErrCount[AskTermCount][1]=0x00;
										 SaveTerm(AskTermCount);//�����б����洢
										 ClearTermCmd(AskTermCount);
										 AskTermCount++;//�ն�������1
										 if (AskTermCount<128) AskTermList[AskTermCount]=0;//���б�����һ���ն˺�Ϊ0
									 }
									 OSSchedUnlock();
									 data[0]=0xa3;
									 data[1]=0xa3;
									 data[2]=0xff;
									 data[3]=0xff;
									 data[4]=0xf0;
									 data[5]=0x04;
									 data[6]=recBuf[6];
									 data[7]=recBuf[7];
									 data[8]=AskTermCount;
									 data[9]=0;
									 data[10]=0;
									 data[11]=0;
									 data[12]=0;
									 data[13]=0;
									 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
									 data[14]=checksum>>8;
									 data[15]=checksum;
									 if (CurUartNo==0x01) UART1_Send(16,data);
									 if (CurUartNo==0x02) UART2_Send(16,data);
									 delay_ms(10);	
												
								}
								else if(recBuf[5]==0x05)  //�����ѭ�ն�
								{          
									 delay_ms(5);
									 AskStart=0;//��ͣ��ѭ
									 AskTermCount=0; //�ն���Ϊ0
									 AskTermIndex=0; //��ǰ�ն��б�����ֵΪ0
									 AskTermList[AskTermCount]=0;//���б�����һ���ն˺�Ϊ0
									 DeleteTerm();//����б�
									 data[0]=0xa3;
									 data[1]=0xa3;
									 data[2]=0xff;
									 data[3]=0xff;
									 data[4]=0xf0;
									 data[5]=recBuf[5];
									 data[6]=0;
									 data[7]=0;
									 data[8]=0;
									 data[9]=0;
									 data[10]=0;
									 data[11]=0;
									 data[12]=0;
									 data[13]=0;
									 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
									 data[14]=checksum>>8;
									 data[15]=checksum;
									 if (CurUartNo==0x01) UART1_Send(16,data);
									 if (CurUartNo==0x02) UART2_Send(16,data);
									 ClearTermStauts(128);
									 delay_ms(10);	
								}
								else if(recBuf[5]==0x06)  //������ѭ����ͣ��ѭ
								{
									 ClearAllTermCmd();
									 ClearTermStauts(AskTermCount);
									 BlanceRunSec=0;
									 delay_ms(5);
									 AskStart=recBuf[6];//0Ϊ��ͣ��ѭ,1Ϊ��ʼ��ѭ
									 data[0]=0xa3;
									 data[1]=0xa3;
									 data[2]=0xff;
									 data[3]=0xff;
									 data[4]=0xf0;
									 data[5]=recBuf[5];
									 data[6]=recBuf[6];
									 data[7]=AskTermCount;
									 data[8]=AskTermIndex;
									 data[9]=AskDelay;
									 data[10]=DebugMode;
									 data[11]=0;
									 data[12]=0;
									 data[13]=0;
									 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
									 data[14]=checksum>>8;
									 data[15]=checksum;
									 if (CurUartNo==0x01) UART1_Send(16,data);
									 if (CurUartNo==0x02) UART2_Send(16,data);
									 delay_ms(10);	

								}   
								else if(recBuf[5]==0x07)  //�鿴�ն��б�
								{
									 AskStart=0;//0Ϊ��ͣ��ѭ
									 if (AskTermCount>0) //���ն��б�
									 {
										 data[0]=0xa3;
										 data[1]=0xa3;
										 data[2]=0xff;
										 data[3]=0xff;
										 data[4]=0xf0;
										 data[5]=0x07;                         
										 for (j=0;j<AskTermCount;j++)
										 { 
											 checksum=AskTermList[j]; 
											 data[6]=checksum>>8;
											 data[7]=checksum;                             
											 data[8]=j+1;
											 data[9]=AskTermErrCount[j][0];
											 data[10]=AskTermErrCount[j][1];
											 data[11]=AskTermCmdLen[j];
											 data[12]=0;
											 data[13]=0;
											 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
											 data[14]=checksum>>8;
											 data[15]=checksum;
											 if (CurUartNo==0x01) UART1_Send(16,data);
											 if (CurUartNo==0x02) UART2_Send(16,data);
											 delay_ms(10);												 
										 }
																							 
									 } 
									 else  //���ն��б�
									 {
											data[0]=0xa3;
											data[1]=0xa3;
											data[2]=0xff;
											data[3]=0xff;
											data[4]=0xf0;
											data[5]=0x07;
											data[6]=0;
											data[7]=0;
											data[8]=0;
											data[9]=0;
											data[10]=0;
											data[11]=0;
											data[12]=0;
											data[13]=0;
											for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
											data[14]=checksum>>8;
											data[15]=checksum;
										  if (CurUartNo==0x01) UART1_Send(16,data);
										  if (CurUartNo==0x02) UART2_Send(16,data);
											delay_ms(10);												 
									 }
								}  
								else if(recBuf[5]==0x08)  //����������� cc2500[9]
								{
									switch (recBuf[6])
									{
										case 0x00://�ͼĴ���״̬40λ
														 for (i=0;i<0x40;i++){
															 data[i]=CC_RdStatus(i,recBuf[9]);                 
														 }
														 if (CurUartNo==0x01) UART1_Send(40,data);
														 if (CurUartNo==0x02) UART2_Send(40,data);
														 break;
				 
										case 0x01://д�Ĵ���
														 CC_WrReg(recBuf[7],recBuf[8],recBuf[9]);
														 break;
														 
										case 0x02://1��ʼ��
														 CC_Init(recBuf[9]);
														 break;
										case 0x03://1(2)�����ŵ�
											       if (recBuf[9]==RF_A) CC_Chan(Apoint,recBuf[9]); 
										         else CC_Chan(Bpoint,recBuf[9]); 
														 break;
										case 0x04://1(2)��������
														 CC_Cmd(0x34,recBuf[9]);
														 break;
										case 0x05://2��λ
														 CC_RESET(recBuf[9]);
														 break;
										case 0x06://2�ض�����
														 CC_RfConfig(&rfCC2500Settings76800,recBuf[9]);
														 break;
										case 0x07://2�źŷ���ǿ��
														 CC_PaTable(PAMAX,recBuf[9]);
														 break;
										case 0x08://2д�Ĵ���
														 CC_WrReg(CCxxx0_MCSM1,0x00,recBuf[9]); 
														 break;
										case 0x09://2����ִ�и�λ����
														 CC_FEC(1,recBuf[9]);
														 break;
										case 0x0A://2����״̬
														 CC2500_RxOn(recBuf[9]);
														 break;
										case 0x0B://�Ĵ���״̬
														 CC_Test(recBuf[9]);
														 break;                                                                                                                                                                                              
									}
								}
								else if (recBuf[5]==0x09)
								{ 
									   //�Ƿ��Զ������Ч�ն�
									   OSSchedLock();
									   AutoClear=recBuf[8];
									   OSSchedUnlock();
										 AT24CXX_WriteOneByte(AUTOCLEAR,AutoClear);//��������;
										 data[0]=0xa3;
										 data[1]=0xa3;
										 data[2]=0xff;
										 data[3]=0xff;
										 data[4]=0xf0;
										 data[5]=recBuf[5];
										 data[6]=Apoint;
										 data[7]=Bpoint;
										 data[8]=HeartEnable;
										 data[9]=Circle;
										 data[10]=MinSignal;
										 data[11]=AutoAsk;
										 data[12]=AutoClear;
										 data[13]=DebugMode;
										 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
										 data[14]=checksum>>8;
										 data[15]=checksum;
										 if (CurUartNo==0x01) UART1_Send(16,data);
										 if (CurUartNo==0x02) UART2_Send(16,data);	
										 delay_ms(10);
								}
								else if (recBuf[5]==0x0B)
								{ 
									   //����ƽ��ϵͳ���ļ��Ƴ�ʼ״̬;
									   OSSchedLock();
									   BlanceRunSec=0;
									   BlanceRunType=recBuf[6];									
									   BlancePlaceCount=recBuf[7];									
									   BlanceSecond=recBuf[8]<<8 | recBuf[9];
									   OSSchedUnlock();
									   AT24CXX_WriteOneByte(BLANCERUNTYPE,BlanceRunType);//��������;
									   AT24CXX_WriteOneByte(BLANCESECOND,BlanceSecond>>8);//��������;
									   AT24CXX_WriteOneByte(BLANCESECOND+1,BlanceSecond);//��������;
									   AT24CXX_WriteOneByte(BLANCEPLACECOUNT,BlancePlaceCount);//��������;
										 data[0]=0xa3;
										 data[1]=0xa3;
										 data[2]=0xff;
										 data[3]=0xff;
										 data[4]=0xf0;
										 data[5]=recBuf[5];
										 data[6]=BlanceRunType;
										 data[7]=BlancePlaceCount;
										 data[8]=BlanceSecond>>8;
										 data[9]=BlanceSecond;
										 data[10]=0x00;
										 data[11]=0x00;
										 data[12]=0x00;
										 data[13]=0x00;
										 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
										 data[14]=checksum>>8;
										 data[15]=checksum;
										 if (CurUartNo==0x01) UART1_Send(16,data);
										 if (CurUartNo==0x02) UART2_Send(16,data);	
										 delay_ms(10);
								}
								else if (recBuf[5]==0x0C)
								{ 
									   //��ȡƽ��ϵͳ���ļ��Ƴ�ʼ״̬;
										 data[0]=0xa3;
										 data[1]=0xa3;
										 data[2]=0xff;
										 data[3]=0xff;
										 data[4]=0xf0;
										 data[5]=recBuf[5];
										 data[6]=BlanceRunType;
									   data[7]=BlancePlaceCount;
										 data[8]=BlanceSecond>>8;
										 data[9]=BlanceSecond;
										 data[10]=BlanceRunSec>>8;
										 data[11]=BlanceRunSec;
										 data[12]=0x00;
										 data[13]=0x00;
										 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
										 data[14]=checksum>>8;
										 data[15]=checksum;
										 if (CurUartNo==0x01) UART1_Send(16,data);
										 if (CurUartNo==0x02) UART2_Send(16,data);	
										 delay_ms(10);
								}
								else if (recBuf[5]==0x0D)
								{ 
									   //��ȡ���û�վ����;
										 data[0]=0xa3;
										 data[1]=0xa3;
										 data[2]=0xff;
										 data[3]=0xff;
										 data[4]=0xf0;
										 data[5]=recBuf[5];
									   if (recBuf[6]==0x01){
											 PowerA=recBuf[7];
											 PowerB=recBuf[8];
											 AT24CXX_WriteOneByte(POWERA,PowerA);
											 AT24CXX_WriteOneByte(POWERB,PowerA);
											 CC_PaTable(PowerA,RF_A);
											 CC_PaTable(PowerB,RF_B);
											 CC2500_RxOn(RF_A);
											 CC2500_RxOn(RF_B);
										 }
										 data[7]=PowerA;
										 data[8]=PowerB;
										 data[9]=0x00;
										 data[10]=0x00;
										 data[11]=0x00;
										 data[12]=0x00;
										 data[13]=0x00;
										 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
										 data[14]=checksum>>8;
										 data[15]=checksum;
										 if (CurUartNo==0x01) UART1_Send(16,data);
										 if (CurUartNo==0x02) UART2_Send(16,data);	
										 delay_ms(10);
								}
             }
						 //��λ������վ������,�ɻ�վ���������
						 if ((recBuf[4]==0xAA)||(recBuf[4]==0xAB)){
							 //�����ն˺���մ�������;
							 tmpno=recBuf[7]<<8|recBuf[8];
							 if (tmpno>0) itemindex=SeekTerm(tmpno);
							 //����ն˺Ŵ���,����շ��ͻ���,AA��������ͻ��棬BB������ͻ���
							 OSSchedLock();
							 if (recBuf[4]==0xAB) {
								 if (tmpno>0) ClearTermCmd(itemindex);
							 }
							 else notRestSend=0x01;
							 //����������ظ����ն�,����λ�������;							 
							 if (recBuf[5]>0x00){
								 for (i=0;i<recBuf[5];i++){
									 RelayData[i]=recBuf[6+i];                 
								 }
								 RelayCount=0;//ת������0;
								 RelayLen=recBuf[5];
							 }
							 //���ճɹ��ظ�������������ľ��ɻ�վ����;
							 stationRtn=0x01;
							 //0xB5,0xFF,���ؼ�¼���ն˷��ؼ�¼��Ϣ
							 if ((RelayData[0]==0xB5)&&(RelayData[3]==0xFF)) stationRtn=0x00;
							 //0xC5,0xFAѯ�ն˽���״̬λ;
							 if ((RelayData[0]==0xC5)&&(RelayData[3]==0xFA)) stationRtn=0x00;
							 OSSchedUnlock();
							 if (stationRtn==0x01){
								 data[0]=0xa3;
								 data[1]=0xa3;
								 data[2]=0xff;
								 data[3]=0xff;
								 data[4]=recBuf[4];
								 data[5]=recBuf[5];
								 data[6]=recBuf[6];
								 data[7]=recBuf[7];
								 data[8]=0x00;
								 data[9]=0x00;
								 data[10]=0x00;
								 data[11]=0x00;
								 data[12]=0x00;
								 data[13]=0;
								 for(checksum=0,i=0;i<14;i++) checksum=checksum+data[i];
								 data[14]=checksum>>8;
								 data[15]=checksum;
								 delay_ms(5);
								 if (CurUartNo==0x01) UART1_Send(16,data);
								 if (CurUartNo==0x02) UART2_Send(16,data);
							 }
						 }
						 
				 } else
				 {
						 if((recBuf[4]==0xFA)||(recBuf[4]==0xFB)||(recBuf[4]==0xFC))  //ע����ߺ�������ģ��B
						 {
							 LEDB_Tx=0x00;
							 CC_SendPacket(recBuf,16,RF_B);
							 delay_ms(3);
							 LEDB_Tx=0x01;
						 }
						 else  //ֱ��ת��;
						 {
							 LEDA_Tx=0x00;
							 LEDA_Rx=0x00;
							 CC_SendPacket(recBuf,recLen,RF_A);
							 delay_ms(3);
							 LEDA_Tx=0x01;
							 LEDA_Rx=0x01;
						 }
				 
				 }
		  }   			
			 
  	  //��շ��������־;
			if (notRestSend==0x00){
				OSSchedLock();//����
				AtSendCmd=0x00;
				FreeCount=0x00;
				OSSchedUnlock();//����;			 
			}
    }
    return 0;
}

//�������״̬;
void ClearTermStauts(int paramCount)//�������״̬;
{
	uint8 i;
	OSSchedLock();
	for (i=0;i<paramCount;i++) {
		AskTermStatus[i][0]=0x00;
		AskTermStatus[i][READY]=0x00;
		AskTermStatus[i][HAVEEMP]=0x00;		
	}
	OSSchedUnlock();
}

//�����ն��б�
void SaveTerm(int index){
	u8 urlh,urll;
	if (index>=128) return;
	urlh=AskTermList[index]>>8;
	urll=AskTermList[index];
  AT24CXX_WriteOneByte(ASKTERMLIST+index*3,urlh);
	AT24CXX_WriteOneByte(ASKTERMLIST+index*3+1,urll);
	AT24CXX_WriteOneByte(ASKTERMLIST+index*3+2,0x01);
	//��һ��Ϊ��;
  AT24CXX_WriteOneByte(ASKTERMLIST+index*3+3,0x00);
	AT24CXX_WriteOneByte(ASKTERMLIST+index*3+4,0x00);
	AT24CXX_WriteOneByte(ASKTERMLIST+index*3+5,0x00);	
}

//��������ն��б�;
void DeleteTerm(void){
  AT24CXX_WriteOneByte(ASKTERMLIST,0);
	AT24CXX_WriteOneByte(ASKTERMLIST+1,0);
	AT24CXX_WriteOneByte(ASKTERMLIST+2,0x00);	
  AT24CXX_WriteOneByte(ASKTERMLIST+3,0);
	AT24CXX_WriteOneByte(ASKTERMLIST+4,0);
	AT24CXX_WriteOneByte(ASKTERMLIST+5,0x00);		
}
//�����������;
void ClearTermCmd(int index)
{
	uint8 i;
	if (index<0) return;
  OSSchedLock();
	AskTermCmdLen[index]=0;
  for (i=0;i<48;i++){
		AskTermCmd[index][i]=0x00;
	}
  OSSchedUnlock();

}

//��������ݴ�����;
void ClearAllTermCmd(void){
	uint8 i;
	for (i=0;i<AskTermCount;i++){
		ClearTermCmd(i);
	}
}

uint8 CheckDataOK(uint8* data,uint8 len)
{
	uint8 i,sumh,suml;
	uint16 checksum;
	if ((len>5)&&(len<MAXLEN)){
		for(checksum=0,i=0;i<len-2;i++) checksum=checksum+data[i];
		sumh=checksum>>8;
		suml=checksum;
		if ((data[len-2]==sumh)&&(data[len-1]==suml)) return 1;
		else return 0;
	} else return 0;
}

//����������;
void SendHeartBag(void)
{
	uint8  sendBuf[32];
  uint8  i;
  uint16 checksum;
	LEDA_Tx=0x00;
	LEDB_Rx=0x00;
	OSSchedLock();
	HeartBagCount=0x00;
	OSSchedUnlock();	
	sendBuf[0]=0xB5;
	sendBuf[1]=0x00;
	sendBuf[2]=0x00;
	sendBuf[3]=0xEE;//������
	sendBuf[4]=0x0A;//Ĭ�������Ϊ07;
	sendBuf[5]=Bpoint;//BƵ��;
	sendBuf[6]=Circle;//����;
	sendBuf[7]=MinSignal;//�źŷ�ֵ;
	for(checksum=0,i=0;i<8;i++)
		checksum=checksum+sendBuf[i];
	sendBuf[8]=checksum>>8;
	sendBuf[9]=checksum;
	//ͨ��AƵ����ѭ
	delay_ms(10);
	CC_SendPacket(sendBuf,0x0A,RF_A);
	delay_ms(2);
	LEDA_Tx=0x01;	
	LEDB_Rx=0x01;
}

