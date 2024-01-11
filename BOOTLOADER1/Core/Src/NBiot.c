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
#include "cjson.h"
#include "ota.h"
#include "flash.h"
/******************    芯片型号：BC260Y    *********************/
//     波特率：9600
//     接串口:USART2
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//测试NB模块运行状态
// MQTT消息回调函数类型
uint8_t BC260_OTA(char* usart_buff,struct aliyun* aliyun1)
{
	//测试模块硬件
	if(Test_NB_STA())
	{
		printf("BC260硬件异常\r\n");
		return -1;
	}
	//MQTT连接
	if(NB_MQTT_CONNECT())
	{
		printf("MQTT连接失败硬件异常\r\n");
		return -1;
	}
	//获取更新信息
	if(getMqttOtaDate(usart_buff,USER_NAME,CLIENT_ID,aliyun1))
	{
		aliyun1->WritePakeNum=0;
		printf("检测到新固件，开始下载新固件\r\n");
		for(int num=1;num<=(aliyun1->PakeNum);num++)
		{
          //获取第num包，起始地址为256*(num-1)
			
			downpack(num,usart_buff,aliyun1);
		}
		if(aliyun1->WritePakeNum==aliyun1->PakeNum)
		{
		printf("新固件更新完毕，启动APP程序\r\n");
		iap_interface_close_all_interrupt();//关闭所有中断  
		iap_interface_load_app(ApplicationAddress);//APP程序跳转
		}
	}
	else//没有更新信息
	{
//		printf("无新固件，启动APP程序\r\n");
//		iap_interface_close_all_interrupt();//关闭所有中断  
//		iap_interface_load_app(ApplicationAddress);//APP程序跳转
	}
	
	
}

//返回：-1测试异常
//返回： 0测试通过
int Test_NB_STA(void )
{
	HAL_GPIO_WritePin(GPIOA,NB_PWR_EN_Pin, GPIO_PIN_SET);//NB模块供电
	delay_ms(3000);
	char *send_buf=NULL;
	BC260Y_send_cmd("AT\r\n", "OK",30);
	delay_ms(200);
	BC260Y_send_cmd("AT\r\n", "OK",30);//返回ok
	delay_ms(200);
	BC260Y_send_cmd("AT+IPR=115200\r\n", "OK",100); 
	delay_ms(200);
	BC260Y_send_cmd("AT+QICLOSE=0\r\n", "OK",100);          //关闭已有连接
	delay_ms(200);
	//BC260Y模块设置
	BC260Y_send_cmd("AT+QSCLK=0\r\n", "OK",100);  //关闭自动休眠
	delay_ms(200);
	BC260Y_send_cmd("ATE0\r\n", "OK",100);            //设置命令不回显
	delay_ms(200);
	BC260Y_send_cmd("ATI\r\n", "",100);             //查询设备型号
	delay_ms(200);
	BC260Y_send_cmd("AT+CSQ\r\n", "+CSQ:",100);          //查询信号强度
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
	if(BC260Y_send_cmd("AT+CGATT=1\r\n", "OK",200))//激活网络
		return -1;//网络激活失败

	
 return 0;
}

