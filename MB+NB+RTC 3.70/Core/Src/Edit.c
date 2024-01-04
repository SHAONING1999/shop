#include "string.h"
#include "stdio.h"
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "modbus.h"
#include "mbcrc.h"
#include "usart.h"
#include <stdlib.h>
#include "w25qxx.h"
#include "gpio.h"
#include "rtc.h"
#include "adc.h"

//传感器1初始化
	extern int rsensornum;
	extern int rsensornum_err;
	extern int sensornum;
	float VERSION;//系统版本
	
	
void sensor1_init(struct senser* sen1,uint8_t* buff)
{	//传感器1通电
	int t;
	uint8_t date=0;
	HAL_GPIO_WritePin(GPIOC, SENS1_PWR_EN_Pin, GPIO_PIN_SET);
	for(int i=0;i<sen1->Delays;i++)
	{osDelay(1000);}
	//配置的报文发送

	for(t=0;t<sen1->DivisorNum;t++)
	{
		printf("传感器1开始检测\r\n");
		Host_Read03_slave1(sen1->SlaveAddr,sen1->RegisterAddress[t],sen1->datenum[t]);
		osDelay(1000);
		if(USART3_RX_STA&0x8000)//如果接收完成（USART_RX_STA==0时为未完成，为1时为接收完成
		{
		date++;
		Sensor1_Date_Prase(sen1,buff,t);//数据处理
		clear_BUF(buff);//清除缓冲区
		}		
	}
	if(date==sen1->DivisorNum)
	{
	sensornum+=1;//传感器1数据采集成功
	HAL_GPIO_WritePin(GPIOC, SENS1_PWR_EN_Pin, GPIO_PIN_RESET);//传感器1断电
	}
	else//采集失败
	{
		sensornum+=1;
		rsensornum_err=1;//传感器1数据采集失败
	}
	
	
}

//传感器2初始化
void sensor2_init(struct senser* sen1,uint8_t* buff)
{	//传感器2通电
	int t;
	uint8_t date=0;
    HAL_GPIO_WritePin(GPIOA, SEN2_PWR_EN_Pin, GPIO_PIN_SET);
	for(int i=0;i<sen1->Delays;i++)
	{osDelay(1000);}
	//配置的报文发送

	for(t=0;t<sen1->DivisorNum;t++)
	{
		printf("传感器2开始检测\r\n");
		Host_Read03_slave2(sen1->SlaveAddr,sen1->RegisterAddress[t],sen1->datenum[t]);
		osDelay(1000);
		if(USART5_RX_STA&0x8000)//如果接收完成（USART_RX_STA==0时为未完成，为1时为接收完成
		{
		date++;
		Sensor2_Date_Prase(sen1,buff,t);//数据处理
		clear_BUF(buff);//清除缓冲区
		}		
	}
	if(date==(sen1->DivisorNum))
	{
	sensornum+=2;//传感器1数据采集成功
	HAL_GPIO_WritePin(GPIOA, SEN2_PWR_EN_Pin, GPIO_PIN_RESET);
	}
	else
	{
		sensornum+=2;
		rsensornum_err=2;//传感器2数据采集失败
	}

	
}



//数据处理函数
//参数： struct senser seb 可配置报文结构体
//buff 串口接收缓存
//t   已发送报文条数
void Sensor1_Date_Prase(struct senser* seb,uint8_t* buff,int t)
{
	//选择解析方式，对数据进行初步的排列
	switch(seb->prasetype[t])
	{
		case 0://小端，不做处理
			break;
		case 1://小端交换，相邻字节交换
			BigLittleEndianSwap(&buff[3], 2*seb->datenum[t]);
			break;
		case 2://大端，
			EndianSwap(&buff[3], 2*seb->datenum[t]);
			break;
		case 3://大端交换
			EndianSwap(&buff[3], 2*seb->datenum[t]);
			BigLittleEndianSwap(&buff[3], 2*seb->datenum[t]);
			break;
	}

	if(seb->datetype[t]==0)//
	{
	 memcpy(&seb->Divisorshort[t],&buff[3],2);
	printf("short:%d\r\n",seb->Divisorshort[t]);
	}
	if(seb->datetype[t]==1)//
	{
	 memcpy(&seb->Divisorint[t],&buff[3],4);
	printf("int:%d\r\n",seb->Divisorint[t]);
	}
	if(seb->datetype[t]==2)//
	{
	 memcpy(&seb->Divisorfloat[t],&buff[3],4);
	printf("float:%f\r\n",seb->Divisorfloat[t]);
	}
	if(seb->datetype[t]==3)//
	{
	 memcpy(&seb->Divisordouble[t],&buff[3],8);
	printf("double:%lf\r\n",seb->Divisordouble[t]);
	}
	if(seb->datetype[t]==4)//
	{
	 memcpy(&seb->Divisorlonglong[t],&buff[3],2*seb->datenum[t]);
	printf("long long:%lld\r\n",seb->Divisorlonglong[t]);
	}
}

