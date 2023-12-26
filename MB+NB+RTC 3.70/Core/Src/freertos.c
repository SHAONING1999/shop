/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"  
#include "gpio.h"
#include "NBiot.h"
#include "EC20.h"
#include <stdio.h>
#include <stdlib.h>
#include "tim.h"
#include "cJSON.h"
#include "cJSON_Process.h"
#include "w25qxx.h"
#include "mbcrc.h"
#include "modbus.h"
#include "spi.h"
#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "stm32f4xx_hal_rtc.h"
#include "adxl345.h"
#include "iwdg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

 
 

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void Analysis_USART3_5_DATE(void);//串口3-5（传感器1）数据处理函数
void Analysis_USART5_3_DATE(void);//串口5-3（传感器2）数据处理函数
void Date_Report_tcp(void);//数据上传函数

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
	char wbuf[500];//准备写入flash的数据缓存
	char rbuf[500];//读出flash的数据的缓存
	char reissue_buff[500];//补发数据缓存
	char message_buff[500];//历史数据缓存
	int page;//数据写入页数变量
	uint8_t cycle[2],rep[2];//周期数据缓存数组

	uint16_t datebuff[20];//存储modbus传回的数据
	
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	int rSamplingTime=600,rReportingTime=600;//采样周期，上报周期
	int cycle_time1=-1;
	 int rwork_state=2 ;//默认间歇工作
	uint16_t crc;//计算crc和接收到的crc
	int flow,level,velocity;//数据
	float COD,turbidity;
	uint8_t date_flag=0;//数据读取情况标志位
	
	uint8_t BLE_Flag = 0; //一帧数据接收完成标志
	volatile uint8_t Code_Versions[10] = "2023.8.10"; //一帧数据接收完成标志
	int rsensornum=0;
	int rsensornum_err=0;
	int sensornum=0;
	int max_page=0;
	struct senser senser1;
	struct senser senser2;
	struct senser rsenser1; 
	struct senser rsenser2;
	uint16_t ADC_Value[100]={0};
#define DATE_SIZE sizeof(tbuf)
	
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern IWDG_HandleTypeDef hiwdg;
/* USER CODE END 0 */
DataU DataTrans;//w25q128存入浮点数结构体
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */


