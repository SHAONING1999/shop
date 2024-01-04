#include "EC20.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>
#include "cmsis_os.h"
#include "usart.h"
#include "delay.h"
#include "w25qxx.h"
#include "rtc.h"
#include "w25qxx.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
/******************    芯片型号：EC20   *********************/
//     波特率：115200
//     接串口:USART6
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
int reissue_page_4g=0;
char reissue_buff_4g[500]={0};
//测试4G模块运行状态
void Test_4G_STA_MQTT(void )
{
	char buff[40];//ip和端口缓存
	char ipbuff[36];//ip和端口缓存
	char portbuff[4];//ip和端口缓存
	
	HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_SET);//4G模块供电
	
	osDelay(1000);
	char *send_buf=NULL;
	EC20_send_cmd("AT\r\n", "OK",300);
	EC20_send_cmd("AT\r\n", "OK",300);//返回ok
	EC20_send_cmd("AT+QICLOSE=0", "OK",1000);          //关闭已有连接
	osDelay(200);
	//EC20模块设置
	EC20_send_cmd("AT+QSCLK=0\r\n", "OK",1000);  //关闭自动休眠
	osDelay(200);
	EC20_send_cmd("ATE0\r\n", "OK",1000);            //设置命令不回显
	osDelay(200);
	EC20_send_cmd("ATI\r\n", "",1000);             //查询设备型号
	osDelay(200);
	EC20_send_cmd("AT+CSQ\r\n", "OK",1000);          //查询信号强度	
	osDelay(200);
  //SIM卡插入检查	
	if(EC20_send_cmd("AT+CIMI\r\n", "460",3000))    //检查是否有卡 
	{		printf("EC20未插卡,即将重启\r\n");       
		while(1);
	}
	osDelay(500);
	//检查网络注册
	EC20_send_cmd("AT+QICSGP=1,1,\"CMNET\",\"\",\"\",1\r\n", "OK",1000);//激活场景
	if(EC20_send_cmd("AT+CGATT?\r\n", "+CGATT: 1",2000))//查询网络是否激活成功
	{
		printf("EC20网络激活失败,即将重启\r\n");       
		while(1);	
	}
	
//联网部分  适用平台onenet 
//配置MQTT	
	EC20_send_cmd("AT+QMTCFG=\"version\",0,1\r\n", "OK",1000); //设置MQTT协议版本为3.1.1
	EC20_send_cmd("AT+QMTCFG=\"keepalive\",0,120\r\n", "OK",1000); //设置MQTT保活时间100s
	EC20_send_cmd("AT+QMTCFG=\"echomode\",0,0\r\n", "OK",1000); //设置数据模式下不向UART回显
	//断开已有网络端口
	if(EC20_send_cmd("AT+QMTOPEN?\r\n", "+QMTOPEN:",2000)==0)   //检查是否已有连接
	{
	   EC20_send_cmd("AT+QMTCLOSE=0\r\n", "0,0",1000); //如有连接断开连接	
	}
//打开网络端口
	osDelay(100);
	
	send_buf=pvPortMalloc(96);
	 char* p1;
	W25QXX_Read( (uint8_t*)buff,IP_ADDR,39);//获取采样时间
	p1=strtok(buff,";");//字符分割
	sprintf(ipbuff,"%s",p1);//IP填充
	p1=strtok(NULL,";");
	sprintf(portbuff,"%s",p1);//端口号填充
	sprintf((char*)send_buf,"AT+QMTOPEN=0,\"%s\",%s\r\n",ipbuff,portbuff);//进行TCP连接
	if(EC20_send_cmd(send_buf, "+QMTOPEN: 0,0",2000))  //如果MQTT客户端打开网络失败进入循环
	{
		printf("EC20网络端口打开失败,即将重启\r\n");  
		while(1);
	}
	vPortFree(send_buf);
	send_buf=NULL;
	//断开已有连接
	if(EC20_send_cmd("AT+QMTCONN?\r\n", "+QMTCONN:",2000)==0)  //检查是否已有连接
		 EC20_send_cmd("AT+QMTDISC=0\r\n", "0,0",1000); 
	//连接服务器
	osDelay(100);
	send_buf=pvPortMalloc(96); 
	sprintf((char*)send_buf,"AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n",CLIENT_ID,USER_NAME,PASSWORD);//设备ID 产品ID 鉴权信息
	if(EC20_send_cmd(send_buf, "+QMTCONN: 0,0,0",5000)) //MQTT客户端连接服务器
	{
		printf("EC20连接服务器失败，即将重启\r\n"); 
		while(1);		
	} 
	vPortFree(send_buf);
	send_buf=NULL;
	//开启自动休眠
	EC20_send_cmd("AT+QSCLK=1\r\n", "OK",1000);
	//网络连接成功
	clear_BUF(rx6_buffer);
}

