#include "led.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "pcf8563.h"
#include "24cxx.h"
#include "cc2500.h"  
#include "includes.h"  

//无线模块定义;
#define RF_A							1
#define RF_B							0
//最多等待10*1000us
#define MaxWaitCount      30
#define MaxCommError      32
#define BPointMaxWait			60
#define MaxTermCount			128
//超过15秒就踢除
#define MaxWaitSecond			15
//定义员工状态位;
#define READY							1
#define ONLINE 						2
#define HAVEEMP						3

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			10 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				  64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	

//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       			8 
//设置任务堆栈大小
#define MAIN_STK_SIZE  					1024
//任务堆栈	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);


//free
//设置任务优先级
#define FREE_TASK_PRIO       			7 
//设置任务堆栈大小
#define FREE_STK_SIZE  					64
//任务堆栈	
OS_STK FREE_TASK_STK[FREE_STK_SIZE];
//任务函数
void free_task(void *pdata);

//-----------------------------------------程序自定义开始---------------------------------------;


uint8 CC2500Data[2][MAXLEN];//无线模块接收的数据;
uint8 CC2500Count[2];//数据长度;

//全局变量;
uint8 CurUartNo=0x01;//当前通讯串口，默认为网口
uint8_t chark;//开机自检;
void ResetCC2500Data(uint8 channel);//清除CC2500数据;
uint8 ExplainCC2500(uint8 channel);//接收检查数据;					 
void TestSetTime(void);//测试设置时钟;
void RefreshTime(void);//刷新时间;
void WriteSetting(void);//读取配置;
void ReadSetting(void);//保存配置;
void AskTerm(void);//循终端;
uint8 CheckPc(void);//检查并处理协议;
int SeekTerm(uint16 paramno);//获取终端号序号;
void ClearTermCmd(int index);//清除缓存命令;
void ClearAllTermCmd(void);//清除所有缓存命令;
void SendHeartBag(void);//发送心跳包;
void ClearInvalidTerm(u8 AskTermIndex);//清理无用终端;
void SaveTerm(int index);//保存指定的终端索引号到存储;
void DeleteTerm(void);//清除所有终端列表;
void CheckBlanceState(void);//检查节拍系统;
void SendBlanceLight(int paramTermNo);//广播平衡系统灯状态;
void ClearTermStauts(int paramCount);//清除所有状态;
void BlanceOver(void);//节拍完成;
void sme1_read(void);//读取无线模块数据1;
void sme2_read(void);//读取无线模块数据2;
void SendOnLineToPC(void);//发送终端数到上位机

//-------------全局变量----------------//
uint16 FreeCount=0,SendTimeOut=0;//无
uint8 AskWaitOver=0,AtSendCmd=0;//全局的等待完成,中断接收处理完成，这边置结束等待;					 
uint8 Apoint,Bpoint,HeartEnable,AutoAsk,Circle,MinSignal,AskDelay,DebugMode;//AutoAsk为自动轮循标志
uint8 AutoClear=1;//自动清理无效终端
uint8 AskTermIndex,AskTermCount,AskStart,AskPcSign,HeartBagCount;//,AskTermIndex为当前位置,AskTermCount为总终端数,AskStart为暂停轮循标志,AskPcSign超时询问PC是否处理结束
uint16 AskTermList[MaxTermCount];//轮循列表，最多128个终端,AskWaitCount等待计数器
uint8 AskTermErrCount[MaxTermCount][2];//轮循出错,要踢除列表,次数而秒数;
uint8 AskTermCmd[MaxTermCount][MAXLEN];//待上传最多48个字节;
uint8 AskTermCmdLen[MaxTermCount];//传上传命令长度;
uint8 AskTermStatus[MaxTermCount][4];//0:表示是否需要取状态;1:平衡系统状态;2:在不在线;3:是否有员工;
uint8 RelayData[MAXLEN],RelayLen,RelayCount=0;//转发次数;
uint8 CheckDataOK(uint8* data,uint8 len);////校验数据是否准确;
//全局变量;
uint8 CurYear,CurMonth,CurDay,CurWeek,CurHour,CurMinute,CurSecond;//当前时分秒;
uint16 BlanceSecond=0x00,BlanceRunSec=0x00;//平衡系统节拍时间;
uint8 BlanceRunType=0x00,NeedSendBlance=0x00,BlancePlaceCount=0x00;//默认为手动,不运行;
uint8 PowerA=0xFF,PowerB=0xFF;
uint16 BPointWait=0x00;
uint8	SendOnLine=0x00;

//-----------------------------------------程序自定义结束---------------------------------------;
//////////////////////////////////////////////////////////////////////////////
    
