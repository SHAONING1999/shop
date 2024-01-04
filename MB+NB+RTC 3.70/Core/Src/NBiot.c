#include "NBiot.h"
#include "usart.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "usart.h"
#include "delay.h"
#include "w25qxx.h"
#include "rtc.h"
#include "main.h"

/******************    芯片型号：BC260Y    *********************/
//     波特率：9600
//     接串口:USART2
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	int nb_reissue_page=0;
	char nb_reissue_buff[500]={0};
	extern int rwork_state;
//测试NB模块运行状态
void Test_NB_STA_MQTT(void )
{
	char buff[40];//ip和端口缓存
	char ipbuff[36];//ip和端口缓存
	char portbuff[4];//ip和端口缓存
	HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_SET);//NB模块供电
	osDelay(1000);
	char *send_buf=NULL;
	BC260Y_send_cmd("AT\r\n", "OK",300);
	BC260Y_send_cmd("AT\r\n", "OK",300);//返回ok
	BC260Y_send_cmd("AT+QICLOSE=0", "OK",1000);          //关闭已有连接
	osDelay(200);
	//BC260Y模块设置
	BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",1000);  //关闭自动休眠
	osDelay(200);
	BC260Y_send_cmd("ATE0\r\n", "OK",1000);            //设置命令不回显
	osDelay(200);
	BC260Y_send_cmd("ATI\r\n", "",1000);             //查询设备型号
	osDelay(200);
	BC260Y_send_cmd("AT+CSQ\r\n", "OK",1000);          //查询信号强度
	BC260Y_send_cmd("AT+CMEE=1\r\n", "OK",1000);       //启用结果码	
	osDelay(200);
  //SIM卡插入检查	
	if(BC260Y_send_cmd("AT+CIMI\r\n", "460",3000))    //检查是否有卡 
	{
		printf("BC260未插卡,即将重启\r\n");       
		while(1);
	}
	osDelay(500);
	//检查网络注册
	BC260Y_send_cmd("AT+CGATT=1\r\n", "OK",1000);//激活网络
//	if(BC260Y_send_cmd("AT+CGATT?\r\n", "+CGATT: 1",2000))//查询网络是否激活成功
//	{
//		printf("BC260网络激活失败,即将重启\r\n");       
//		while(1);	
//	}
	
//联网部分  适用平台onenet 
//配置MQTT	
	BC260Y_send_cmd("AT+QMTCFG=\"version\",0,1\r\n", "OK",1000); //设置MQTT协议版本为3.1.1
	BC260Y_send_cmd("AT+QMTCFG=\"keepalive\",0,120\r\n", "OK",1000); //设置MQTT保活时间100s
	BC260Y_send_cmd("AT+QMTCFG=\"echomode\",0,0\r\n", "OK",1000); //设置数据模式下不向UART回显
	//断开已有网络端口
	if(BC260Y_send_cmd("AT+QMTOPEN?\r\n", "+QMTOPEN:",2000)==0)   //检查是否已有连接
	{
	   BC260Y_send_cmd("AT+QMTCLOSE=0\r\n", "0,0",1000); //如有连接断开连接	
	}
//打开网络端口
	osDelay(100);
	send_buf=pvPortMalloc(96);
	char* p1;
	W25QXX_Read( (uint8_t*)buff,IP_ADDR,40);//获取采样时间
	p1=strtok(buff,";");//字符分割
	sprintf(ipbuff,p1);//IP填充
	p1=strtok(NULL,";");
	sprintf(portbuff,p1);//端口号填充
	sprintf((char*)send_buf,"AT+QMTOPEN=0,\"%s\",%s\r\n",ipbuff,portbuff);//进行TCP连接
	if(BC260Y_send_cmd(send_buf, "+QMTOPEN: 0,0",2000))  //如果MQTT客户端打开网络失败进入循环
	{
		printf("BC260网络端口打开失败,即将重启\r\n");  
		while(1);
	}
	vPortFree(send_buf);
	send_buf=NULL;
	//断开已有连接
	if(BC260Y_send_cmd("AT+QMTCONN?\r\n", "+QMTCONN:",2000)==0)  //检查是否已有连接
		 BC260Y_send_cmd("AT+QMTDISC=0\r\n", "0,0",1000); 
	//连接服务器
	osDelay(100);
	send_buf=pvPortMalloc(96); 
	sprintf((char*)send_buf,"AT+QMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n",CLIENT_ID,USER_NAME,PASSWORD);//设备ID 产品ID 鉴权信息
	if(BC260Y_send_cmd(send_buf, "+QMTCONN: 0,0,0",5000)) //MQTT客户端连接服务器
	{
		printf("BC260连接服务器失败，即将重启\r\n"); 
		while(1);		
	} 
	vPortFree(send_buf);
	send_buf=NULL;
	//开启自动休眠
	BC260Y_send_cmd("AT+QSCLK=1\r\n", "OK",1000);
	//网络连接成功
	clear_BUF(rx2_buffer);
}

