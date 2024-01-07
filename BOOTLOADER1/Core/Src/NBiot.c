#include "NBiot.h"
#include "usart.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"
#include "delay.h"
#include "w25qxx.h"
#include "rtc.h"
#include "main.h"

/******************    芯片型号：BC260Y    *********************/
//     波特率：9600
//     接串口:USART2
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//测试NB模块运行状态
int Test_NB_STA(void )
{
	HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_SET);//NB模块供电
	delay_ms(1000);
	char *send_buf=NULL;
	BC260Y_send_cmd("AT\r\n", "OK",30);
	BC260Y_send_cmd("AT\r\n", "OK",30);//返回ok
	BC260Y_send_cmd("AT+QICLOSE=0", "OK",100);          //关闭已有连接
	delay_ms(200);
	//BC260Y模块设置
	BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",100);  //关闭自动休眠
	delay_ms(200);
	BC260Y_send_cmd("ATE0\r\n", "OK",100);            //设置命令不回显
	delay_ms(200);
	BC260Y_send_cmd("ATI\r\n", "",100);             //查询设备型号
	delay_ms(200);
	BC260Y_send_cmd("AT+CSQ\r\n", "OK",100);          //查询信号强度
	BC260Y_send_cmd("AT+CMEE=1\r\n", "OK",100);       //启用结果码	
	delay_ms(200);
  //SIM卡插入检查	
	if(BC260Y_send_cmd("AT+CIMI\r\n", "460",300))    //检查是否有卡 
	{
		printf("BC260未插卡,即将重启\r\n");       
		return -1;
	}
	delay_ms(500);
	//检查网络注册
	if(BC260Y_send_cmd("AT+CGATT=1\r\n", "OK",100))//激活网络
		return -1;//网络激活失败

	
 return 0;
}

int NB_MQTT_CONNECT(void )
{
	//设置mqtt协议版本
	//设置保活时间
	//设置消息不回显
	//连接阿里云平台
	//查询连接状态
	//设备登录
	//登录mqtt
	
 return 0;
}
//NB通讯：往MQTT订阅主题
uint8_t MQTTMsgPublish2dp(uint8_t* msg)
{

	return 0;
}

//NB通讯：往"topic"主题发送数据"buf"
void Pub_to_server(char* buf,char* topic)
{	char *send_buf=NULL;
	if(BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",1000))  //关闭自动休眠
	 BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",1000); 
		
//	//NB模块先订阅该主题，方便调试
//	sprintf(send_buf,"AT+QMTSUB=0,1,\"%s\",2\r\n",topic);//订阅该主题
//	BC260Y_send_cmd(send_buf,"OK",5000);
	
	sprintf(send_buf,"AT+QMTPUB=0,0,0,0,\"%s\"\r\n",topic);//发布消息主题
	if(BC260Y_send_cmd(send_buf,">",100)!=0)  
	printf("BC260发送数据请求失败\r\n");
	else 
	{
		clear_BUF(rx2_buffer);
		send_to_BC260Y(buf);
		if(BC260Y_send_cmd((char*)0X1A,"+QMTPUB:",100)==0)             
		{
			printf("BC260 MQTT 数据发送成功！！\r\n");
		}
	}
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
			delay_ms(50);
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
	printf("USART2发送：%s\r\n",pData);  //串口1打印显示串口2发送的结果
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


//************************************************************************/




//失败返回1，成功返回0
int  BC260_send_cmd_LOOP(char* cmd,char* ack,uint16_t waittime,int loops,int err,char* errbuff)
{
	while(BC260Y_send_cmd(cmd, ack,waittime))   
	{
		loops--;
		delay_ms(1000);		
		if(loops<=0)
		{
		printf("%s\r\n",errbuff);
		return err;	
		}      
	}
	return 0;
}