//4G通讯：往MQTT主题发送数据
void Pub_to_server_4G(uint8_t* buf,uint8_t* topic)
{	char *send_buf=NULL;
	send_buf=pvPortMalloc(200);  //动态分配内存
	if(EC20_send_cmd("AT+QSCLK=0\r\n", "OK",1000))  //关闭自动休眠
	 EC20_send_cmd("AT+QSCLK=0\r\n", "OK",1000); 
		
	//NB模块先订阅该主题，方便调试
	sprintf(send_buf,"AT+QMTSUB=0,1,\"%s\",2\r\n",topic);//订阅该主题
	EC20_send_cmd(send_buf,"OK",5000);
	
	sprintf(send_buf,"AT+QMTPUB=0,0,0,0,\"%s\"\r\n",topic);//发布消息主题
	if(EC20_send_cmd(send_buf,">",5000)!=0)  
	printf("EC20发送数据请求失败\r\n");
	else 
	{
		clear_BUF(rx6_buffer);
		send_to_EC20((char*)buf);
		if(EC20_send_cmd((char*)0X1A,"+QMTPUB:",5000)==0)             
		{
			printf("EC20数据发送成功！！\r\n");
		}
	}
	vPortFree(send_buf);//释放动态内存
	send_buf=NULL;
		//开启自动休眠
	EC20_send_cmd("AT+QSCLK=1\r\n", "OK",1000);
	clear_BUF(rx6_buffer);
}


//发送AT命令到4G模块
//cmd——要发送的命令
//ack——期待的回答
//waittime——等待时间
uint8_t EC20_send_cmd(char *cmd,char *ack,uint16_t waittime)
{
	uint8_t res=0; //函数返回值
	if((uint32_t)cmd<=0XFF)//如果上一条指令没有发送完
	{
		while((USART6->SR&0X40)==0);//等待上一次数据发送完成  
		USART6->DR=(uint32_t)cmd;
	}
	else send_to_EC20(cmd);    //发送命令
	if(ack&&waittime)		         //需要等待应答
	{
		while(--waittime)	         //等待倒计时
		{
			osDelay(20);
			if(EC20_rec_flag==1)//如果接收到了应答
			{
				if(EC20_check_cmd(ack))//接收到期待的应答结果
				{
					printf("串口6接收(期待的应答结果)");
					clear_BUF(rx6_buffer);   //如果是期待的应答结果，则清除接收数据
					return 0;//得到有效数据 ，则退出，并清除接收缓存，此时返回值为0
				}
				else 
				{
					printf("串口6接收(不是期待的应答结果)：\r\n%s\r\n",rx6_buffer);
					clear_BUF(rx6_buffer);   //如果不是期待的应答结果，则清除接收数据
					//不是期待的应答结果，返回1
					return 1;
				}  
			} 
		}
		if(waittime==0)res=1; //如果时间到了还没有等到期待的应答结果，则函数返回1
	}
	return res;
}
//给EC20发数据
void send_to_EC20(char *pData)  
{
	uint16_t i=strlen((const char*)pData);       //i(此次发送数据的长度)
	HAL_UART_Transmit(&huart6,(const unsigned char*)pData,i,1000);
	vPortEnterCritical();//打印临界保护
	printf("USART6发送：%s\r\n",pData);  //串口1打印显示串口6发送的结果
	vPortExitCritical();//退出临界保护
}


