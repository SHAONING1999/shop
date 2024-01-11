#include "usart.h"
#include "stdio.h"
#include "main.h"
#ifndef _nbiot_H
#define  _nbiot_H
#define BC260Y_RX_BUF      USART2_RX_BUF
#define BC260Y_RX_STA      USART2_RX_STA

#define   HOST_NAME     "iot-as-mqtt.cn-shanghai.aliyuncs.com"        //服务器IP地址(mqtt固定)
#define   HOST_PORT     "1883"                						  //阿里云  Mqtt 
#define   USER_NAME     "a1SVuSaiVBK"               				  //产品ID
#define   CLIENT_ID     "MN80808080800003"          				  //设备ID
#define   PASSWORD      "57e4c89ce35385f33172703997d47dde"            //鉴权信息，设置为芯片序列号
#define   SIZE         512            									  //分包大小


 struct aliyun
{

	int streamId;//升级包ID
	double version;//固件版本
	int PacketSize;//包总大小
	int offsize;//下载偏移位置
	int PakeNum;//分包数量
	int lastPackSize; //最后一包字节数
	int WritePakeNum;//写入的分包数量
};
extern struct aliyun MQTTaliyun;
int Test_NB_STA(void );                               //测试NB情况
int NB_MQTT_CONNECT(void );								//MQTT网络连接
int Test_NB_STA_TCP(char* msg  );   						//测试NB_TCP连接情况
uint8_t BC260Y_send_cmd(char *cmd,char *ack,uint16_t waittime);      //向BC260Y发送命令
int  BC260_send_cmd_LOOP(char* cmd,char* ack,uint16_t waittime,int loops,int err,char* errbuff);
void mess_at_response(uint8_t mode);                          //将串口2接收的数据通过串口1打印出来
void send_to_BC260Y(char *pData);                       //给BC260Y发数据
uint8_t* BC260Y_check_cmd(char *str);                         //检查BC260Y应答字符串
uint8_t MqttOnenet_Savedata(uint8_t *t_payload,uint8_t temp,uint8_t humi);
//公共函数
char* Delete_Spac(char * buf);                          //去掉字符串中的空格,'/',':'

void Pub_to_server(char* buf,char* topic);                  //给指定主题发布信息      
uint8_t MQTTSubTopic(char* msg);              		 //订阅主题
uint8_t MQTTUnSubTopic(char* UNSubTopic);
void fileDownloadCallback(const char* jsonString,struct aliyun* aliyun1) ;
int otaCallback(const char* jsonString,struct aliyun* aliyun1);
int getMqttOtaDate( char* recbuff,char* productKey,char* deviceName,struct aliyun* aliyun1);
int receiveMqttMessage(const char* message,struct aliyun* aliyun1);
int downpack(int num ,char* buff,struct aliyun* aliyun1);
uint8_t BC260_OTA(char* usart_buff,struct aliyun* MQTTaliyun);
#endif