int NB_MQTT_CONNECT(void )
{
	char sendbuff[100];
	//设置mqtt协议版本
	BC260Y_send_cmd("AT+QMTCFG=\"version\",0,1\r\n", "+IP",100);
	
	//设置保活时间
	BC260Y_send_cmd("AT+QMTCFG=\"keepalive\",0,120\r\n", "OK",100);
	
	//设置消息不回显
	BC260Y_send_cmd("AT+QMTCFG=\"echomode\",0,0\r\n", "OK",100);
	
	//连接阿里云平台
	sprintf(sendbuff,"AT+QMTOPEN=0,\"%s\",%s\r\n",HOST_NAME,HOST_PORT);
	BC260Y_send_cmd(sendbuff, "+QMTOPEN: 0,0",100);
	
	//设备登录
	sprintf(sendbuff,"AT+QMTCFG=\"aliauth\",0,\"%s\",\"%s\",\"%s\"\r\n",USER_NAME,CLIENT_ID,PASSWORD);
	BC260Y_send_cmd(sendbuff, "OK",100);
	
	//查询连接状态
	BC260Y_send_cmd("AT+QMTCONN?\r\n", "+QMTOPEN: 0,1",100);
		
	
	//登录mqtt
	if(BC260Y_send_cmd("AT+QMTCONN=0,\"clientExample_1987\"\r\n", "+QMTCONN: 0,0,0",100))
		return -1;
	
	
	//订阅OTA主题
//	//设备请求OTA升级包信息
//	sprintf(sendbuff,"/sys/%s/%s/thing/file/download",USER_NAME,CLIENT_ID);
//	MQTTSubTopic(sendbuff);
	//设备请求OTA升级包信息，消息响应主题
	sprintf(sendbuff,"/sys/%s/%s/thing/ota/firmware/get_reply",USER_NAME,CLIENT_ID);
	MQTTSubTopic(sendbuff);
	//设备分包下载升级包响应主题
	sprintf(sendbuff,"/sys/%s/%s/thing/file/download_reply",USER_NAME,CLIENT_ID);
	MQTTSubTopic(sendbuff);
	//退订响应主题，
	sprintf(sendbuff,"/sys/%s/%s/thing/file/download",USER_NAME,CLIENT_ID);
	MQTTUnSubTopic(sendbuff);

 return 0;
}


//NB通讯：往MQTT订阅主题
uint8_t MQTTSubTopic(char* SubTopic)
{
	char topic_buff[80];
	sprintf(topic_buff,"AT+QMTSUB=0,1,\"%s\",0\r\n",SubTopic);
	if(BC260Y_send_cmd(topic_buff, "+QMTSUB",100))	
	return -1;
	
	return 0;
}

//NB通讯：往MQTT退订主题
uint8_t MQTTUnSubTopic(char* UNSubTopic)
{
	char topic_buff[80];
	sprintf(topic_buff,"AT+QMTUNS=0,1,\"%s\"\r\n",UNSubTopic);
	if(BC260Y_send_cmd(topic_buff, "+QMTUNS",100))	
	return -1;
	
	return 0;
}