void Sensor2_Date_Prase(struct senser* seb,uint8_t* buff,int t)
{
	//选择解析方式，对数据进行初步的排列
	switch(seb->prasetype[t])
	{
		case 0://小端，不做处理
			break;
		case 1://小端交换，相邻字节交换
			BigLittleEndianSwap(&buff[3], 2*seb->datenum[t]);
			break;
		case 2://大端，
			EndianSwap(&buff[3], 2*seb->datenum[t]);
			break;
		case 3://大端交换
			EndianSwap(&buff[3], 2*seb->datenum[t]);
			BigLittleEndianSwap(&buff[3], 2*seb->datenum[t]);
			break;
		
	}
if(seb->datetype[t]==0)//short
	{
	 memcpy(&seb->Divisorshort[t],&buff[3],2);
	printf("short:%d\r\n",seb->Divisorshort[t]);
	}
	if(seb->datetype[t]==1)//int
	{
	 memcpy(&seb->Divisorint[t],&buff[3],4);
	printf("int:%d\r\n",seb->Divisorint[t]);
	}
	if(seb->datetype[t]==2)//float
	{
	 memcpy(&seb->Divisorfloat[t],&buff[3],4);
	printf("float:%f\r\n",seb->Divisorfloat[t]);
	}
	if(seb->datetype[t]==3)//double
	{
	 memcpy(&seb->Divisordouble[t],&buff[3],8);
	printf("double:%lf\r\n",seb->Divisordouble[t]);
	}
	if(seb->datetype[t]==4)//
	{
	 memcpy(&seb->Divisorlonglong[t],&buff[3],2*seb->datenum[t]);
	printf("long long:%lld\r\n",seb->Divisorlonglong[t]);
	}
}

