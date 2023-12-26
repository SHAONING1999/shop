#include "usart.h"
#include "stdio.h"
#include "main.h"
#ifndef  _EC20_H
#define  _EC20_H
#define EC20_RX_BUF      USART6_RX_BUF
#define EC20_RX_STA      USART6_RX_STA

#define   HOST_NAME     "183.230.40.39"        //服务器IP地址(mqtt固定)
#define   HOST_PORT     "6002"                 //ONENET  Mqtt 
#define   CLIENT_ID     "1051916078"            //设备ID
#define   USER_NAME     "575870"               //产品ID
#define   PASSWORD      "123456789123"      //鉴权信息，设置为芯片序列号
#define   TOPIC         "Pipe Network"                //发布主题


void Test_4G_STA_MQTT(void );                               //测试NB_MQTT连接情况
int Test_4G_STA_TCP(char* msg );                               //测试NB_TCP连接情况
uint8_t EC20_send_cmd(char *cmd,char *ack,uint16_t waittime);      //向BC260Y发送命
int  EC20_send_cmd_LOOP(char* cmd,char* ack,uint16_t waittime,int loops,int err,char* errbuff);
void send_to_EC20(char *pData);                       //给BC260Y发数据
uint8_t* EC20_check_cmd(char *str);                         //检查BC260Y应答字符串
uint8_t MqttOnenet_Savedata(uint8_t *t_payload,uint8_t temp,uint8_t humi);

void Pub_to_server_4G(uint8_t* buf,uint8_t* topic);                  //给指定主题发布信息      
int32_t MQTTMsgPublish2dp_4G(uint8_t* msg);               //MQTT上传数据到服务器端口
int32_t TCP_Sent_Date_4G(char* msg);					//TCP上传数据到服务器端口
int parseGPSData( const char* gpsData, float* longitude, float* latitude) ;
#define   sampling time         "Pipe Network"                //发布主题
#endif