//EC20发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
uint8_t* EC20_check_cmd(char *str)
{
	char *strx=NULL;
	if(EC20_rec_flag==1)		//接收到一次数据了
	{ 
		strx=strstr((const char*)rx6_buffer,(const char*)str);
	} 
	return (uint8_t*)strx;
}

//** 函数名称: MQTTMsgPublish2dp						
//** 函数功能: 用户推送消息到'$dp'系统主题,ONENET使用
//** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
//** 出口参数: >=0:发送成功 <0:发送失败
//** 备    注: 
//************************************************************************/
int32_t MQTTMsgPublish2dp_4G(uint8_t* msg)
{
    int32_t ret;
    uint16_t msg_len = 0;
    msg_len = strlen((char *)msg);
    char* q = (char *)pvPortMalloc(msg_len); 
    memcpy((uint8_t*)(&q[0]),(uint8_t*)msg,msg_len);
    //上传数据点
//		if(EC20_send_cmd("AT+QSCLK=0\r\n", "OK",1000))  //关闭自动休眠
//	  	  EC20_send_cmd("AT+QSCLK=0\r\n", "OK",1000);  
		//订阅NBOO1的主题
		EC20_send_cmd("AT+QMTPUB=0,0,0,0,\"NB001\"\r\n",">",1000);
		clear_BUF(rx6_buffer);
	//发送发布消息内容
		for(int i=0;i<msg_len;i++)
		{
			while((USART6->SR&0X40)==0){}//等待发送完成 
			USART6->DR = (uint8_t) q[i]; 
			printf("%s",&q[i]);
		}
		//发送0X1A后缀代表消息结束
		if(EC20_send_cmd((char*)0X1A,"+QMTPUB:",3000)==0)    
		{
			printf("EC20 MQTT SEND OK!!\r\n");
		}
		//开启自动休眠
		clear_BUF(rx6_buffer);
//	  EC20_send_cmd("AT+QSCLK=1\r\n", "OK",1000);
		clear_BUF(rx6_buffer);
		vPortFree(q);
    q = NULL;
    return ret;
}