//数据填充函数
//参数：
//senser seb 可配置报文信息结构体
//已发送报文条数
//需要追加的报文地址
void Sensor1_Date_Add(struct senser* seb,char* sendbuff)
{
	char tempbuff[30];//临时数据缓存
	//选择填充的数据类型
	for(int i=0;i<seb->DivisorNum ;i++)
	{
		if(rsensornum_err!=1)//传感器正常应答
		{
			if(rsensornum==1)//如果需要使用传感器1
			{
			
				if((i+1)!=seb->DivisorNum)//如果不是最后一个因子，带上分号
				{
					switch(seb->datetype[i])
					{
						case 0://short
							sprintf((char*)tempbuff,"%s=%d;",seb->Divisorname[i],seb->Divisorshort[i]);
							break;
						case 1://int
							sprintf((char*)tempbuff,"%s=%d;",seb->Divisorname[i],seb->Divisorint[i]);
							break;
						case 2://float
							sprintf((char*)tempbuff,"%s=%f;",seb->Divisorname[i],seb->Divisorfloat[i]);
							break;
						case 3://double
							sprintf((char*)tempbuff,"%s=%lf;",seb->Divisorname[i],seb->Divisordouble[i]);
							break;
						case 4://long long
							sprintf((char*)tempbuff,"%s=%lld;",seb->Divisorname[i],seb->Divisorlonglong[i]);
							break;
					}
					strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾
				}
			
				if((i+1)==seb->DivisorNum)//如果是最后一个因子，不带上分号
				{		
					switch(seb->datetype[i])
					{
						case 0://short
							sprintf((char*)tempbuff,"%s=%d&&",seb->Divisorname[i],seb->Divisorshort[i]);
							break;
						case 1://int
							sprintf((char*)tempbuff,"%s=%d&&",seb->Divisorname[i],seb->Divisorint[i]);
							break;
						case 2://float
							sprintf((char*)tempbuff,"%s=%f&&",seb->Divisorname[i],seb->Divisorfloat[i]);
							break;
						case 3://double
							sprintf((char*)tempbuff,"%s=%lf&&",seb->Divisorname[i],seb->Divisordouble[i]);
							break;
						case 4://long long
							sprintf((char*)tempbuff,"%s=%lld&&",seb->Divisorname[i],seb->Divisorlonglong[i]);
							break;
					}
					strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾
				}
			}
			if(rsensornum==3)//如果需要使用传感器1和传感器2
			{

				switch(seb->datetype[i])
				{
					case 0://short
						sprintf((char*)tempbuff,"%s=%d;",seb->Divisorname[i],seb->Divisorshort[i]);
						break;
					case 1://int
						sprintf((char*)tempbuff,"%s=%d;",seb->Divisorname[i],seb->Divisorint[i]);
						break;
					case 2://float
						sprintf((char*)tempbuff,"%s=%f;",seb->Divisorname[i],seb->Divisorfloat[i]);
						break;
					case 3://double
						sprintf((char*)tempbuff,"%s=%lf;",seb->Divisorname[i],seb->Divisordouble[i]);
						break;
					case 4://long long
							sprintf((char*)tempbuff,"%s=%lld;",seb->Divisorname[i],seb->Divisorlonglong[i]);
							break;
				}
				strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾	
			}
		}
		if(rsensornum_err==1)//传感器异常应答
		{
			if(rsensornum==1)//如果只需要使用传感器1
			{
			
				if((i+1)!=seb->DivisorNum)//如果不是最后一个因子，带上分号
				{
					switch(seb->datetype[i])
					{
						case 0://short
							sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
							break;
						case 1://int
							sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
							break;
						case 2://float
							sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
							break;
						case 3://double
							sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
							break;
						case 4://long long
							sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
							break;
					}
					strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾
				}
			
				if((i+1)==seb->DivisorNum)//如果是最后一个因子，不带上分号
				{		
					switch(seb->datetype[i])
					{
						case 0://short
							sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
							break;
						case 1://int
							sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
							break;
						case 2://float
							sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
							break;
						case 3://double
							sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
							break;
						case 4://long long
							sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
							break;
					}
					strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾
				}
			}
			if(rsensornum==3)//如果需要同时使用传感器1和传感器2
			{

				switch(seb->datetype[i])
				{
					case 0://short
						sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
						break;
					case 1://int
						sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
						break;
					case 2://float
						sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
						break;
					case 3://double
						sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
						break;
					case 4://long long
						sprintf((char*)tempbuff,"%s=0",seb->Divisorname[i]);
						break;
				}
				strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾	
			}
		}
	}
	
}
void Sensor2_Date_Add(struct senser* seb,char* sendbuff)
{	
	char tempbuff[30];//临时数据缓存
	//选择填充的数据类型
	for(int i=0;i<seb->DivisorNum ;i++)
	{
		if(rsensornum_err!=2)//传感器2正常应答
		{
			if((i+1)!=seb->DivisorNum)//如果不是最后一个因子，带上分号
			{
				switch(seb->datetype[i])
				{
					case 0://short
						sprintf((char*)tempbuff,"%s=%d;",seb->Divisorname[i],seb->Divisorshort[i]);
						break;
					case 1://int
						sprintf((char*)tempbuff,"%s=%d;",seb->Divisorname[i],seb->Divisorint[i]);
						break;
					case 2://float
						sprintf((char*)tempbuff,"%s=%f;",seb->Divisorname[i],seb->Divisorfloat[i]);
						break;
					case 3://double
						sprintf((char*)tempbuff,"%s=%lf;",seb->Divisorname[i],seb->Divisordouble[i]);
						break;
					case 4://long long
							sprintf((char*)tempbuff,"%s=%lld;",seb->Divisorname[i],seb->Divisorlonglong[i]);
							break;
				}
				strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾
			}
			
			if((i+1)==seb->DivisorNum)//如果是最后一个因子，不带上分号
			{		
				switch(seb->datetype[i])
				{
					case 0://short
						sprintf((char*)tempbuff,"%s=%d&&",seb->Divisorname[i],seb->Divisorshort[i]);
						break;
					case 1://int
						sprintf((char*)tempbuff,"%s=%d&&",seb->Divisorname[i],seb->Divisorint[i]);
						break;
					case 2://float
						sprintf((char*)tempbuff,"%s=%f&&",seb->Divisorname[i],seb->Divisorfloat[i]);
						break;
					case 3://double
						sprintf((char*)tempbuff,"%s=%lf&&",seb->Divisorname[i],seb->Divisordouble[i]);
						break;
					case 4://long long
							sprintf((char*)tempbuff,"%s=%lld",seb->Divisorname[i],seb->Divisorlonglong[i]);
							break;
				}
				strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾
			}
		}
		
		if(rsensornum_err==2)//传感器异常应答
		{
			if((i+1)!=seb->DivisorNum)//如果不是最后一个因子，带上分号
			{
				switch(seb->datetype[i])
				{
					case 0://short
						sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
						break;
					case 1://int
						sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
						break;
					case 2://float
						sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
						break;
					case 3://double
						sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
						break;
					case 4://long long
						sprintf((char*)tempbuff,"%s=0;",seb->Divisorname[i]);
						break;
				}
				strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾
			}
			
			if((i+1)==seb->DivisorNum)//如果是最后一个因子，不带上分号
			{		
				switch(seb->datetype[i])
				{
					case 0://short
						sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
						break;
					case 1://int
						sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
						break;
					case 2://float
						sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
						break;
					case 3://double
						sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
						break;
					case 4://long long
						sprintf((char*)tempbuff,"%s=0&&",seb->Divisorname[i]);
						break;
				}
				strcat(sendbuff,tempbuff);//把数据追加到指向的字符串末尾
			}
		}
	}
	
}


