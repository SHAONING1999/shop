#include "usart.h"
#include "stdio.h"
#include "main.h"
#ifndef _nbiot_H
#define  _nbiot_H
#define BC260Y_RX_BUF      USART2_RX_BUF
#define BC260Y_RX_STA      USART2_RX_STA

#define   HOST_NAME     "183.230.40.39"        //服务器IP地址(mqtt固定)
#define   HOST_PORT     "6002"                 //ONENET  Mqtt 
#define   CLIENT_ID     "1051916078"            //设备ID
#define   USER_NAME     "575870"               //产品ID
#define   PASSWORD      "123456789123"      //鉴权信息，设置为芯片序列号
#define   TOPIC         "Pipe Network"                //发布主题


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
uint8_t MQTTMsgPublish2dp(uint8_t* msg);               //MQTT上传数据到服务器端口
#endif