//OS_EVENT * msg_key;			  //按键邮箱事件块	  
//OS_EVENT * q_msg;			    //消息队列
OS_TMR   * tmr1;			    //软件定时器1
OS_TMR   * tmr2;			    //软件定时器1
OS_EVENT * RF1_user;
OS_EVENT * RF2_user;
//软件定时器1的回调函数	
//每1s执行一次,用于显示CPU使用率和内存使用率		   
void tmr1_callback(OS_TMR *ptmr,void *p_arg) 
{	
	  RefreshTime();//一秒一次;
	  //接收保护;
	  if (BPointWait<=BPointMaxWait) {
			BPointWait++;
		}
		//发送在线终端;
		if (SendOnLine<0xFF)	SendOnLine++;
	  //仅自动节拍时才处理
	  if ((BlanceRunType==2)&&(BlanceSecond>0)){
			BlanceRunSec++;
			if (BlanceRunSec>=BlanceSecond){
				NeedSendBlance=0x01;
				BlanceRunSec=0;
			}
		}
 	  //发送命令超时(10秒没回就认为超时了需重发)
	  if (AtSendCmd==0x01) SendTimeOut++;
	  if (SendTimeOut>10) {
			SendTimeOut=0x00;
			AtSendCmd=0x00;
	  };
	  //轮循状态无终端时
		if (AutoAsk==0x01) {
			FreeCount++;
			//需要往基站发命令,表示空闲;
	    if (FreeCount>15) {
				AskPcSign=1;
				FreeCount=0x00;
			}
		};
		//心跳包发送5秒1次;
		if (HeartBagCount<0xFF) HeartBagCount++;	
}
//软件定时器1的回调函数	
//每100ms执行一次,用于显示CPU使用率和内存使用率		   
void tmr2_callback(OS_TMR *ptmr,void *p_arg) 
{   
    delay_ms(100);	   
} 
			


int main(void)
{	 
  u8 comport;
 	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	LED_Init();
	IIC_Init();
	uart1dma_init(115200);
	uart2dma_init(115200);
	uart3dma_init(115200);
	TIM2_Init(100,7199);
	TIM3_Init(500,7199);
	TIM4_Init(1000,7199);
  HWSPI_Init();
	Buzzer(30);//响个30ms;
	//灯全亮
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
  ReadSetting();//读取配置
	//设置发射功率;
	CC_PaTable(PowerA,RF_A);
	delay_ms(200);
  CC_PaTable(PowerB,RF_B);
	delay_ms(200);
	//灯全灭
	LEDA_Tx=!LEDA_Tx;
	LEDA_Rx=!LEDA_Rx;
	LEDB_Tx=!LEDB_Tx;
	LEDB_Rx=!LEDB_Rx;
	//双串口，读取用于通讯的默认串口
	comport=AT24CXX_ReadOneByte(COMPORT);
	if ((comport>=1)||(comport<=3)) {
		CurUartNo=comport;
	}
	OSInit();  	 			//初始化UCOSII		  
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	


}							    
//开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;  	    
 	RF1_user=OSSemCreate(0); 	//创建SPI信号量
  RF2_user=OSSemCreate(0); 	//创建RFID信号量	  	  
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)     				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);
	OSTaskCreate(free_task,(void *)0,(OS_STK*)&FREE_TASK_STK[FREE_STK_SIZE-1],FREE_TASK_PRIO);				    				   
 	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}
//LED任务