Config parseConfig(const char* input) {
    Config config;

    // 使用sscanf逐个匹配格式
    sscanf(input, "AT+SET;STIME=%d; RTIME=%d; VALUE=%f; IP=%24[^;]; PORT=%d; LONGITUDE=%f; LATITUDE=%f; MN=%19[^;]; BOAUD1=%d; BOAUD2=%d; COMMOD=%d; SENSORNUM=%d; AGPS=%d; STATE=%d;",
        &config.STIME, &config.RTIME, &config.VALUE, config.IP, &config.PORT,
        &config.LONGITUDE, &config.LATITUDE, config.MN, &config.BOAUD1, &config.BOAUD2,
        &config.COMMOD, &config.SENSORNUM, &config.AGPS, &config.STATE);
    // 打印解析结果
	W25QXX_Write((uint8_t*)&config.STIME,SamplingTime_ADDR, sizeof(int));//采集时间
	W25QXX_Write((uint8_t*)&config.RTIME,Reportingtime_ADDR, sizeof(int));//上报时间
	W25Q128_Write_float(PWR_Tvalue_ADDR,config.VALUE);//电压报警阈值
	W25QXX_Write((uint8_t*)config.IP,IP_ADDR,40);//IP地址
	W25QXX_Write((uint8_t*)&config.PORT,PORT_ADDR,sizeof(int));//端口号
	W25Q128_Write_float(LONGITUDE_ADDR,config.LONGITUDE);//经度
	W25Q128_Write_float(LATITUDE_ADDR,config.LATITUDE);//纬度
	W25QXX_Write((uint8_t*)config.MN,MN_ADDR, 20*sizeof(char));//设备号
	W25QXX_Write((uint8_t*)&config.BOAUD1,BOAUD1_RATE_ADDR, sizeof(int));//485-1波特率
	W25QXX_Write((uint8_t*)&config.BOAUD2,BOAUD2_RATE_ADDR, sizeof(int));//485-2波特率
	W25QXX_Write((uint8_t*)&config.COMMOD,COMMOD_ADDR, sizeof(int));//通讯模式
	W25QXX_Write((uint8_t*)&config.SENSORNUM,SENSORNUM_ADDR, sizeof(int));//传感器数量
	W25QXX_Write((uint8_t*)&config.AGPS,AGPS_ADDR, sizeof(int));//GPS功能是否开启
	W25QXX_Write((uint8_t*)&config.STATE,WORK_STATE_ADDR, sizeof(int));//工作模式
	
	printf("%s\r\n",input);
    return config;
}
///参数查询
int seekConfig()
{
	Config seekconfig;
	memset (&seekconfig,0,sizeof(seekconfig));
	RTC_TimeTypeDef Time = {0};
	RTC_DateTypeDef Date = {0};
	char sendbuff[300];
	uint16_t ADC_Value[100]={0};
	W25QXX_Read((uint8_t*)&seekconfig.STIME,SamplingTime_ADDR, sizeof(int));//采集时间
	W25QXX_Read((uint8_t*)&seekconfig.RTIME,Reportingtime_ADDR, sizeof(int));//上报时间
	seekconfig.VALUE=W25Q128_Read_float(PWR_Tvalue_ADDR);//电压报警阈值
	W25QXX_Read((uint8_t*)seekconfig.IP,IP_ADDR,40);//IP地址
	W25QXX_Read((uint8_t*)&seekconfig.PORT,PORT_ADDR,sizeof(int));//端口号
	seekconfig.LONGITUDE=W25Q128_Read_float(LONGITUDE_ADDR);//经度
	seekconfig.LATITUDE=W25Q128_Read_float(LATITUDE_ADDR);//纬度
	W25QXX_Read((uint8_t*)seekconfig.MN,MN_ADDR, 20*sizeof(char));//设备号
	W25QXX_Read((uint8_t*)&seekconfig.BOAUD1,BOAUD1_RATE_ADDR, sizeof(int));//485-1波特率
	W25QXX_Read((uint8_t*)&seekconfig.BOAUD2,BOAUD2_RATE_ADDR, sizeof(int));//485-2波特率
	
	//电池电压
	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_Value,100);//开启DMA传输，ADC开始转换
	osDelay(500);
	seekconfig.Volt=(((GET_Voltage_value(ADC_Value)*5.3)+0.73)*1.063918538+0.018723337);//电量信息传递给报文数据结构
	
	W25QXX_Read((uint8_t*)&seekconfig.COMMOD,COMMOD_ADDR, sizeof(int));//通讯模式
	W25QXX_Read((uint8_t*)&seekconfig.SENSORNUM,SENSORNUM_ADDR, sizeof(int));//传感器数量
	W25QXX_Read((uint8_t*)&seekconfig.AGPS,AGPS_ADDR, sizeof(int));//GPS功能是否开启
	W25QXX_Read((uint8_t*)&seekconfig.STATE,WORK_STATE_ADDR, sizeof(int));//工作模式
	W25QXX_Read((uint8_t*)seekconfig.card_nb,BC260_CMMI_ADDR,17);//BC260物联网卡卡号
	W25QXX_Read((uint8_t*)seekconfig.card_4g,EC20_CMMI_ADDR,17);//EC20物联网卡卡号
	//系统时间
	HAL_RTC_GetTime (&hrtc,&Time,RTC_FORMAT_BIN);
	HAL_RTC_GetDate (&hrtc,&Date,RTC_FORMAT_BIN);
	VERSION=_VERSION_;//系统版本

	
	sprintf(sendbuff,
	"AT+SEEK;ST=%d;RT=%d;VALUE=%.2f;IP=%s;PORT=%d;LON=%f;LAT=%f;MN=%s;BOAUD1=%d;BOAUD2=%d;V=%.2f;COMMOD=%d;SENSORNUM=%d;GPS=%d;STATE=%d;NB=%s;4G=%s;CLK=%d/%d/%d/%d/%d/%d;VERSION=%.3f;\r\n",
	seekconfig.STIME,seekconfig.RTIME,seekconfig.VALUE,seekconfig.IP,seekconfig.PORT,seekconfig.LONGITUDE,seekconfig.LATITUDE,seekconfig.MN,seekconfig.BOAUD1,seekconfig.BOAUD2,seekconfig.Volt,seekconfig.COMMOD,seekconfig.SENSORNUM,
	seekconfig.AGPS,seekconfig.STATE,seekconfig.card_nb,seekconfig.card_4g,(Date.Year-48),Date.Month ,Date.Date ,Time.Hours ,Time.Minutes ,Time.Seconds,VERSION);
    printf("%s",sendbuff);
	
	
}