//MQTT发布通讯：往"topic"主题发送数据"buf"
void Pub_to_server(char* buf,char* Pubtopic)
{	

	char send_buf[100];
	restart:	
//	//NB模块先订阅该主题，方便调试
//	sprintf(send_buf,"AT+QMTSUB=0,1,\"%s\",2\r\n",topic);//订阅该主题
//	BC260Y_send_cmd(send_buf,"OK",5000);
	
	sprintf(send_buf,"AT+QMTPUB=0,0,0,0,%s\r\n",Pubtopic);//发布消息主题
	BC260Y_send_cmd(send_buf,">",50);
	send_to_BC260Y(buf);
	if(BC260Y_send_cmd((char*)0X1A,"OK",1000)!=0)             
	{
		printf("BC260 MQTT 数据发送失败，重新发送！！\r\n");
		Usart2_Handle();
		goto restart;
	}
//   Usart2_Handle();

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
				Usart2_Handle();
				if(BC260Y_check_cmd(ack))//接收到期待的应答结果
				{
					printf("串口2接收(期待的应答结果)：\r\n%s",rx2_buffer);
					clear_BUF(rx2_buffer);   //如果不是期待的应答结果，则清除接收数据
//					res=0;
//					break;//得到有效数据 ，则退出，并清除接收缓存，此时返回值为0
					return 0;
				}
				else 
				{
					printf("串口2接收(不是期待的应答结果)：\r\n%s",rx2_buffer);
					clear_BUF(rx2_buffer);   //如果不是期待的应答结果，则清除接收数据
					res=1;
					continue;//继续等
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
	Usart2_Handle();
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


// MQTT主题对应的回调函数
//返回值0 无新固件 1有新固件
int otaCallback(const char* jsonString,struct aliyun* aliyun1) 
{
    // 在这里处理/sys/${productKey}/${deviceName}/thing/ota/firmware/get_reply主题的数据
    // 解析jsonString，执行相应的逻辑
	if(strstr(jsonString,"streamId")==NULL)//无更新包
	{
		 printf("无更新包\r\n");
		return 0;
	}
    cJSON* root = cJSON_Parse(jsonString);
    if (root != NULL) {
        cJSON* code = cJSON_GetObjectItem(root, "code");
        if (code != NULL && code->valueint == 200) {
            // 提取数据并处理
            cJSON* data = cJSON_GetObjectItem(root, "data");
            if (data != NULL) {
                cJSON* size = cJSON_GetObjectItem(data, "size");
                cJSON* streamId = cJSON_GetObjectItem(data, "streamId");
                cJSON* version = cJSON_GetObjectItem(data, "version");

                if (size != NULL && streamId != NULL && version != NULL) 
				{
					aliyun1->PacketSize=size->valueint;
                    printf("PACK Size: %d\n", aliyun1->PacketSize);
					aliyun1->streamId=streamId->valueint;
                    printf("PACK Stream ID: %d\n", aliyun1->streamId);
					aliyun1->version = atof(version->valuestring);
                    printf("PACK Version: %.4f\n", aliyun1->version);
					
                    // 在这里可以执行相应的逻辑，比如进行固件升级等操作
					cJSON_Delete(root);
					return 1;
                }
            }
        }
        cJSON_Delete(root);
    }
	return 1;
}

void fileDownloadCallback(const char* jsonString,struct aliyun* aliyun1) 
{
    // 在这里处理/sys/${productKey}/${deviceName}/thing/file/download主题的数据
    // 解析jsonString，执行相应的逻辑
    cJSON* root = cJSON_Parse(jsonString);
    if (root != NULL) {
        cJSON* params = cJSON_GetObjectItem(root, "params");
        if (params != NULL) {
            cJSON* fileBlock = cJSON_GetObjectItem(params, "fileBlock");
            if (fileBlock != NULL) {
                cJSON* size = cJSON_GetObjectItem(fileBlock, "size");
                cJSON* offset = cJSON_GetObjectItem(fileBlock, "offset");

                if (size != NULL && offset != NULL) {
                    printf("File Download Size: %d\n", size->valueint);
					aliyun1->PacketSize=size->valueint;
                    printf("File Download Offset: %d\n", offset->valueint);
					aliyun1->offsize=offset->valueint;
                    // 在这里可以执行相应的逻辑，比如处理文件下载等操作
                }
            }
        }
        cJSON_Delete(root);
    }
}

// 对接收的MQTT消息分离出主题Topic和内容Payload
//0 无新固件 1有新固件
int receiveMqttMessage(const char* message,struct aliyun* aliyun1)
{
    // 模拟MQTT消息
//    printf("Received MQTT Message: %s\n", message);

    // 从消息中提取主题
    char* topicStart = strstr(message, "\"") + 1;
    char* topicEnd = strstr(topicStart, "\"");
    char topic[100];
    strncpy(topic, topicStart, topicEnd - topicStart);
    topic[topicEnd - topicStart] = '\0';
    printf("topic: %s\n\n", topic);
    //从消息中提取内容
    char* payloadStart = strstr(topicEnd + 1, "\"") + 1;
    char* payloadEnd = strstr(payloadStart, "}\"")+2;
    char payload[500];
    strncpy(payload, payloadStart, payloadEnd - payloadStart);
    payload[payloadEnd - payloadStart] = '\0';
    printf("分包信息: %s\n\n", payload);

    // 处理MQTT消息
    if (strstr(topic, "/thing/file/download") != NULL) //分包信息
	{
		printf("获取分包信息\r\n");
        fileDownloadCallback(payload,aliyun1);
    }
    else if (strstr(topic, "/thing/ota/firmware/get_reply") != NULL) //数据包信息
	{
		printf("获取整包信息\r\n");
        if(otaCallback(payload,aliyun1)==0)
			return 0;
		return 1;
    }
    else //其他信息
	{
        printf("Unknown topic: %s\n\n", topic);
		return 0;
    }
}
//获取mqtt升级包信息
//返回值 0未接收到数据包 1接收到数据包
int getMqttOtaDate( char* recbuff,char* productKey,char* deviceName,struct aliyun* aliyun1)
{

	char topic_buff[100];
	char send_buff[100];
//	//订阅响应主题
//	sprintf(topic_buff,"/sys/%s/%s/thing/ota/firmware/get_reply",productKey,deviceName);
//	MQTTSubTopic(send_buff);
	//发布消息--获取平台固件信息
	sprintf(topic_buff,"/sys/%s/%s/thing/ota/firmware/get",productKey,deviceName);
	sprintf(send_buff,"{\"id\": \"123\",\"version\": \"1.0\",\"params\": {},\"method\":\"thing.ota.firmware.get\"}");
	Pub_to_server(send_buff,topic_buff);
	//对返回的消息进行解析
	int time=5;
	while(time--)
	{
		if(time==0)
			return 0;
		if(BC260_rec_flag==1&&strstr(recbuff,"streamId")!=NULL )
		{
			if(receiveMqttMessage(recbuff,aliyun1))//有新固件
			{
				aliyun1->lastPackSize=(aliyun1->PacketSize%SIZE);//保存最后一包剩余字节数
				if(aliyun1->PacketSize%SIZE!=0)
					aliyun1->PakeNum =(aliyun1->PacketSize/SIZE+1);//总包数
				else
					aliyun1->PakeNum =(aliyun1->PacketSize/256);
					printf("共 %d 包数据，最后一包 %d 字节\r\n",aliyun1->PakeNum,aliyun1->lastPackSize);
					return 1;
			}
		}
		else
		{
		Usart2_Handle();
		delay_ms(1000);
		}
	}
	return 0;
}


//下载升级包
//num第N包
//buff接收缓冲区
//返回0表示接收成功，-1表示失败
int downpack(int num ,char* buff,struct aliyun* aliyun1)
{

	char sendbuff[200];
	char topicbuff[100];
	//获取第NUM包
	if(num<aliyun1->PakeNum)//前NUM-1包
	{
	sprintf(sendbuff,
	"{ \"id\": \"2331\",\"version\": \"1.0\",\"params\": {\"fileInfo\":{\"streamId\":%d,\"fileId\":1},\"fileBlock\":{\"size\":%d,\"offset\":%d }}}",
	aliyun1->streamId,SIZE,(num-1)*SIZE);
	sprintf(topicbuff,"/sys/%s/%s/thing/file/download",USER_NAME,CLIENT_ID);
	printf("获取第 %d 包\r\n",num);
//	printf("%s %s\r\n",topicbuff,sendbuff);
	Pub_to_server(sendbuff,topicbuff);
	}
	else if(num==aliyun1->PakeNum)////如果是最后一包,特殊处理
	{
	sprintf(sendbuff,
	"{ \"id\": \"2331\",\"version\": \"1.0\",\"params\": {\"fileInfo\":{\"streamId\":%d,\"fileId\":1},\"fileBlock\":{\"size\":%d,\"offset\":%d }}}",
	aliyun1->streamId,aliyun1->lastPackSize,(num-1)*SIZE);
	sprintf(topicbuff,"/sys/%s/%s/thing/file/download",USER_NAME,CLIENT_ID);
	Pub_to_server(sendbuff,topicbuff);
	printf("获取最后一包，第 %d 包\r\n",num);
//	printf("%s %s\r\n",topicbuff,sendbuff);
	}
	
	//CRC校验
	int t=20;//每段数据等5秒
	while(t--)
	{
		if(t==0)
		{
		printf("等待超时\r\n");
		return -1;
		}
		unsigned char* p1;
		if((unsigned char*)strstr(( const char*)rx2_buffer, "download_reply")!=NULL)
	    { p1 = ((unsigned char*)strstr(( const char*)rx2_buffer, "download_reply"))+21;
			for(int i=0;i<800;i++)
			{printf("%c ",*(rx2_buffer+i));}
		}//定位到代码位置
		//
		if(strstr(p1, "success")!=NULL)//接收到了数据
		{
			if(num==(aliyun1->PakeNum+1))//如果是最后一包,特殊处理
			{
			ota_write_appbin(ApplicationAddress+(num-1)*aliyun1->lastPackSize,(uint8_t*)p1,aliyun1->lastPackSize );
			aliyun1->WritePakeNum++;	
			printf("写入最后一包,即将跳转APP程序\r\n");
			Usart2_Handle();
			return 1;
			}
			else//不是最后一包
			{
			 aliyun1->WritePakeNum++;
			 printf("写入第%d包\r\n",aliyun1->WritePakeNum);
			 ota_write_appbin(ApplicationAddress+(num-1)*SIZE,(uint8_t*)p1,SIZE);
			
			 Usart2_Handle();
			 return 1;
			}
		}
		delay_ms(500);	
    }
	return 0;
}