/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTask02Handle;
osThreadId myTask03Handle;
osThreadId myTask04Handle;
osSemaphoreId NET_FLAGHandle;
osSemaphoreId ALM_FLAGHandle;
osSemaphoreId DATE_SENT_FLAGHandle;
osSemaphoreId BLE_DEBUG_FLAGHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);
void StartTask04(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of NET_FLAG */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityHigh, 0,1000);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTask02 */
  osThreadDef(myTask02, StartTask02, osPriorityAboveNormal, 0, 500);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

  /* definition and creation of myTask03 */
  osThreadDef(myTask03, StartTask03, osPriorityNormal, 0, 3000);
  myTask03Handle = osThreadCreate(osThread(myTask03), NULL);

  /* definition and creation of myTask04 */
  osThreadDef(myTask04, StartTask04, osPriorityBelowNormal, 0, 256);
  myTask04Handle = osThreadCreate(osThread(myTask04), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
__weak void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	DEVICE_Init();//设备初始化
//	int	reissue_page=0;
	W25QXX_Read((uint8_t*)&rsensornum,SENSORNUM_ADDR, sizeof(int));
	W25QXX_Read((uint8_t*)&rsenser1,SNE1DATE_ADDR, sizeof(rsenser1));
	W25QXX_Read((uint8_t*)&rsenser2,SNE2DATE_ADDR, sizeof(rsenser2));
	W25QXX_Read((uint8_t*)&rSamplingTime,SamplingTime_ADDR, sizeof(int));
	W25QXX_Read((uint8_t*)&rReportingTime,Reportingtime_ADDR, sizeof(int));
	printf("采样时间=%d\r\n",rSamplingTime);
	printf("上报时间=%d\r\n",rReportingTime);
	max_page=rReportingTime/rSamplingTime;
	printf("max_page=%d\r\n",max_page);
	W25QXX_Read((uint8_t*)&rwork_state,WORK_STATE_ADDR, sizeof(int));//读取设备工作状态,1连续工作。2间歇工作
//	HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, rSamplingTime, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);//设置采样时间
	osDelay(500);  
	float PWR_Tvalue= W25Q128_Read_float(PWR_Tvalue_ADDR);
	printf("电池低电压报警阈值：%2.2fV\r\n",PWR_Tvalue);
	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_Value,100);//开启DMA传输，ADC开始转换
	osDelay(500);
	modbus.epower=(((GET_Voltage_value(ADC_Value)*5.3)+0.73)*1.063918538+0.018723337);//电量信息传递给报文数据结构
	printf ("电池电压:%.3f\r\n",modbus.epower);
	int t=0;
  /* Infinite loop */
  for(;;)
  {	
	 
	if(rec1_end_flag)  //判断是否USART1接收到1帧数据
	{Usart1_Handle(); }	 //前往数据处理函数处理接收到的数据。
	if(rec2_end_flag)  //判断是否USART2接收到1帧数据
	{Usart2_Handle(); }//前往数据处理函数处理接收到的数据。
	if(rec6_end_flag)  //判断是否USART6接收到1帧数据
	{Usart6_Handle(); }//前往数据处理函数处理接收到的数据。		
	t++;
	if(t>=500)
	{
	//工作状态指示
	t=0;
	HAL_GPIO_TogglePin (PWR_LED_GPIO_Port, PWR_LED_Pin);
	//喂狗
	HAL_IWDG_Refresh(&hiwdg);
	}
	osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/ 
/* USER CODE END Header_StartTask02 */
__weak void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
	{	

    osDelay(10);
	}
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the myTask03 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
__weak void StartTask03(void const * argument)
{
		osDelay(3000);
  /* USER CODE BEGIN StartTask03 */
	//传感器1初始化
	 for(;;)
	{
		if(cycle_time1>=rSamplingTime||cycle_time1==-1)//到规定时间，并且不是第一次采集
		{	
			cycle_time1=0;
			W25QXX_Read((uint8_t*)&rsensornum,SENSORNUM_ADDR, sizeof(int));
			if((rsensornum==1)||(rsensornum==3))//如果需要使用传感器1
			{
				W25QXX_Read((uint8_t*)&rsenser1,SNE1DATE_ADDR, sizeof(rsenser1));
				printf("传感器1从机地址:%d\r\n",rsenser1.SlaveAddr);
				printf("传感器1延时时间:%d\r\n",rsenser1.Delays);
			for(int i=0;i<rsenser1.DivisorNum;i++)
			{
				printf("因子名称:%s ",rsenser1.Divisorname[i]);
				printf("寄存器起始地址:%d ",rsenser1.RegisterAddress[i]);
				printf("数据个数:%d ",rsenser1.datenum[i]);
				printf("数据类型:%d ",rsenser1.datetype[i]);
				printf("解析方式:%d\r\n",rsenser1.prasetype[i]);				
			}		
			sensor1_init(&rsenser1,USART3_RX_BUF);
			}
			//传感器2初始化
			if((rsensornum==2)||(rsensornum==3))//如果需要使用传感器2
			{
				W25QXX_Read((uint8_t*)&rsenser2,SNE2DATE_ADDR, sizeof(rsenser2));
				printf("传感器2从机地址:%d\r\n",rsenser2.SlaveAddr);
				printf("传感器2延时时间:%d\r\n",rsenser2.Delays);
				for(int i=0;i<rsenser2.DivisorNum;i++)
				{
					printf("因子名称:%s ",rsenser2.Divisorname[i]);
					printf("寄存器起始地址:%d ",rsenser2.RegisterAddress[i]);
					printf("数据个数:%d ",rsenser2.datenum[i]);
					printf("数据类型:%d ",rsenser2.datetype[i]);
					printf("解析方式:%d\r\n",rsenser2.prasetype[i]);				
				}		
				sensor2_init(&rsenser2,USART5_RX_BUF);
			} 			
			
		}
	  if((sensornum==rsensornum)&&(sensornum!=0))
	  {
		Date_Report_tcp();//数据上传
		sensornum=0;//传感器采集状态初始化
	  }
	osDelay(10);
    }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the myTask04 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
__weak void StartTask04(void const * argument)
{
  /* USER CODE BEGIN StartTask04 */
	//蓝牙调试打印任务
	osDelay(6000);
  /* Infinite loop */
  for(;;)
  {
	  	cycle_time1++;
//	  int monitor=-1;
//	W25QXX_Read((uint8_t*)&monitor,MONITOR_ADDR, sizeof(int));
//	  if(BLE_Flag==1||monitor==1)// 如果外部申请蓝牙连接
//	  {		
//		 //蓝牙模块供电
//		HAL_GPIO_WritePin(BLE_PWR_EN_GPIO_Port, BLE_PWR_EN_Pin, GPIO_PIN_SET);
//		  
////		int rSamplingTime,rReportingTime;
//		char buff[40];//ip和端口缓存
//		char ipbuff[36];//ip和端口缓存
//		char portbuff[4];//ip和端口缓存
//		char* p2; 
//		
//		W25QXX_Read( (uint8_t*)buff,IP_ADDR,40);//获取IP
//		p2=strtok(buff,";");//字符分割
//		sprintf(ipbuff,p2);//IP填充
//		p2=strtok(NULL,";");
//		sprintf(portbuff,p2);//端口号填充
//		float gps_lonngitude= W25Q128_Read_float(LONGITUDE_ADDR);
//		float gps_latitude=W25Q128_Read_float(LATITUDE_ADDR);

//		//状态指示
//		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
////		//打印设备配置信息：
////		printf("设备采样时间：%dS\r\n",rSamplingTime);
////		printf("设备数据上传时间：%dS\r\n",rReportingTime);
////		printf("电池低电压报警阈值：%fV\r\n",W25Q128_Read_float(PWR_Tvalue_ADDR));
////		printf("上传平台IP及端口号：%s;%s\r\n",ipbuff,portbuff);
////		printf("设备位置信息：设备经度：%f;设备纬度：%f\r\n",gps_lonngitude,gps_latitude);
////		//采集加速度数据
////		
//		osDelay(1000);			
//	  }
//		short x,y,z,xang,yang,zang;
//	  ADXL345_Read_Average(&x,&y,&z,10);  //读取x,y,z 3个方向的加速度值 总共10次	
//		//采集倾斜角数据 
//		xang=ADXL345_Get_Angle(x,y,z,1);
//		yang=ADXL345_Get_Angle(x,y,z,2);
//		zang=ADXL345_Get_Angle(x,y,z,0);  
////		printf("X轴倾斜角:%d,Y轴倾斜角:%d,Z轴倾斜角:%d\r\n",xang,yang,zang);	
	  osDelay(1000);
//	  Tilt_check();//倾斜度检测
  }
  /* USER CODE END StartTask04 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */



//数据填报+上传
void Date_Report_tcp(void)
{			
			int reissue_page=0;//补发页数
			char head[6];//报文头缓存
			char tail[8];//报文尾缓存
			char rMN[20];//MN号缓存
			int rmod;//设备通讯模式缓存
			int writepage;//历史数据缓存页数
			float gps_lonngitude, gps_latitude; //经度数据读取缓冲//纬度数据读取缓冲
			gps_lonngitude= W25Q128_Read_float(LONGITUDE_ADDR);
			gps_latitude=W25Q128_Read_float(LATITUDE_ADDR);
			printf("设备经度：%f\r\n",gps_lonngitude);
			printf("设备纬度：%f\r\n",gps_latitude);
			W25QXX_Read((uint8_t*)rMN,MN_ADDR, 20*sizeof(char));
			W25QXX_Read((uint8_t*)&rmod,COMMOD_ADDR, sizeof(int));//读取通讯模式
			W25QXX_Read((uint8_t*)&writepage,Datewritepage_ADDR, sizeof(int));//读取写入数据页数
			W25QXX_Read((uint8_t*)&rSamplingTime,SamplingTime_ADDR, sizeof(int));
			W25QXX_Read((uint8_t*)&rReportingTime,Reportingtime_ADDR, sizeof(int));
			printf("writepage：%d\r\n",writepage);
			HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_Value,100);//开启DMA传输，ADC开始转换
			modbus.epower=(((GET_Voltage_value(ADC_Value)*5.3)+0.73)*1.063918538+0.018723337);//电量信息传递给报文数据结构
			printf ("电池电压:%.3f\r\n",modbus.epower);
			
			HAL_RTC_GetTime (&hrtc,&sTime,RTC_FORMAT_BIN);
			HAL_RTC_GetDate (&hrtc,&sDate,RTC_FORMAT_BIN);
			sprintf((char*)wbuf,"QN=%02d%02d%02d%02d%02d%02d;ST=21;CN=2011;PW=123456;MN=%s;FLAG=5;CP=&&DataTime=%02d%02d%02d%02d%02d%02d;YSRTU01-Rtd=%.3f;YSRTU02-Rtd=%f;YSRTU03-Rtd=%f;",
										sDate.Year+1952 ,sDate.Month,sDate.Date,sTime.Hours,sTime.Minutes,sTime.Seconds,rMN,
										sDate.Year+1952,sDate.Month,sDate.Date,sTime.Hours,sTime.Minutes,sTime.Seconds,
										modbus.epower,gps_lonngitude,gps_latitude); 
			//数据填充
			if(sensornum==1)//如果只接了传感器1
			Sensor1_Date_Add(&rsenser1,wbuf);//传感器1数据填充
			if(sensornum==2)//如果只接了传感器2
			Sensor2_Date_Add(&rsenser2,wbuf);//传感器2数据填充
			if(sensornum==3)//如果传感器1，2都接了
			{
				Sensor1_Date_Add(&rsenser1,wbuf);//传感器1数据填充
				Sensor2_Date_Add(&rsenser2,wbuf);//传感器2数据填充
			}
			
			
			//加入报头
			modbus.date_num=countDataInCharArray(wbuf);//获取报文字节数(212格式前缀)
			sprintf(head,"##%04d",modbus.date_num);
			crc = CRC16_Checkout((unsigned char*)wbuf,countDataInCharArray(wbuf)); //计算CRC校验位(212格式后缀)
			char *newString = prependString(head, wbuf);
			if (newString != NULL)
			{
//			printf("加入报头后:%s\r\n", newString);
			}
			//加入报尾
			sprintf(tail,"%04X\r\n",crc);
			strcat(newString, tail);
			printf("加入报尾后%s\r\n",newString);
	
			
	if((((writepage+1)>=max_page))||(max_page==1))//如果采样数据次数需要上传，或者为实时采样上报模式
	{
			//进行网络连接
			if((rmod==1)&&(Test_NB_STA_TCP((char*)newString))==0)
			{
				free(newString);	
				return;//NB建立SOCKET失败，直接返回
			}
			if(rmod==2&&Test_4G_STA_TCP((char*)newString)==0)
			{
				free(newString);	
				return;//EC20 socket建立失败，直接返回
			}
				
//			if(rmod==3)
	//如果是实时采集上报模式
		printf("M采样时间=%d\r\n",rSamplingTime);
		printf("M上报时间=%d\r\n",rReportingTime);
		max_page=(rReportingTime/rSamplingTime);
		printf("max_page=%d\r\n",max_page);
			if(max_page==1)
		{
			//NB-iot通讯
			if(rmod==1)
			{
				if(TCP_Sent_Date_NB((char*)newString))//上报最新检测到的数据成功
				{
					//进行数据补发
					W25QXX_Read( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发页数	
					printf("待补发数据页数:%d\r\n",reissue_page);
					while(reissue_page)
					{	
						
						W25QXX_Read( (uint8_t*)reissue_buff,REISSUE_MESSAGE_ADDR+500*(reissue_page-1),500);//读取需要补发的内容
						if(TCP_Sent_Date_NB(reissue_buff))//如果补发成功
						{
						reissue_page--;
						printf("数据补发中......\r\n");
						printf("剩余页数：%d\r\n",reissue_page);
						W25QXX_Write( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发完成，页数清零	
						}
						else//补发失败
						{
						printf("数据补发失败\r\n");
						BC260_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"BC260:TCP网络端口断开失败,即将重启");
						printf("rwork_state:%d\r\n",rwork_state);
						if(rwork_state==2)
						{
						START_STANDBY();//进入待机
						}
						free(newString);
						HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_RESET);//NB模块供电
						return;
						}
						if(reissue_page==0)//如果所有数据补发完成
						{
						printf("数据补发完成\r\n");
						BC260_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"BC260:TCP网络端口断开失败,即将重启");
						W25QXX_Write( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发完成，页数清零
							printf("rwork_state:%d\r\n",rwork_state);
						if(rwork_state==2)
						{
						START_STANDBY();//进入待机
						}
						free(newString);
						HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_RESET);//NB模块供电
						return;
						}
					}
					BC260_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"BC260:TCP网络端口断开失败,即将重启");
					memset(rbuf,0,countDataInCharArray(newString));//清除缓存
					// 记得释放内存
					free(newString);		
					printf("rwork_state:%d\r\n",rwork_state);
					if(rwork_state==2)
					{
					START_STANDBY();//进入待机
					}
					HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_RESET);//NB模块供电
					return;
				}
				else//上报最新检测到的数据失败
				{
					printf("新数据上传失败，备份数据\r\n");
					BC260_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"BC260:TCP网络端口断开失败,即将重启");
					if(rwork_state==2)
					{
					START_STANDBY();//进入待机
					}
					free(newString);
					HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_RESET);//NB模块供电
					return;
				}
				
			}
			//4G通讯
			if(rmod==2)
			{	
				if(TCP_Sent_Date_4G((char*)newString))//上报最新检测到的数据成功
				{
					//进行数据补发
					W25QXX_Read( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发页数	
					printf("待补发数据页数:%d\r\n",reissue_page);
					while(reissue_page)
					{
						W25QXX_Read( (uint8_t*)reissue_buff,REISSUE_MESSAGE_ADDR+500*(reissue_page-1),500);//读取需要补发的内容
						if(TCP_Sent_Date_4G(reissue_buff))//如果补发成功
						{
						printf("数据补发中......\r\n");
						reissue_page--;
						printf("剩余页数：%d\r\n",reissue_page);
						W25QXX_Write( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发完成，页数清零	
						}
						else//补发失败
						{
						printf("数据补发失败\r\n");
						EC20_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");
						//EC20_send_cmd_LOOP("AT+QPOWD=1\r\n","OK",1000,5,1,"EC20关机失败");
						osDelay(300);
						if(rwork_state==2)
						{
						START_STANDBY();//进入待机
						}
						free(newString);
						HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_RESET);//4G模块断电
						return;
						}
						if(reissue_page==0)//如果所有数据补发完成
						{
						printf("数据补发完成\r\n");
						EC20_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");
						//EC20_send_cmd_LOOP("AT+QPOWD=1\r\n","OK",1000,5,1,"EC20关机失败");
						osDelay(300);
						W25QXX_Write( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发完成，页数清零
						if(rwork_state==2)
						{
						START_STANDBY();//进入待机
						}
						free(newString);
						HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_RESET);//4G模块断电
						return;
						}
					}
					memset(rbuf,0,countDataInCharArray(newString));//清除缓存
					// 记得释放内存
					free(newString);	
					EC20_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");		
					//EC20_send_cmd_LOOP("AT+QPOWD=1\r\n","OK",1000,5,1,"EC20关机失败");
					osDelay(300);					
					if(rwork_state==2)
					{
					START_STANDBY();//进入待机
					}
					HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_RESET);//4G模块断电
					return;
				}
				else//上报最新检测到的数据失败
				{
					printf("新数据上传失败，备份数据\r\n");
					EC20_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");
				//	EC20_send_cmd_LOOP("AT+QPOWD=1\r\n","OK",1000,5,1,"EC20关机失败");
					osDelay(300);
					if(rwork_state==2)
					{
					START_STANDBY();//进入待机
					}
					free(newString);
					HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_RESET);//4G模块断电
					return;
				}
			}
		}	
	//如果是周期采集上报模式
		
		else if(max_page>1)
		{	//NB-iot通讯

			if(rmod==1)
			{
				for(int rp=0;((rp+1)<=max_page);rp++)//数据逐条上传
				{
					if((rp+1)<max_page)//上报周期前采集到的数据上传
					{
					printf("周期writepage:%d\r\n",rp);
					W25QXX_Read( (uint8_t*)message_buff,MESS_ADDR+500*rp,500);//读取数据
//					printf("writebuff:%s\r\n",message_buff);
					TCP_Sent_Date_NB((char*)message_buff);//上报检测到的数据
					}
					if((rp+1)==max_page)//上报周期内采集到的数据上传
					{
					TCP_Sent_Date_NB((char*)newString);//上报检测到的数据
					writepage=0;
					W25QXX_Write( (uint8_t*)&writepage,Datewritepage_ADDR,sizeof(int));//发送完成，页数清零
					free(newString);		
					}
				}
				//这一次的数据发送完成，进行数据补发
				W25QXX_Read( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发页数	
				printf("待补发数据页数:%d\r\n",reissue_page);
				while(reissue_page)
				{	
					W25QXX_Read( (uint8_t*)reissue_buff,REISSUE_MESSAGE_ADDR+500*(reissue_page-1),500);//读取需要补发的内容
					if(TCP_Sent_Date_NB(reissue_buff))//如果补发成功
					{
					reissue_page--;
					printf("数据补发中......\r\n");
					printf("剩余页数：%d\r\n",reissue_page);
					W25QXX_Write( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发完成，页数清零	
					}
					else//补发失败
					{
					printf("数据补发失败\r\n");
					BC260_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");
					if(rwork_state==2)
					{
					START_STANDBY();//进入待机
					}
					HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_RESET);//NB模块供电
					free(newString);
					return;
					}
					if(reissue_page==0)//如果所有数据补发完成
					{
					printf("数据补发完成\r\n");
					BC260_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");
					W25QXX_Write( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发完成，页数清零
					if(rwork_state==2)
					{
					START_STANDBY();//进入待机
					}
					HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_RESET);//NB模块供电
					free(newString);
					return;
					}
				}
				//没有需要补发的数据：
				BC260_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");
				memset(rbuf,0,countDataInCharArray(newString));//清除缓存
				// 记得释放内存
				free(newString);				
				if(rwork_state==2)
				{
				START_STANDBY();//进入待机
				}
				HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_RESET);//NB模块供电
				return;
			}
			
			//4G通讯
			if(rmod==2)
			{
				for(int rp=0;(rp+1)<=max_page;rp++)//数据逐条上传
				{
					if((rp+1)<max_page)//上报周期前采集到的数据上传
					{
					printf("writepage:%d\r\n",rp);
					W25QXX_Read( (uint8_t*)message_buff,MESS_ADDR+500*rp,500);//读取数据
//					printf("writebuff:%s\r\n",message_buff);
					TCP_Sent_Date_4G((char*)message_buff);//上报检测到的数据
					}
					if((rp+1)==max_page)//上报周期内采集到的数据上传
					{
						TCP_Sent_Date_4G((char*)newString);//上报检测到的数据
						writepage=0;
						W25QXX_Write( (uint8_t*)&writepage,Datewritepage_ADDR,sizeof(int));//发送完成，页数清零
					}
				}
				//这一次的数据发送完成，进行数据补发
				W25QXX_Read( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发页数	
				printf("待补发数据页数:%d\r\n",reissue_page);
				while(reissue_page)
				{	
					W25QXX_Read( (uint8_t*)reissue_buff,REISSUE_MESSAGE_ADDR+500*(reissue_page-1),500);//读取需要补发的内容
					if(TCP_Sent_Date_4G(reissue_buff))//如果补发成功
					{
					reissue_page--;
					printf("数据补发中......\r\n");
					printf("剩余页数：%d\r\n",reissue_page);
					W25QXX_Write( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发完成，页数清零	
					}
					else//补发失败
					{
					printf("数据补发失败\r\n");
					EC20_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");
					//EC20_send_cmd_LOOP("AT+QPOWD=1\r\n","OK",1000,5,1,"EC20关机失败");
					osDelay(300);
					if(rwork_state==2)
					{
					START_STANDBY();//进入待机
					}
					free(newString);
					HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_RESET);//4G模块断电
					return;
					}
					if(reissue_page==0)//如果所有数据补发完成
					{
					printf("数据补发完成\r\n");
					EC20_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");
					//EC20_send_cmd_LOOP("AT+QPOWD=1\r\n","OK",1000,5,1,"EC20关机失败");
					osDelay(300);	
					W25QXX_Write( (uint8_t*)&reissue_page,REISSUE_PAG_ADDR,sizeof(int));//补发完成，页数清零
					if(rwork_state==2)
					{
					START_STANDBY();//进入待机
					}
					free(newString);
					HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_RESET);//4G模块断电
					return;
					}
				}
				//没有需要补发的数据：
				EC20_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",1000,5,1,"EC20:TCP网络端口断开失败,即将重启");
				//EC20_send_cmd_LOOP("AT+QPOWD=1\r\n","OK",1000,5,1,"EC20关机失败");
				osDelay(300);	
				memset(rbuf,0,countDataInCharArray(newString));//清除缓存
				// 记得释放内存
				free(newString);				
				if(rwork_state==2)
				{
				START_STANDBY();//进入待机
				}
				HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_RESET);//4G模块断电
				return;
				
			}
			
		}	
	}
	//保存采样数据，直到需要上报数据的时候
	else
	{
			printf("数据保存页数：%d\r\n",writepage);
			printf("未到上报时间,保存数据\r\n");
			W25QXX_Read( (uint8_t*)&writepage,Datewritepage_ADDR,sizeof(int));//读取之前缓存的页数
			W25QXX_Write( (uint8_t*)newString,MESS_ADDR+writepage*500,500);//进行数据备份
			writepage++;
			W25QXX_Write( (uint8_t*)&writepage,Datewritepage_ADDR,sizeof(int));//更新缓存页数
			if(rwork_state==2)
			{
			START_STANDBY();//进入待机
			}
			free(newString);
			HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_RESET);//NB模块供电
			HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_RESET);//4G模块断电
			return;
	}
	
}
/* USER CODE END Application */
