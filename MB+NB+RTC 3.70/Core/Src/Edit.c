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

//传感器1初始化
	extern int rsensornum;
	extern int rsensornum_err;
	extern int sensornum;
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