//主任务
void main_task(void *pdata)
{							 
	u8 i,err,tbyte[MAXLEN],waitSecond;
	u16 AskWaitCount=0;
	OS_CPU_SR cpu_sr=0; 
  //创建定时器;
 	tmr1=OSTmrCreate(10,100,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,(INT8U *)"tmr1",&err);   //1s执行一次	
	OSTmrStart(tmr1,&err);//启动软件定时器1		
	tmr2=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr2_callback,0,(INT8U *)"tmr2",&err);	//100ms执行一次
  //OSTmrStart(tmr2,&err);//启动软件定时器1			 
	IWDG_Init(4,625);     //看门狗初始化1s;
	CC_Chan(Apoint,RF_A);
	CC_Chan(Bpoint,RF_B);	
	AskWaitCount=MaxWaitCount;
	AskWaitOver=0;
	AtSendCmd=0x00;
	FreeCount=0x00;
	//数据
	ResetCC2500Data(RF_A);
	ResetCC2500Data(RF_B);
	//调试信号输出
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
	//置接收模式;
	CC2500_RxOn(RF_A);
	CC2500_RxOn(RF_B);
	//如果有终端需要轮循，直接开启
	if (AskTermCount>0) AskStart=0x01;
 	while(1)
	{	
		IWDG_Feed();//喂狗防死机
		//超时或接收到，就继续发送;
		if ((AskWaitCount>=MaxWaitCount)|| AskWaitOver){
			//检查是否需要清理无效终端;
			if ((AskStart==1)&&(AskPcSign!=0x01)&&(AskTermCount>0)&&(AskTermCount<=MaxTermCount)){
				OSSchedLock();
				//有应答复位
				if (AskWaitOver==0x01) {
					AskTermErrCount[AskTermIndex][0]=0x00;
					AskTermErrCount[AskTermIndex][1]=0x00;
					AskTermStatus[AskTermIndex][ONLINE]=0x00;//在线;					
				}
				else {					
					//无应答累加,超过一定次数无效;	
          if (AskTermErrCount[AskTermIndex][0]==0x00) {
						AskTermErrCount[AskTermIndex][1]=CurSecond;
					}
					//不超过就增加
					if (AskTermErrCount[AskTermIndex][0]<=MaxCommError) AskTermErrCount[AskTermIndex][0]++;
					//超过就判断是否超时
					if (AskTermErrCount[AskTermIndex][0]>MaxCommError){
						if (CurSecond<AskTermErrCount[AskTermIndex][1]) waitSecond=CurSecond+60-AskTermErrCount[AskTermIndex][1];
						else waitSecond=CurSecond-AskTermErrCount[AskTermIndex][1];
						//清理无用终端;
						if (waitSecond>=MaxWaitSecond) {
							if (AutoClear==1) ClearInvalidTerm(AskTermIndex);
							else AskTermStatus[AskTermIndex][ONLINE]=0xEE;//不在线;
						}
					}
				};
	      OSSchedUnlock();				
			}
			//5秒发送心跳包,更新信号强度;
			if (HeartEnable==0x01){
				if ((HeartBagCount>5)&&(RelayLen==0)) {
					SendHeartBag();		
				}
			}
			//任务正式开始;
			delay_us(500);	//延时1000us,等下位机恢得接收状态;
			//是否要复位B模块;
			if (BPointWait>=BPointMaxWait){
				BPointWait=0x00;			
				CC2500_RxOn(RF_B);
			}
			OSSchedLock();
			AskWaitOver=0;
      OSSchedUnlock();
			//如果没有终端，也是需要延时等;
			if (AskTermCount==0) AskWaitCount=0;
			//无数据则进行轮循;
			if ((CC2500Count[RF_A]==0)&&(CC2500Count[RF_B]==0)&&(RelayLen==0)) {
				//循下一终端
				OSSchedLock();
				AskTermIndex++;
				//判断是否为列表中的最后一终端,如果是则回到第一个
				if (AskTermIndex>=AskTermCount) AskTermIndex=0;	
				OSSchedUnlock();			
				//终端延时;
				if (AskWaitCount<5) {
					err=10-AskWaitCount;
					delay_ms(err);
				};
				//复位等待时间次数;
				AskWaitCount=0;				
				//询终端数据;
				AskTerm();
				//延时;
		    delay_us(500);			
			}
		}			
		//检查是否有数据需要解析;
		if (SerOK[RF_A]) sme1_read();
		if (SerOK[RF_B]) sme2_read();
		//检查是需要发送;
		if (AtSendCmd==0) {
			ExplainCC2500(RF_A);
		  ExplainCC2500(RF_B);
			//判断是否要广播平衡系统的灯状态
			if (BlanceRunType==0x01) CheckBlanceState();
			if (NeedSendBlance==0x01){
				//发送灯状态;
				OSSchedLock();
				NeedSendBlance=0x00;
				OSSchedUnlock();
				//发送灯状态两遍
				SendBlanceLight(0);
				delay_ms(10);
				SendBlanceLight(0);
				delay_ms(10);
				SendBlanceLight(0);
				delay_ms(10);
				//清除标志;
				ClearTermStauts(AskTermCount);
				//发送命令到上位机
				BlanceOver();
			}
		}
		//检查上位机;
		if (CheckPc()==100){
       __set_FAULTMASK(1);//关闭总中断
       NVIC_SystemReset();//请求单片机重启
		}
		//有转发命令则转发
		if (RelayLen>0){
			LEDA_Tx=0x00;
			LEDB_Tx=0x00;
			OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)   
			if ((RelayCount<5)&&(RelayLen<MAXLEN)){
				for(i=0;i<RelayLen;i++) tbyte[i]=RelayData[i];
				RelayCount++;//最多转发5次;
				CC_SendPacket(tbyte,RelayLen,RF_A);	
			} else{
				//复位;
				RelayLen=0;
				RelayCount=0;
			}
			OS_EXIT_CRITICAL();			//退出临界区(无法被中断打断)   
			delay_ms(5);			
			LEDA_Tx=0x01;
			LEDB_Tx=0x01;
		}
		//如果不在发送命令,则上传一次在线用户;
		if ((SendOnLine>0xB4)&&(AtSendCmd==0)){
			SendOnLine=0;
			SendOnLineToPC();
		}
		//延时1000us;
		delay_us(AskDelay*200);
		//等待加一次;
		OSSchedLock();
		AskWaitCount++;
		OSSchedUnlock();
	}
	
}		
//发送终端数给上位机
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
//检查平衡系统;
void CheckBlanceState(){
	uint16 i,readyCount;
	for (readyCount=0,i=0;i<AskTermCount;i++){
    //只处理在线设备
		if (AskTermStatus[AskTermIndex][ONLINE]!=0xEE){
			if ((AskTermStatus[i][READY]==0x01)&&(AskTermStatus[i][HAVEEMP]==0x01)) readyCount++; //完成设备
		}
	}	
	//全部在线且完成;
	if (AskTermCount>0){
		if ((readyCount>=BlancePlaceCount)||(readyCount>=AskTermCount)){
			OSSchedLock();
			NeedSendBlance=0x01;
			OSSchedUnlock();
		}
	}
}

//广播平衡系统的灯状态;
void SendBlanceLight(int paramTermNo)
{
	OS_CPU_SR cpu_sr=0;
  uint8  sendBuf[20];
  uint8  i;
  uint16 checksum;
	LEDA_Tx=0x00;
	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)
	sendBuf[0]=0xB5;
	sendBuf[1]=paramTermNo>>8;
	sendBuf[2]=paramTermNo;
	sendBuf[3]=0x0D;//灯命令;
	sendBuf[4]=0x08;//命令长度
	sendBuf[5]=0;//复位状态;
	for(checksum=0,i=0;i<6;i++)
		checksum=checksum+sendBuf[i];
	sendBuf[6]=checksum>>8;
	sendBuf[7]=checksum;
	//通过A频点轮循
	CC_SendPacket(sendBuf,0x08,RF_A);
	OS_EXIT_CRITICAL();			//进入临界区(无法被中断打断)
	delay_ms(5);
	LEDA_Tx=0x01;	
}