//EC20以TCP/IP协议连接云平台
int Test_4G_STA_TCP(char* msg )
{
	char buff[40];//ip和端口缓存
	char ipbuff[36];//ip和端口缓存
	char portbuff[4];//ip和端口缓存
	float longitude,  latitude;//经纬度
	int gps_fun=-1;
	W25QXX_Read((uint8_t*)&gps_fun,AGPS_ADDR, sizeof(int));//GPS是否开启确认
	HAL_GPIO_WritePin(PWR_EN_4G_GPIO_Port, PWR_EN_4G_Pin, GPIO_PIN_SET);//4G模块供电
	osDelay(10000);
	char *send_buf=NULL;
	EC20_send_cmd("AT\r\n", "OK",30);
	osDelay(200);
	EC20_send_cmd("AT\r\n", "OK",30);//返回ok
	osDelay(200);
	EC20_send_cmd("AT+QICLOSE=0\r\n", "OK",10);          //关闭已有连接
	osDelay(200);
	//EC20模块设置
	EC20_send_cmd("AT+QSCLK=0\r\n", "OK",10);  
	osDelay(200);
	EC20_send_cmd("ATE0\r\n", "OK",20);            //设置命令不回显
	osDelay(200);
	if(gps_fun==1)
	EC20_send_cmd_LOOP("AT+QGPS=1\r\n", "OK",10,3,1,"GPS定位开启失败");//开启gps定位
	osDelay(200);
	EC20_send_cmd("ATI\r\n", "",20);             //查询设备型号
	osDelay(200);
	EC20_send_cmd("AT+CSQ\r\n", "OK",20);          //查询信号强度
	osDelay(200);
  //SIM卡插入检查	
	EC20_send_cmd_LOOP("AT+CIMI\r\n", "460",30,3,1,"EC20未插卡,即将重启");
	osDelay(200);
	send_to_EC20("AT+CIMI\r\n");
	osDelay(200);
	unsigned char* pm;
	pm = (unsigned char*)strstr(( const char*)rx6_buffer, "460");//在字符串中寻找+CCLK
	if(pm!=NULL)
	{
	char cmmi[20]={0};
	strcpy(cmmi,(char*)rx6_buffer);
	printf("4G物联网卡号获取成功\r\n");
	W25QXX_Write((uint8_t*)&cmmi,EC20_CMMI_ADDR,20);//把卡号信息写入flash
	}
	clear_BUF(rx6_buffer);
	osDelay(500);
	//检查网络注册
	EC20_send_cmd("AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1\r\n", "OK",10);//配置场景 1，APN 配置为"CMNET"（中国移动）。
	//激活场景
	osDelay(500);
	EC20_send_cmd_LOOP("AT+QIDEACT=1\r\n", "OK",20,3,1,"EC20激活场景失败");
	osDelay(200);
	//查询场景
	EC20_send_cmd_LOOP("AT+QIACT?\r\n", "OK",20,3,1,"EC20激活场景查询失败");
	osDelay(200);
	EC20_send_cmd_LOOP("AT+CEREG?\r\n", "OK",20,3,1,"EC20激活场景查询失败");
	//RTC自动对时
	osDelay(1000);
	send_to_EC20("AT+QNTP=1,\"171.34.197.243\",123\r\n");//NTP服务器对时。
	osDelay(500);
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	clear_BUF(rx6_buffer);
	send_to_EC20("AT+CCLK?\r\n");
	osDelay(1000);
	int year,mouth,date,hour,min,second;
	unsigned char* p3;
	p3 = (unsigned char*)strstr(( const char*)rx6_buffer, "+CCLK");//在字符串中寻找+CCLK
	if(p3!=NULL)
	{						//+CCLK: "2000/01/01,00:00:10+08"
		sscanf((const char*)p3,"+CCLK: \"%d/%d/%d,%d:%d:%d+",&year,&mouth,&date,&hour,&min,&second);
		if((23<=year)&&(year<30)&&(date<=27))//月底涉及到日月年变动，容易出bug，取消对时，沿用系统RTC时间
		{	hour=hour+8;
			sDate.Year = (year+2000);
			sDate.Month = mouth;
			if(hour>=24)//解决零点过后对时错误的bug
			{
				hour=hour-24;
				date++;
			}
			sTime.Hours =hour;
			sDate.Date = date;
			sTime.Minutes = min;
			sTime.Seconds = second;
			if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
			{
			Error_Handler();
			}
			if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
			{
			Error_Handler();
			}
			printf("%d年,%d月,%d日,%d时,%d分,%d秒\r\n",year,mouth,date,hour,min,second);
			printf("RTC自动对时成功\r\n");
		}
		else
		{
			printf("RTC自动对时失败，保留时间\r\n");
		}
	}	
	

	clear_BUF(rx6_buffer);
//打开网络端口
	send_buf=pvPortMalloc(96);
	char* p2;
	W25QXX_Read( (uint8_t*)buff,IP_ADDR,40);//获取IP地址
	int port;
	W25QXX_Read((uint8_t*)&port,PORT_ADDR,sizeof(int));
	sprintf((char*)send_buf,"AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,1\r\n",buff,port);//进行TCP连接，0，1（设置为缓存模式）
	EC20_send_cmd_LOOP(send_buf, "OK",10,3,1,"EC20:TCP网络端口打开失败");
	osDelay(1000);
	//+QISTATE: 0,"TCP","171.34.196.54",26000,0,1,1,0,1,"uart1"
	if(EC20_send_cmd_LOOP("AT+QISTATE=1,0\r\n", "2,1,0",10,3,1,"EC20:检查到TCP网络端口打开失败"))//2：SOCKET建立成功，1：固定（无特别作用）0：缓存模式
		{
		printf("EC20:TCP  SOCKET建立失败,备份数据\r\n");
		EC20_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",10,3,1,"EC20:TCP网络端口断开失败,即将重启");
		W25QXX_Read( (uint8_t*)&reissue_page_4g,REISSUE_PAG_ADDR,sizeof(int));//读取之前缓存的页数
		strncpy(reissue_buff_4g, msg, sizeof(reissue_buff_4g) - 1);	//把数据copy到缓存数组中
		W25QXX_Write( (uint8_t*)reissue_buff_4g,REISSUE_MESSAGE_ADDR+reissue_page_4g*500,500);//进行数据备份
		reissue_page_4g++;
		W25QXX_Write( (uint8_t*)&reissue_page_4g,REISSUE_PAG_ADDR,sizeof(int));//更新缓存页数
		printf("EC20:备份数据成功\r\n");
		osDelay(1000);
			if(rwork_state==2)//直接待机，后面的查询不用进行了。防止页数错误
			{
				START_STANDBY();//进入待机
			}
			vPortFree(send_buf);//记得清理堆区
		return 0;//EC20 socket建立失败
		}
		vPortFree(send_buf);//记得清理堆区
	//自动获取经纬度数据
	if(gps_fun==1)
	{
	clear_BUF(rx6_buffer);
	osDelay (300);
	EC20_send_cmd_LOOP("AT+QGPS?\r\n", "OK",10,3,1,"GPS定位查询：失败");
	osDelay(15000);
	send_to_EC20("AT+QGPSLOC=2\r\n");//发送获取经纬度AT指令
	osDelay (300);
	parseGPSData((char*)rx6_buffer,&longitude,&latitude);
//	EC20_send_cmd_LOOP("AT+QGPSEND\r\n", "OK",10,5,1,"GPS关闭失败");
	}
	return 1;//EC20 socket建立成功

}

