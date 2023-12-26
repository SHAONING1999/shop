#ifndef _modbus_h_
#define _modbus_h_
#include "main.h"

typedef struct 
{
	//作为从机时使用
  uint8_t  myadd;        //本设备从机地址
	uint8_t  rcbuf[100];   //modbus接受缓冲区
	uint8_t  timout;       //modbus数据持续时间
	uint8_t  recount;      //modbus端口接收到的数据个数
	uint8_t  timrun;       //modbus定时器是否计时标志
	uint8_t  reflag;       //modbus一帧数据接受完成标志位
	uint8_t  sendbuf[100]; //modbus接发送缓冲区
	
	//作为主机添加部分
	uint8_t Host_Txbuf[8];	//modbus1发送数组	
	uint8_t slave_add;		//要匹配的从机设备地址（做主机实验时使用）
	uint8_t Host_send_flag;//主机设备发送数据完毕标志位
	int Host_Sendtime;//发送完一帧数据后时间计数
	uint8_t Host_time_flag;//发送时间到标志位，=1表示到发送数据时间了
	uint8_t Host_End;//接收数据后处理完毕
	
	uint8_t Host_Txbuf2[8];	//modbus2发送数组	
	uint8_t slave_add2;		//要匹配的从机设备地址（做主机实验时使用）
	uint8_t Host_send_flag2;//主机设备发送数据完毕标志位
	int Host_Sendtime2;//发送完一帧数据后时间计数
	uint8_t Host_time_flag2;//发送时间到标志位，=1表示到发送数据时间了
	uint8_t Host_End2;//接收数据后处理完毕
	int date_num;
	int temp;//温度数据
	int humi;//湿度数据
	int flow;//流量数据
	int level;//水位数据
	int velocity;//流速数据
	double epower;//电池电量
	float longitude;//经度
	float latitude;//维度
	
	uint8_t BLE_DEBUG_FLAG;//蓝牙连接标志位
	uint8_t FLAG_NB;//NB网络连接标志位
	uint8_t FLAG_4G;//4G网络连接标志位
	uint8_t FLAG_ALM;//设备报警标志位
	uint8_t PWR_FLAG;//电池低电压报警标志位

}MODBUS;

extern uint16_t Reg[];
extern MODBUS modbus;

void Modbus_Func3(void);//读寄存器数据
void Modbus_Func6(void);//往1个寄存器中写入数据
void Modbus_Func16(void);//往多个寄存器中写入数据

void Host_Read03_slave1(uint8_t slave,uint16_t StartAddr,uint16_t num);
void Host_Read03_slave2(uint8_t slave,uint16_t StartAddr,uint16_t num);
void Host_write06_slave(uint8_t slave,uint8_t fun,uint16_t StartAddr,uint16_t num);

int parseMODBUSData(uint8_t* data, int len, uint16_t* resultArray, int resultArraySize);

void EndianSwap(void *data, int size);//大小端转化
void BigLittleEndianSwap(unsigned char *array, size_t numBytes);//字节交换
char *prependString(const char *prefix, const char *original) ;
#endif