//平衡节拍系统开始;
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

//读取数据A
void sme1_read(void){
	u8 i,needupload=0;
	needupload=0;//默认不需要上传至服务器;
	LEDA_Rx=0x00;
	CC_RdPacket(RF_A);
	//有数据处理;
	if (SerLen[RF_A]>5) 
	{		
		//校验数据;
		if (CheckDataOK(SerData[RF_A],SerLen[RF_A])==0x01)
		{
			//判断是否需要清除转发状态,非轮循应答的，一般为转发应答;
			if ((RelayLen>0)&&(SerData[RF_A][0]==0xB3)&&(SerData[RF_A][3]!=0xFF)&&(SerData[RF_A][3]!=0xFE)) RelayLen=0;
			//如果在轮循过程中,则暂存数据致对应
			if (AskTermCount>0)
			{
				if ((SerData[RF_A][0]==0xB3)||(SerData[RF_A][0]==0xC3)||(SerData[RF_A][0]==0xD3))
				{
					//地址相同,表示终端回传的;
					if (AskTermList[AskTermIndex]==(SerData[RF_A][1]<<8|SerData[RF_A][2]))
					{
						//终端返回PC相关的命令
						if (SerData[RF_A][0]==0xB3){
							//记录需上传到PC，长度为0x21表示有记录，0x07表示没记录;
							if ((SerData[RF_A][3]==0xFF)&&(SerData[RF_A][4]==0x21)){														
								//如果在上传过程中，则暂存否则直接传出;
								if (AtSendCmd==1){
									//数据暂存到对应的数组中
									AskTermCmdLen[AskTermIndex]=SerLen[RF_A];
									//存数据到暂存;
									for(i=0;i<SerLen[RF_A];i++)
									{
										AskTermCmd[AskTermIndex][i]=SerData[RF_A][i];
									}
								} else needupload=1;
							} else{
								//无记录时看节拍;
								if (SerData[RF_A][3]==0xFE)	{
										AskTermStatus[AskTermIndex][HAVEEMP]=SerData[RF_A][5];//员工在线情况;
										AskTermStatus[AskTermIndex][READY]=SerData[RF_A][6];//节拍周期内完成操作或不需要判断
								}
							}
						}
						//平衡SerData直接更新状态;
						if (SerData[RF_A][0]==0xC3){
							needupload=1;										
						}
						//无数据回复无需等待;
						if (SerData[RF_A][0]==0xD3){
							ClearTermCmd(AskTermIndex);
						}						
					}
				}
			} else{
				//记录需上传到PC,别的不需要保存;
				if ((SerData[RF_A][0]==0xB3)&&((SerData[RF_A][3]==0xFF)||(SerData[RF_A][3]==0xFE))) {
						needupload=1;//在非采集模式，直接上传到PC					
				};
				//节拍读取上传到PC,别的不需要;
				if ((SerData[RF_A][0]==0xC3)&&(SerData[RF_A][3]==0xFA)) {
						needupload=1;//在非采集模式，直接上传到PC					
				};					
			}
			//需放到实时上传命令
			if (needupload){ 
				//存数据到暂存;
				for(i=0;i<SerLen[RF_A];i++)
				{
					CC2500Data[RF_A][i]=SerData[RF_A][i];
				}	
				CC2500Count[RF_A]=SerLen[RF_A];
			}
			//有应答可进行下一个轮循;
			AskWaitOver=1;
		}
	}
	//清除
	SerOK[RF_A]=0x00;
	Clean_Rx_buffer(SerLen[RF_A],RF_A);
  CC2500_RxOn(RF_A);	
	LEDA_Rx=!LEDA_Rx;	
}