//** 函数名称: TCP_Sent_Date_4G()						
//** 函数功能: 用户推送消息到'$dp'系统主题,ONENET使用
//** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
//** 出口参数: 1:发送成功 0:发送失败
//** 备    注: 
//************************************************************************/
int32_t TCP_Sent_Date_4G(char* msg)
{
	
	char sentbuff[20];
	char *p7;
    uint16_t msg_len = 0;
    msg_len = strlen((char *)msg);//计算数据长度
    uint8_t* q = (uint8_t *)pvPortMalloc(msg_len); //给数据安排动态缓存
    memcpy((uint8_t*)(&q[0]),(uint8_t*)msg,msg_len);
    //上传数据点
		if(EC20_send_cmd("AT+QSCLK=0\r\n", "OK",10))  //关闭自动休眠
	  	EC20_send_cmd("AT+QSCLK=0\r\n", "OK",10);  
		clear_BUF(rx6_buffer);
		osDelay(200);
		sprintf(sentbuff,"AT+QISEND=0,%d\r\n",msg_len);//AT设置发送数据的长度
		EC20_send_cmd_LOOP(sentbuff, ">",20,3,1,"EC20:TCP数据发送失败 \">\"");
		osDelay(500);
		if(EC20_send_cmd_LOOP(msg, "",20,3,1,"EC20:TCP数据发送失败 \"SEND OK\""))
		{
//		printf("EC20:TCP数据发送失败,备份数据\r\n");
//		W25QXX_Read( (uint8_t*)&reissue_page_4g,REISSUE_PAG_ADDR,sizeof(int));//读取之前缓存的页数
//		strncpy(reissue_buff_4g, msg, sizeof(reissue_buff_4g) - 1);	//把数据copy到缓存数组中
//		W25QXX_Write( (uint8_t*)reissue_buff_4g,REISSUE_MESSAGE_ADDR+reissue_page_4g*500,500);//进行数据备份
//		reissue_page_4g++;
//		W25QXX_Write( (uint8_t*)&reissue_page_4g,REISSUE_PAG_ADDR,sizeof(int));//更新缓存页数
//		vPortFree(q);
//		q = NULL;
//		return 0;//直接返回，后面的查询不用进行了。防止页数错误
		}
//		else
//		{
//		printf("EC20:TCP数据发送成功\r\n");
////		ret=1;//返回1
//		}
		
		int loop=0;
		p7=strstr((char *)rx6_buffer,"##");//通过服务器返回判断是否发送数据成功
		if(p7)
		{
		printf("EC20:收到服务器应答,TCP数据发送成功\r\n");
		clear_BUF(rx6_buffer);
		vPortFree(q);
		q = NULL;
		return 1;//返回1
		}
		while(p7==NULL)
		{
			p7=strstr((char *)rx6_buffer,"##");//通过服务器返回判断是否发送数据成功
			if(p7)
			{
			printf("EC20:收到服务器应答,TCP数据发送成功\r\n");
			clear_BUF(rx6_buffer);
			vPortFree(q);
			q = NULL;
			return 1;//返回1
			}
			osDelay(1000);
			if(loop++>=7)//等待10秒还没接收到应答
			{
				printf("EC20:未收到服务器应答,TCP数据发送失败,备份数据\r\n");
				W25QXX_Read( (uint8_t*)&reissue_page_4g,REISSUE_PAG_ADDR,sizeof(int));//读取之前缓存的页数
				strncpy(reissue_buff_4g, msg, sizeof(reissue_buff_4g) - 1);	//把数据copy到缓存数组中
				W25QXX_Write( (uint8_t*)reissue_buff_4g,REISSUE_MESSAGE_ADDR+reissue_page_4g*500,500);//进行数据备份
				reissue_page_4g++;
				W25QXX_Write( (uint8_t*)&reissue_page_4g,REISSUE_PAG_ADDR,sizeof(int));//更新缓存页数
				
				clear_BUF(rx6_buffer);
				vPortFree(q);
				q = NULL;
				return 0;//返回0
			}
		}
		clear_BUF(rx6_buffer);
		vPortFree(q);
		q = NULL;
		return 1;
}