//NB通讯：往MQTT主题发送数据
void Pub_to_server(char* buf,char* topic)
{	char *send_buf=NULL;
	send_buf=pvPortMalloc(200);  //动态分配内存
	if(BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",1000))  //关闭自动休眠
	 BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",1000); 
		
	//NB模块先订阅该主题，方便调试
	sprintf(send_buf,"AT+QMTSUB=0,1,\"%s\",2\r\n",topic);//订阅该主题
	BC260Y_send_cmd(send_buf,"OK",5000);
	
	sprintf(send_buf,"AT+QMTPUB=0,0,0,0,\"%s\"\r\n",topic);//发布消息主题
	if(BC260Y_send_cmd(send_buf,">",5000)!=0)  
	printf("BC260发送数据请求失败\r\n");
	else 
	{
		clear_BUF(rx2_buffer);
		send_to_BC260Y(buf);

		if(BC260Y_send_cmd((char*)0X1A,"+QMTPUB:",5000)==0)             
		{
			printf("BC260 MQTT 数据发送成功！！\r\n");
		}
	}
	vPortFree(send_buf);//释放动态内存
	send_buf=NULL;
		//开启自动休眠
	BC260Y_send_cmd("AT+QSCLK=1\r\n", "OK",1000);
	clear_BUF(rx2_buffer);
}


//发送AT命令到NB模块
//cmd——要发送的命令
//ack——期待的回答
//waittime——等待时间
//返回值：成功返回0，失败返回1
uint8_t BC260Y_send_cmd(char *cmd,char *ack,uint16_t waittime)
{
	uint8_t res=0; //函数返回值
	if((uint32_t)cmd<=0XFF)//如果上一条指令没有发送完
	{
		while((USART2->SR&0X40)==0);//等待上一次数据发送完成  
		USART2->DR=(uint32_t)cmd;
	}
	else send_to_BC260Y(cmd);    //发送命令
	if(ack&&waittime)		         //需要等待应答
	{
		while(--waittime)	         //等待倒计时
		{
			osDelay(100);
			if(BC260_rec_flag==1)//如果接收到了应答
			{
				if(BC260Y_check_cmd(ack))//接收到期待的应答结果
				{
					printf("串口2接收(期待的应答结果)：\r\n%s\r\n",rx2_buffer);
					clear_BUF(rx2_buffer);   //如果不是期待的应答结果，则清除接收数据
					res=0;
					break;//得到有效数据 ，则退出，并清除接收缓存，此时返回值为0
				}
				else 
				{
					printf("串口2接收(不是期待的应答结果)：\r\n%s\r\n",rx2_buffer);
					clear_BUF(rx2_buffer);   //如果不是期待的应答结果，则清除接收数据
					res=1;//不是期待的应答结果，返回1
					break;
				}
			} 
		}
		if(waittime==0)res=1; //如果时间到了还没有等到期待的应答结果，则函数返回1
	}
	return res;
}
//给BC260Y发数据
void send_to_BC260Y(char *pData)  
{
	uint16_t i=strlen((const char*)pData);       //i(此次发送数据的长度)
	HAL_UART_Transmit(&huart2,(uint8_t*)pData,i,1000);
	vPortEnterCritical();
	printf("USART2发送：%s\r\n",pData);  //串口1打印显示串口2发送的结果
	vPortExitCritical();
}


//BC260Y发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
uint8_t* BC260Y_check_cmd(char *str)
{
	char *strx=NULL;
	if(BC260_rec_flag==1)		//接收到一次数据了
	{ 
		strx=strstr((const char*)rx2_buffer,(const char*)str);
	} 
	return (uint8_t*)strx;
}


//去除字符串中的' ' '/' ':'
char *Delete_Spac(char * buf)
{
	char *p = buf;
	char Buf[257]="";
	char *s=Buf;
	while(*p!=0)
	{
		if((*p!=0x20)&&(*p!=0x2f)&&(*p!=0x3A))
		{	
			*s=*p;
			 s++;
		}
		p++;
	}
	*s=0;
	strcpy((char*)buf,(char*)Buf);
	return buf;
}


//访问ONENET需要提交JSON数据
uint8_t MqttOnenet_Savedata(uint8_t *t_payload,uint8_t temp,uint8_t humi)
{

		char json[]="{\"datastreams\":[{\"id\":\"temp\",\"datapoints\":[{\"value\":%d}]},{\"id\":\"humi\",\"datapoints\":[{\"value\":%d}]}]}";
    char t_json[200];
  //  int payload_len;
    unsigned short json_len;
	  sprintf(t_json, json, temp, humi);
  //  payload_len = 1 + 2 + strlen(t_json)/sizeof(char);
    json_len = strlen(t_json)/sizeof(char);
    //type
    t_payload[0] = '\x01';
    //length
    t_payload[1] = (json_len & 0xFF00) >> 8;
    t_payload[2] = json_len & 0xFF;
	//json
  	memcpy(t_payload+3, t_json, json_len);
    return json_len+3;
}


//** 函数名称: MQTTMsgPublish2dp						
//** 函数功能: 用户推送消息到'$dp'系统主题,ONENET使用
//** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
//** 出口参数: >=0:发送成功 <0:发送失败
//** 备    注: 
//************************************************************************/
int32_t MQTTMsgPublish2dp(uint8_t* msg)
{
    int32_t ret;
    uint16_t msg_len = 0;
    msg_len = strlen((char *)msg);
    uint8_t* q = (uint8_t *)pvPortMalloc(msg_len); 
    memcpy((uint8_t*)(&q[0]),(uint8_t*)msg,msg_len);
    //上传数据点
//		if(BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",1000))  //关闭自动休眠
//	  	  BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",1000);  
		//订阅NBOO1的主题
		BC260Y_send_cmd("AT+QMTPUB=0,0,0,0,\"NB001\"\r\n",">",1000);
		clear_BUF(rx2_buffer);
	//发送发布消息内容
		for(int i=0;i<msg_len;i++)
		{
			while((USART2->SR&0X40)==0){}//等待发送完成 
			USART2->DR = (uint8_t) q[i]; 
			printf("%s",&q[i]);
		}
		//发送0X1A后缀代表消息结束
		if(BC260Y_send_cmd((char*)0X1A,"+QMTPUB:",3000)==0)    
		{
			printf("BC260 SEND OK!!\r\n");
		}
		//开启自动休眠
		clear_BUF(rx2_buffer);
//	  BC260Y_send_cmd("AT+QSCLK=1\r\n", "OK",1000);
		clear_BUF(rx2_buffer);
		vPortFree(q);
    q = NULL;
    return ret;
}

//cJSON* cJSON_Data = NULL;
//	char *p =NULL;
//	cJSON_Data = cJSON_Data_Init();		
//	cJSON_Update(cJSON_Data,"temp",&modbus.temp );
//	cJSON_Update(cJSON_Data,"humi",&modbus.humi);
//	p = cJSON_Print(cJSON_Data);
//	MQTTMsgPublish2dp((uint8_t *)p);
//	vPortFree(p);
//	p = NULL;

//BC260以TCP/IP协议连接云平台

int Test_NB_STA_TCP(char* msg)
{	//int err;
	char buff[50];//ip和端口缓存
	char ipbuff[36];//ip和端口缓存
	char portbuff[4];//ip和端口缓存
	HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_SET);//NB模块供电
	osDelay(1000);
	char send_buf[100];
	BC260Y_send_cmd("AT\r\n", "OK",6);
	osDelay(30);
	BC260Y_send_cmd("AT\r\n", "OK",6);//返回ok
	osDelay(30);
	BC260Y_send_cmd("AT+QICLOSE=0\r\n", "OK",20);          //关闭已有连接
	osDelay(200);
	//BC260Y模块设置
	if(BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",20))  //关闭自动休眠
	 BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",20);  
	osDelay(200);
	BC260Y_send_cmd("ATE0\r\n", "OK",20);            //设置命令不回显
	osDelay(200);
	BC260Y_send_cmd("ATI\r\n", "",20);             //查询设备型号
	osDelay(200);
	BC260Y_send_cmd("AT+CSQ\r\n", "OK",20);          //查询信号强度
	osDelay(200);
	BC260Y_send_cmd("AT+CMEE=1\r\n", "OK",20);       //启用结果码	
	osDelay(200);
  //SIM卡插入检查	
	BC260Y_send_cmd("AT+CIMI\r\n", "460",30);//检查是否有卡 
	osDelay(100);
	send_to_BC260Y("AT+CIMI\r\n");
	osDelay(200);
	unsigned char* pm;
	pm = (unsigned char*)strstr(( const char*)rx2_buffer, "460");//在字符串中寻找+CCLK
	if(pm!=NULL)
	{
	char cmmi[20]={0};
	strcpy(cmmi,(char*)rx2_buffer);
	printf("NB物联网卡号获取成功\r\n");
	W25QXX_Write((uint8_t*)&cmmi,BC260_CMMI_ADDR,20);//把卡号信息写入flash
	}
	osDelay(300);
	//检查网络注册
	clear_BUF(rx2_buffer);
	BC260_send_cmd_LOOP("AT+CGATT=1\r\n","OK",70,3,1,"网络激活失败,即将重启");
	osDelay(300);
	BC260_send_cmd_LOOP("AT+CGATT?\r\n","+CGATT: 1",40,5,1,"网络激活结果查询为激活失败,即将重启");
	//RTC自动对时
	osDelay(1000);
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	clear_BUF(rx2_buffer);
	send_to_BC260Y("AT+CCLK?\r\n");
	osDelay(1000);
	int year,mouth,date,hour,min,second;
	unsigned char* p3;
	p3 = (unsigned char*)strstr(( const char*)rx2_buffer, "+CCLK");//在字符串中寻找+CCLK
	if(p3!=NULL)
	{						//+CCLK: "2000/01/01,00:00:10+08"
		sscanf((const char*)p3,"+CCLK: \"%d/%d/%d,%d:%d:%d+",&year,&mouth,&date,&hour,&min,&second);
		if(year!=2000&&date<=27)//月底涉及到日月年变动，容易出bug，取消对时，沿用系统RTC时间
		{
			
			sDate.Month = mouth;
			sDate.Year = year;
			hour=hour+8;
			sTime.Minutes = min;
			sTime.Seconds = second;
			if(hour>=24)//解决零点过后对时错误的bug
			{
				hour-=24;
				date++;
			}
			sTime.Hours =hour;
			sDate.Date = date;
			if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
			{
			Error_Handler();
			}
			if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
			{
			Error_Handler();
			}
			printf("%d年,%d月,%d日,%d时,%d分,%d秒\r\n",year,mouth,date,(hour),min,second);
			printf("RTC自动对时成功\r\n");
		}
		else
		{
			printf("RTC自动对时失败，保留时间\r\n");
		}
	}	
	clear_BUF(rx2_buffer);
//打开网络端口
	osDelay(1000);
	char* p2;
	W25QXX_Read( (uint8_t*)buff,IP_ADDR,40);//获取IP
	int port;
	W25QXX_Read((uint8_t*)&port,PORT_ADDR,sizeof(int));
	sprintf((char*)send_buf,"AT+QIOPEN=0,0,\"TCP\",\"%s\",%d\r\n",buff,port);//进行TCP连接
	BC260_send_cmd_LOOP(send_buf,"OK",10,5,1,"BC260:TCP网络端口打开失败,即将重启");
	osDelay(1000);
	if(BC260_send_cmd_LOOP("AT+QISTATE=1,0\r\n","0,2,0",100,5,1,"BC260:TCP网络端口查询连接失败,即将重启"))
	{
		BC260_send_cmd_LOOP("AT+QICLOSE=0\r\n","OK",100,5,1,"BC260:TCP网络端口断开失败,即将重启");
		printf("BC260:TCP连接失败,备份数据\r\n");
		W25QXX_Read( (uint8_t*)&nb_reissue_page,REISSUE_PAG_ADDR,sizeof(int));//读取之前缓存的页数
		strncpy(nb_reissue_buff, msg, sizeof(nb_reissue_buff) - 1);	//把数据copy到缓存数组中
		W25QXX_Write( (uint8_t*)nb_reissue_buff,REISSUE_MESSAGE_ADDR+nb_reissue_page*500,500);//进行数据备份
		nb_reissue_page++;
		W25QXX_Write( (uint8_t*)&nb_reissue_page,REISSUE_PAG_ADDR,sizeof(int));//更新缓存页数
		printf("BC260:备份数据成功\r\n");
		osDelay(1000);
		if(rwork_state==2)//直接待机，后面的查询不用进行了。防止页数错误
		{
		START_STANDBY();//进入待机
		}
		return 0;//socket建立失败
	}
	osDelay(1000);
	clear_BUF(rx2_buffer);
	return 1;//socket建立成功
	
}

//** 函数名称: TCP_Sent_Date()						
//** 函数功能: 用户推送消息到'$dp'系统主题,ONENET使用
//** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
//** 出口参数: 发送成功1 发送失败0
//************************************************************************/
int32_t TCP_Sent_Date_NB(char* msg)
{
    int32_t ret=0;
	char *p7;
	char sentbuff[20];
    uint16_t msg_len = 0;
    msg_len = strlen((char *)msg);//计算数据长度
    uint8_t* q = (uint8_t *)pvPortMalloc(msg_len); //给数据安排动态缓存
    memcpy((uint8_t*)(&q[0]),(uint8_t*)msg,msg_len);
	
    //上传数据点
		if(BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",10))  //关闭自动休眠
	  	  BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",10);  
		osDelay(500);
		BC260_send_cmd_LOOP("AT+QICFG=\"dataformat\",0,0\r\n","OK",20,5,1,"BC260:设置报文text格式失败,即将重启");
		osDelay(500);
		sprintf(sentbuff,"AT+QISEND=0,%d\r\n",msg_len);//AT设置发送数据的长度
		BC260_send_cmd_LOOP(sentbuff, ">",10,5,1,"BC260:TCP数据长度设置失败 \">\"");
		osDelay(500);
		if(BC260_send_cmd_LOOP(msg, "SEND OK",10,5,1,"BC260:TCP数据发送失败 "))
		{
//		printf("BC260:TCP数据发送失败,备份数据\r\n");
//		W25QXX_Read( (uint8_t*)&nb_reissue_page,REISSUE_PAG_ADDR,sizeof(int));//读取之前缓存的页数
//		strncpy(nb_reissue_buff, msg, sizeof(nb_reissue_buff) - 1);	//把数据copy到缓存数组中
//		W25QXX_Write( (uint8_t*)nb_reissue_buff,REISSUE_MESSAGE_ADDR+nb_reissue_page*500,500);//进行数据备份
//		nb_reissue_page++;
//		W25QXX_Write( (uint8_t*)&nb_reissue_page,REISSUE_PAG_ADDR,sizeof(int));//更新缓存页数
//		clear_BUF(rx2_buffer);
//		vPortFree(q);
//		q = NULL;
//		return 0;//直接返回，后面的查询不用进行了。防止页数错误
		}
		else
		{
		printf("BC260:TCP数据发送成功\r\n");
		ret=1;//返回1
		}
			
		int loop=0;
		p7=strstr((char *)rx2_buffer,"##");//通过服务器返回判断是否发送数据成功
		if(p7)
		{
		printf("BC260:收到服务器应答,TCP数据发送成功\r\n");
		clear_BUF(rx2_buffer);
		vPortFree(q);
		q = NULL;
		return 1;//返回1
		}
		while(p7==NULL)
		{
			p7=strstr((char *)rx2_buffer,"##");//通过服务器返回判断是否发送数据成功
			if(p7)
			{
			printf("BC260:收到服务器应答,TCP数据发送成功\r\n");
			clear_BUF(rx2_buffer);
			vPortFree(q);
			q = NULL;
			return 1;//返回1
			}
			osDelay(1000);
			if(loop++>=7)//等待10秒还没接收到应答
			{
				printf("BC260:未收到服务器应答,TCP数据发送失败,备份数据\r\n");
				W25QXX_Read( (uint8_t*)&nb_reissue_page,REISSUE_PAG_ADDR,sizeof(int));//读取之前缓存的页数
				strncpy(nb_reissue_buff, msg, sizeof(nb_reissue_buff) - 1);	//把数据copy到缓存数组中
				W25QXX_Write( (uint8_t*)nb_reissue_buff,REISSUE_MESSAGE_ADDR+nb_reissue_page*500,500);//进行数据备份
				nb_reissue_page++;
				W25QXX_Write( (uint8_t*)&nb_reissue_page,REISSUE_PAG_ADDR,sizeof(int));//更新缓存页数
				ret=0;//返回1
				clear_BUF(rx2_buffer);
				vPortFree(q);
				q = NULL;
				return ret;
			}
		}
		
		clear_BUF(rx2_buffer);
		vPortFree(q);
		q = NULL;
		return ret;
}

//失败返回1，成功返回0
int  BC260_send_cmd_LOOP(char* cmd,char* ack,uint16_t waittime,int loops,int err,char* errbuff)
{
	while(BC260Y_send_cmd(cmd, ack,waittime))   
	{
		loops--;
		osDelay(1000);		
		if(loops<=0)
		{
		printf("%s\r\n",errbuff);
		return err;	
		}      
	}
	return 0;
}