//读取数据A
void sme2_read(void){
u8 i,cmdlen,needupload=0,returnOK=0,sendDelay=20;
	uint16 checksum;
	uint8 heartdata[32];
	LEDB_Rx=0x00;
	BPointWait=0x00;
	CC_RdPacket(RF_B);
	//有数据处理;
	if (SerLen[RF_B]>0) 
	{
			if (CheckDataOK(SerData[RF_B],SerLen[RF_B])==0x01)
			{
				//检查是否来询基站的 0xB0 UnitAddrH,UnitAddrL,0xFF,CmdLen,CheckSumH,CheckSumL;
				if (SerData[RF_B][0]==0xB0)
				{
					cmdlen=14;//命令长度;
					heartdata[0]=0xA0;
					heartdata[1]=SerData[RF_B][1];
					heartdata[2]=SerData[RF_B][2];
					heartdata[3]=0xFF;//命令;
					heartdata[4]=cmdlen;//长度;
					heartdata[5]=Apoint; //命令
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
				//请求注册的 0xB1 UnitAddrH,UnitAddrL,0xFF,CmdLen,CheckSumH,CheckSumL;
				if (SerData[RF_B][0]==0xB1)
				{
					if (AskTermCount<128){
						returnOK=1;
					} else returnOK=0;
					cmdlen=10;//命令长度;
					heartdata[0]=0xA1;
					heartdata[1]=SerData[RF_B][1];
					heartdata[2]=SerData[RF_B][2];
					heartdata[3]=0xFF;//命令;
					heartdata[4]=cmdlen;//长度;
					heartdata[5]=Apoint; //命令
					heartdata[6]=Bpoint;
					heartdata[7]=returnOK;//注册是否成功; 
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
					//未在当前基站注册的则添加上
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
						if (AskTermCount<128) AskTermList[AskTermCount]=0;//置列表的最后一条终端号为0
						if (AskTermCount>=MaxTermCount) AskTermCount=0;
						OSSchedUnlock();
					};
					Buzzer(10);
					//有注册过来就启动轮循;
					AskStart=0x01;
				}
				//呼叫类的信息;
				if (SerData[RF_B][0]==0xB2){
					LEDB_Tx=0x00;
					//B待上传数据为空则表示呼叫OK，可以转发到PC
					if (CC2500Count[RF_B]==0) {
						returnOK=1;
						needupload=0x01;//呼叫是需要上传;
					} else returnOK=0;
					cmdlen=10;//命令长度;
					heartdata[0]=0xA2;
					heartdata[1]=SerData[RF_B][1];
					heartdata[2]=SerData[RF_B][2];
					heartdata[3]=0xFF;//命令;
					heartdata[4]=cmdlen;//长度;
					heartdata[5]=Apoint; //命令
					heartdata[6]=Bpoint;
					heartdata[7]=returnOK;//注册是否成功; 
					for(checksum=0,i=0;i<cmdlen-2;i++)
						 checksum=checksum+heartdata[i];
					heartdata[cmdlen-2]=checksum>>8;
					heartdata[cmdlen-1]=checksum;
					delay_ms(sendDelay);
					CC_SendPacket(heartdata,cmdlen,RF_B); 
					LEDB_Tx=0x01;

				}
				//准备可能要实时上传到PC
				if (needupload==0x01)
				{
					needupload=0;
					//存数据到暂存;
					for(i=0;i<=SerLen[RF_B];i++)
						CC2500Data[RF_B][i]=SerData[RF_B][i];
					CC2500Count[RF_B]=SerLen[RF_B];
				}
			}
	}
	//清除
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
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
  if(EXTI_GetITStatus(EXTI_Line0)!= RESET)
	{
    SerOK[RF_A]=0x01;
		EXTI_ClearFlag(EXTI_Line0);   
		EXTI_ClearITPendingBit(EXTI_Line0);
	}
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();  											 
#endif  
}