//函数作用：EC20自动获取经纬度数据
//入口参数：
//gpsData 待解析的字符串数组地址
//longitude存放经度数据的地址
//latitude存放纬度数据的地址
//返回值：
//1： 解析成功 0：解析失败 

int parseGPSData( const char *data, float *latitude, float *longitude) {
    char *token;
    char tempData[256];  // 临时存储原始数据的副本，以免修改原始数据
	token = strtok(tempData, " ");
	 if (token == NULL) {
        return -1;
    }
	token = strtok(NULL, ".");
	 if (token == NULL) {
        return -1;
    }
	
	token = strtok(NULL, ",");
    if (token == NULL) {
        return -1;
    }
    // 解析纬度字段
    token = strtok(NULL, ",");
    if (token == NULL) {
        return -1;
    }
    *latitude = atof(token);
	W25Q128_Write_float(*latitude,LATITUDE_ADDR);//把纬度信息写入flash
	printf("gps_latitude:%f\r\n",W25Q128_Read_float(LATITUDE_ADDR));

    // 解析经度字段
    token = strtok(NULL, ",");
    if (token == NULL) {
        return -1;
    }
    *longitude = atof(token);
	W25Q128_Write_float(*longitude,LONGITUDE_ADDR);//把经度信息写入flash
	printf("gps_lonngitude:%f\r\n",W25Q128_Read_float(LONGITUDE_ADDR));
	
	token = strtok(NULL, ",");
	 if (token == NULL) {
        return -1;
    }
	token = strtok(NULL, ",");
	 if (token == NULL) {
        return -1;
    }
	token = strtok(NULL, ",");
	 if (token == NULL) {
        return -1;
    }
	token = strtok(NULL, ",");
	 if (token == NULL) {
        return -1;
    }
	token = strtok(NULL, ",");
	 if (token == NULL) {
        return -1;
    }
	token = strtok(NULL, ",");
	 if (token == NULL) {
        return -1;
    }
	token = strtok(NULL, ",");
	 if (token == NULL) {
        return -1;
    }
    return 0;  // 解析成功
}


int  EC20_send_cmd_LOOP(char* cmd,char* ack,uint16_t waittime,int loops,int err,char* errbuff)
{
	while(EC20_send_cmd(cmd, ack,waittime))    //检查是否有卡 
	{
		loops--;
		osDelay(500);		
		if(loops<=0)
		{
			printf("%s\r\n",errbuff);
			return err;
		}      
	}
	return 0;
}