void EXTI4_IRQHandler(void)
{
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
  if(EXTI_GetITStatus(EXTI_Line4)!= RESET)
	{
    SerOK[RF_B]=0x01;
		EXTI_ClearFlag(EXTI_Line4);   
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();  											 
#endif  
}






//刷新时间;
void RefreshTime(void) 
{
	uint8 TimeBuf[7];//当前时钟的全局变量;,2时1分0秒 6年5月3日 4星期;
	//取时钟
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
		//加载终端列表
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
		//自动清除不在线终端
		AutoClear=AT24CXX_ReadOneByte(AUTOCLEAR);
	  if (AutoClear>0x01) {
			AutoClear=0x01;//默认清除;
			AT24CXX_WriteOneByte(AUTOCLEAR,AutoClear);
		}
		//读取平衡系统状态
		BlanceRunType=AT24CXX_ReadOneByte(BLANCERUNTYPE);	
	  if (BlanceRunType>0x02) {
			BlanceRunType=0x00;//默认手动;
			AT24CXX_WriteOneByte(BLANCERUNTYPE,BlanceRunType);
		}		
		//读节拍时间
		urlh=AT24CXX_ReadOneByte(BLANCESECOND);
		urll=AT24CXX_ReadOneByte(BLANCESECOND+1);
		if ((urlh==0xFF)&&(urll==0xFF)) BlanceSecond=0;
    else BlanceSecond=urlh<<8 | urll;
		BlanceRunSec=0;
		//平衡需要在线的位置
		BlancePlaceCount=AT24CXX_ReadOneByte(BLANCEPLACECOUNT);
		//发射功率;
		PowerA=AT24CXX_ReadOneByte(POWERA);
		if (PowerA<0x1F){
			PowerA=0xFF;//最高强度
			AT24CXX_WriteOneByte(POWERA,PowerA);			
		}
		PowerB=AT24CXX_ReadOneByte(POWERB);
		if (PowerB<0x1F){
			PowerB=0xFF;//最高强度
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
 
//清除CC2500数据;
void ResetCC2500Data(uint8 channel)
{
	uint8 i;
	CC2500Count[channel]=0x00;
	//存数据到暂存;
	for(i=0;i<MAXLEN;i++)
	{
		CC2500Data[channel][i]=0x00;
	}
}

//接收检查数据;
uint8 ExplainCC2500(uint8 channel)
{
   if (CC2500Count[channel]>0)
	 {
		 //延时;
		 delay_ms(5);
     //置发送命令标志;
     AtSendCmd=0x01;
		 //根据数据来处理,有些数据不需要输出到PC
		 //发送至PC,调试模式都需要输出到PC;
		 if (CurUartNo==0x01) UART1_Send(CC2500Count[channel],CC2500Data[channel]);
		 if (CurUartNo==0x02) UART2_Send(CC2500Count[channel],CC2500Data[channel]);
		 //清空数据
     ResetCC2500Data(channel);
		 delay_ms(5);
	 }
	 return 1;	 
} 
 
//测试设置时间;
void TestSetTime(void){
	uint8 paramtime[7];
	paramtime[0]=CurSecond;//秒
	paramtime[1]=CurMinute;//分
	paramtime[2]=CurHour;//时
	paramtime[3]=CurDay;//日
	paramtime[4]=0;//星期日0-6;
	paramtime[5]=CurMonth;//月
	paramtime[6]=CurYear;//年
	//设置时间;
	PCF8563_Set_Time(paramtime);
}

//获取终端序号;
int SeekTerm(uint16 paramno){
	uint8 i;
	for (i=0;i<AskTermCount;i++){
		if (AskTermList[i]==paramno) return i;
	}
	return -1;
}


//根据轮循列表询终端是否需要上传;
void AskTerm(void)
{
  uint8  sendBuf[32];
  uint8  data[16];
  uint8  i;
  uint16 checksum,TmpTermNo;
  //如果标志为1,表示需要询终端
  if (AutoAsk==1)
  {
    if ((AskStart==1)&&(AskTermCount>0)&&(AskPcSign!=1)) //轮循中
    {
				//发送询终端命令; A5 A5 00 01 FF 05 00 00 00 00 00 00 00 00 02 4F 
				TmpTermNo=AskTermList[AskTermIndex];
				if (TmpTermNo>0)
				{ 
					//复位空闲;
					FreeCount=0;
					//循终端状态,用于平衡系统（每个周期询到1为止,触发后置0）
					 //数据上传实现过程0xB5;
					if (AskTermCmdLen[AskTermIndex]==0x00)//无待上传数据;
				  {
						LEDA_Tx=0x00;			
						sendBuf[0]=0xB5;
						sendBuf[1]=TmpTermNo>>8;
						sendBuf[2]=TmpTermNo;
						sendBuf[3]=0xFF;//循上传命令
						sendBuf[4]=0x07;//默认命令长度为07;
						for(checksum=0,i=0;i<5;i++)
							checksum=checksum+sendBuf[i];
						sendBuf[5]=checksum>>8;
						sendBuf[6]=checksum;
						//通过A频点轮循
						CC_SendPacket(sendBuf,0x07,RF_A);
						LEDA_Tx=0x01;	
				  } else //检查现在是不是要将命令发出;
					{
						 if (AtSendCmd==0x00){
							 //置发送命令标志;
							 AtSendCmd=0x01;				 
							 //通过串口发数据,有返回则置0;
							 if (CurUartNo==0x01) UART1_Send(AskTermCmdLen[AskTermIndex],AskTermCmd[AskTermIndex]);
							 if (CurUartNo==0x02) UART2_Send(AskTermCmdLen[AskTermIndex],AskTermCmd[AskTermIndex]);
							 delay_ms(10);
						 }
						 //直接查询下一个，无需等待;
						 AskWaitOver=0x01;
					}
			  }
    } else if (AskPcSign==1)
    {
      //串口复位
      USART2_RX_STA=0;
      //状态复位
			OSSchedLock();
      AskPcSign=0;
			FreeCount=0;
      OSSchedUnlock();
      //发命令问PC是否已处理结束，通知PC目前采集处理闲置状态
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


//清理无用终端;
void ClearInvalidTerm(u8 AskTermIndex){
	u8 i,j;
	//发送命令至上位机abort
	
	//其他数据清理;
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
	//终端数-1
	AskTermCount--;
  AskTermList[AskTermCount]=0x00;
	AskTermErrCount[AskTermCount][0]=0x00;
	AskTermErrCount[AskTermCount][1]=0x00;
	AskTermCmdLen[AskTermCount]=0x00;
	for (j=0;j<48;j++) AskTermCmd[AskTermCount][j]=0x00;
	//当前终端-1
	if (AskTermIndex>0) AskTermIndex--;
	
}

//检查上位机
uint8 CheckPc(void)
{
   uint8  recBuf[128],recLen=0,notRestSend=0x00,data[MAXLEN],stationRtn;
	 uint16 tmpno;
   uint8  uartno,i,j,AllowAdd;//AllowAdd为允计增加终端号标志,用于判断终端号是否需要加到轮循列表中;
	 int itemindex;
   uint16 checksum;
	 //串口接收数据判断
	 if (USART1_RX_STA>0x00) {		 
		   uartno=1;
		   //接收数据;
			 recLen=USART1_RX_STA;
       for(i=0;i<USART1_RX_STA;i++)
       {
				 if (i<128) recBuf[i]=USART1_RX_BUF[i];
       }
			 USART1_RX_STA=0x00;//串口复位;
	 }	 	
	 if (USART2_RX_STA>0x00) {		 
		   uartno=2;
		   //接收数据;
			 recLen=USART2_RX_STA;
       for(i=0;i<USART2_RX_STA;i++)
       {
				 if (i<128) recBuf[i]=USART2_RX_BUF[i];
       }
			 USART2_RX_STA=0x00;//串口复位;
	 }
   if ((recLen>0x00)&&(recLen<MAXLEN))
   {
			 //只有标准基站命令才更改端口;
			 if ((recBuf[0]==0xA5)&&(recBuf[1]==0xA5)) {
				 //当前端口与实际数据交换端口不同则保存端口供下次重启使用
				 if (CurUartNo!=uartno){
					 CurUartNo=uartno;
					 AT24CXX_WriteOneByte(COMPORT,CurUartNo);
				 }
			 }
		   if (recLen>=16){
				 if((recBuf[2]==0xff)&&(recBuf[3]==0xff))  //地址为0xFFFF采集器，否则为终端
				 {
					   //基站设置; 
						 if(recBuf[4]==0xf0)
						 {
							  if (recBuf[5]==0x0A) //啥事不做，仅仅是回复为了消除AtSendCmd状态;
								{									
									//也不用回复,一般用于回复呼叫指令接收或上传终端列表接收;
									notRestSend=0x00;//复位发送状态命令;		
                  AskStart=0x01;//启动轮循;		
									delay_ms(10);									
								}
								else if(recBuf[5]==0x01)  //取
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
								else if(recBuf[5]==0x02)  //置
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
								else if(recBuf[5]==0x03)  //复位
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
								else if(recBuf[5]==0x04)  //增加轮循终端
								{    
									 OSSchedLock();
									 AskStart=0;//暂停轮循
									 AllowAdd=0;//置初始值,允计加入;
									 //判断终端号是否存在,不存在就加
									 tmpno=(recBuf[6]<<8)+recBuf[7];
									 if (SeekTerm(tmpno)<0) AllowAdd=1;
									 if ((AllowAdd==1)&&(AskTermCount<128))
									 {
										 AskTermList[AskTermCount]=tmpno;//将终端号存进列表中
										 AskTermStatus[AskTermCount][0]=0x00;
										 AskTermStatus[AskTermCount][READY]=0x00;
										 AskTermStatus[AskTermCount][ONLINE]=0x00;
										 AskTermStatus[AskTermCount][HAVEEMP]=0x00;
										 AskTermErrCount[AskTermCount][0]=0x00;
										 AskTermErrCount[AskTermCount][1]=0x00;
										 SaveTerm(AskTermCount);//保存列表至存储
										 ClearTermCmd(AskTermCount);
										 AskTermCount++;//终端数增加1
										 if (AskTermCount<128) AskTermList[AskTermCount]=0;//置列表的最后一条终端号为0
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
								else if(recBuf[5]==0x05)  //清空轮循终端
								{          
									 delay_ms(5);
									 AskStart=0;//暂停轮循
									 AskTermCount=0; //终端数为0
									 AskTermIndex=0; //当前终端列表索引值为0
									 AskTermList[AskTermCount]=0;//置列表的最后一条终端号为0
									 DeleteTerm();//清空列表
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
								else if(recBuf[5]==0x06)  //启动轮循或暂停轮循
								{
									 ClearAllTermCmd();
									 ClearTermStauts(AskTermCount);
									 BlanceRunSec=0;
									 delay_ms(5);
									 AskStart=recBuf[6];//0为暂停轮循,1为开始轮循
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
								else if(recBuf[5]==0x07)  //查看终端列表
								{
									 AskStart=0;//0为暂停轮循
									 if (AskTermCount>0) //有终端列表
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
									 else  //无终端列表
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
								else if(recBuf[5]==0x08)  //查问题调试用 cc2500[9]
								{
									switch (recBuf[6])
									{
										case 0x00://送寄存器状态40位
														 for (i=0;i<0x40;i++){
															 data[i]=CC_RdStatus(i,recBuf[9]);                 
														 }
														 if (CurUartNo==0x01) UART1_Send(40,data);
														 if (CurUartNo==0x02) UART2_Send(40,data);
														 break;
				 
										case 0x01://写寄存器
														 CC_WrReg(recBuf[7],recBuf[8],recBuf[9]);
														 break;
														 
										case 0x02://1初始化
														 CC_Init(recBuf[9]);
														 break;
										case 0x03://1(2)设置信道
											       if (recBuf[9]==RF_A) CC_Chan(Apoint,recBuf[9]); 
										         else CC_Chan(Bpoint,recBuf[9]); 
														 break;
										case 0x04://1(2)发送命令
														 CC_Cmd(0x34,recBuf[9]);
														 break;
										case 0x05://2复位
														 CC_RESET(recBuf[9]);
														 break;
										case 0x06://2重读配置
														 CC_RfConfig(&rfCC2500Settings76800,recBuf[9]);
														 break;
										case 0x07://2信号发送强度
														 CC_PaTable(PAMAX,recBuf[9]);
														 break;
										case 0x08://2写寄存器
														 CC_WrReg(CCxxx0_MCSM1,0x00,recBuf[9]); 
														 break;
										case 0x09://2依次执行复位操作
														 CC_FEC(1,recBuf[9]);
														 break;
										case 0x0A://2接收状态
														 CC2500_RxOn(recBuf[9]);
														 break;
										case 0x0B://寄存器状态
														 CC_Test(recBuf[9]);
														 break;                                                                                                                                                                                              
									}
								}
								else if (recBuf[5]==0x09)
								{ 
									   //是否自动清除无效终端
									   OSSchedLock();
									   AutoClear=recBuf[8];
									   OSSchedUnlock();
										 AT24CXX_WriteOneByte(AUTOCLEAR,AutoClear);//保存配置;
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
									   //设置平衡系统节拍及灯初始状态;
									   OSSchedLock();
									   BlanceRunSec=0;
									   BlanceRunType=recBuf[6];									
									   BlancePlaceCount=recBuf[7];									
									   BlanceSecond=recBuf[8]<<8 | recBuf[9];
									   OSSchedUnlock();
									   AT24CXX_WriteOneByte(BLANCERUNTYPE,BlanceRunType);//保存配置;
									   AT24CXX_WriteOneByte(BLANCESECOND,BlanceSecond>>8);//保存配置;
									   AT24CXX_WriteOneByte(BLANCESECOND+1,BlanceSecond);//保存配置;
									   AT24CXX_WriteOneByte(BLANCEPLACECOUNT,BlancePlaceCount);//保存配置;
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
									   //读取平衡系统节拍及灯初始状态;
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
									   //读取设置基站功率;
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
						 //上位机给基站发命令,由基站根据命令处理
						 if ((recBuf[4]==0xAA)||(recBuf[4]==0xAB)){
							 //根据终端号清空待发命令;
							 tmpno=recBuf[7]<<8|recBuf[8];
							 if (tmpno>0) itemindex=SeekTerm(tmpno);
							 //如果终端号存在,则清空发送缓存,AA不清除发送缓存，BB清除发送缓存
							 OSSchedLock();
							 if (recBuf[4]==0xAB) {
								 if (tmpno>0) ClearTermCmd(itemindex);
							 }
							 else notRestSend=0x01;
							 //将后续命令回复给终端,第七位是命令长度;							 
							 if (recBuf[5]>0x00){
								 for (i=0;i<recBuf[5];i++){
									 RelayData[i]=recBuf[6+i];                 
								 }
								 RelayCount=0;//转发次数0;
								 RelayLen=recBuf[5];
							 }
							 //接收成功回复给主机，，别的均由基站代回;
							 stationRtn=0x01;
							 //0xB5,0xFF,下载记录由终端返回记录信息
							 if ((RelayData[0]==0xB5)&&(RelayData[3]==0xFF)) stationRtn=0x00;
							 //0xC5,0xFA询终端节拍状态位;
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
						 if((recBuf[4]==0xFA)||(recBuf[4]==0xFB)||(recBuf[4]==0xFC))  //注册或者呼叫则用模块B
						 {
							 LEDB_Tx=0x00;
							 CC_SendPacket(recBuf,16,RF_B);
							 delay_ms(3);
							 LEDB_Tx=0x01;
						 }
						 else  //直接转发;
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
			 
  	  //清空发送命令标志;
			if (notRestSend==0x00){
				OSSchedLock();//上锁
				AtSendCmd=0x00;
				FreeCount=0x00;
				OSSchedUnlock();//解锁;			 
			}
    }
    return 0;
}

//清除所有状态;
void ClearTermStauts(int paramCount)//清除所有状态;
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

//保存终端列表
void SaveTerm(int index){
	u8 urlh,urll;
	if (index>=128) return;
	urlh=AskTermList[index]>>8;
	urll=AskTermList[index];
  AT24CXX_WriteOneByte(ASKTERMLIST+index*3,urlh);
	AT24CXX_WriteOneByte(ASKTERMLIST+index*3+1,urll);
	AT24CXX_WriteOneByte(ASKTERMLIST+index*3+2,0x01);
	//下一个为空;
  AT24CXX_WriteOneByte(ASKTERMLIST+index*3+3,0x00);
	AT24CXX_WriteOneByte(ASKTERMLIST+index*3+4,0x00);
	AT24CXX_WriteOneByte(ASKTERMLIST+index*3+5,0x00);	
}

//清除所有终端列表;
void DeleteTerm(void){
  AT24CXX_WriteOneByte(ASKTERMLIST,0);
	AT24CXX_WriteOneByte(ASKTERMLIST+1,0);
	AT24CXX_WriteOneByte(ASKTERMLIST+2,0x00);	
  AT24CXX_WriteOneByte(ASKTERMLIST+3,0);
	AT24CXX_WriteOneByte(ASKTERMLIST+4,0);
	AT24CXX_WriteOneByte(ASKTERMLIST+5,0x00);		
}
//清除缓存命令;
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

//清除所有暂存命令;
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

//发送心跳包;
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
	sendBuf[3]=0xEE;//心跳包
	sendBuf[4]=0x0A;//默认命令长度为07;
	sendBuf[5]=Bpoint;//B频点;
	sendBuf[6]=Circle;//周期;
	sendBuf[7]=MinSignal;//信号阀值;
	for(checksum=0,i=0;i<8;i++)
		checksum=checksum+sendBuf[i];
	sendBuf[8]=checksum>>8;
	sendBuf[9]=checksum;
	//通过A频点轮循
	delay_ms(10);
	CC_SendPacket(sendBuf,0x0A,RF_A);
	delay_ms(2);
	LEDA_Tx=0x01;	
	LEDB_Rx=0x01;
}

