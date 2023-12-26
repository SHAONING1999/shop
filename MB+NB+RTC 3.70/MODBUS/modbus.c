#include "modbus.h"
#include "usart.h"
#include "cmsis_os.h"
#include "nbiot.h"
#include "main.h"
#include "cJSON.h"
#include "cJSON_Process.h"
#include "mbcrc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
MODBUS modbus;//结构体变量
uint16_t Reg[] ={0x0001,
				0x0002,
				0x0003,
				0x0004,
				0x0005,
				0x0006,
				0x0007,
				0X0008,
           };//reg是提前定义好的寄存器和寄存器数据，要读取和改写的部分内容
//下面时作为主机时内容

//主机选择从机
//参数1从机，参数2起始地址，参数3寄存器个数
void Host_Read03_slave1(uint8_t slave,uint16_t StartAddr,uint16_t num)
{
	int j;
	uint16_t crc;//计算的CRC校验位
	modbus.slave_add=slave;
	modbus.Host_Txbuf[0]=slave;//这是要匹配的从机地址
	modbus.Host_Txbuf[1]=0x03;//功能码
	modbus.Host_Txbuf[2]=StartAddr/256;//起始地址高位
	modbus.Host_Txbuf[3]=StartAddr%256;//起始地址低位
	modbus.Host_Txbuf[4]=num/256;//寄存器个数高位
	modbus.Host_Txbuf[5]=num%256;//寄存器个数低位
	crc=usMBCRC16(&modbus.Host_Txbuf[0],6); //获取CRC校验位
	modbus.Host_Txbuf[6]=crc%256;//寄存器个数高位  // ***************************************根据usMBCRC16函数手动更改过
	modbus.Host_Txbuf[7]=crc/256;//寄存器个数低位
	

	//发送数据包装完毕
//	osDelay(100);
	for(j=0;j<8;j++)
	{
		Modbus1_Send_Byte(modbus.Host_Txbuf[j]);
	}
	osDelay(10);
	modbus.Host_send_flag=1;//表示发送数据完毕
}

void Host_Read03_slave2(uint8_t slave,uint16_t StartAddr,uint16_t num)
{
	int j;
	uint16_t crc;//计算的CRC校验位
	modbus.slave_add2=slave;
	modbus.Host_Txbuf2[0]=slave;//这是要匹配的从机地址
	modbus.Host_Txbuf2[1]=0x03;//功能码
	modbus.Host_Txbuf2[2]=StartAddr/256;//起始地址高位
	modbus.Host_Txbuf2[3]=StartAddr%256;//起始地址低位
	modbus.Host_Txbuf2[4]=num/256;//寄存器个数高位
	modbus.Host_Txbuf2[5]=num%256;//寄存器个数低位
	crc=usMBCRC16(&modbus.Host_Txbuf2[0],6); //获取CRC校验位
	modbus.Host_Txbuf2[6]=crc%256;//寄存器个数高位  // ***************************************根据usMBCRC16函数手动更改过
	modbus.Host_Txbuf2[7]=crc/256;//寄存器个数低位
	

	//发送数据包装完毕
//	osDelay(100);
	for(j=0;j<8;j++)
	{
		Modbus2_Send_Byte(modbus.Host_Txbuf2[j]);
	}
	osDelay(10);
}

//向一个寄存器中写数据的参数设置
void Host_write06_slave(uint8_t slave,uint8_t fun,uint16_t StartAddr,uint16_t num)
{
	uint16_t crc,j;//计算的CRC校验位
	modbus.slave_add=slave;//从机地址赋值一下，后期有用
	modbus.Host_Txbuf[0]=slave;//这是要匹配的从机地址
	modbus.Host_Txbuf[1]=fun;//功能码
	modbus.Host_Txbuf[2]=StartAddr/256;//起始地址高位
	modbus.Host_Txbuf[3]=StartAddr%256;//起始地址低位
	modbus.Host_Txbuf[4]=num/256;
	modbus.Host_Txbuf[5]=num%256;
	crc=usMBCRC16(&modbus.Host_Txbuf[0],6); //获取CRC校验位
	modbus.Host_Txbuf[6]=crc%256;//寄存器个数高位
	modbus.Host_Txbuf[7]=crc/256;//寄存器个数低位
	
		//发送数据包装完毕 
//	osDelay(100);
	for(j=0;j<8;j++)
	{
		Modbus1_Send_Byte(modbus.Host_Txbuf[j]);
	}
	osDelay(10);
	modbus.Host_send_flag=1;//表示发送数据完毕

}

//这是往多个寄存器器中写入数据
//功能码0x10指令即十进制16
void Modbus_Func16()
{
		uint16_t Regadd;//地址16位
		uint16_t Reglen;
		uint16_t i,crc,j;
		
		Regadd=modbus.rcbuf[2]*256+modbus.rcbuf[3];  //要修改内容的起始地址
		Reglen = modbus.rcbuf[4]*256+modbus.rcbuf[5];//读取的寄存器个数
		for(i=0;i<Reglen;i++)//往寄存器中写入数据
		{
			//接收数组的第七位开始是数据
			Reg[Regadd+i]=modbus.rcbuf[7+i*2]*256+modbus.rcbuf[8+i*2];//对寄存器一次写入数据
		}
		//写入数据完毕，接下来需要进行打包回复数据了
		
		//以下为回应主机内容
		//内容=接收数组的前6位+两位的校验位
		modbus.sendbuf[0]=modbus.rcbuf[0];//本设备地址
		modbus.sendbuf[1]=modbus.rcbuf[1];  //功能码 
		modbus.sendbuf[2]=modbus.rcbuf[2];//写入的地址
		modbus.sendbuf[3]=modbus.rcbuf[3];
		modbus.sendbuf[4]=modbus.rcbuf[4];
		modbus.sendbuf[5]=modbus.rcbuf[5];
		crc=usMBCRC16(modbus.sendbuf,6);//获取crc校验位
		modbus.sendbuf[6]=crc/256;  //crc校验位加入包中
		modbus.sendbuf[7]=crc%256;
		//数据发送包打包完毕
		
		for(j=0;j<8;j++)
		{
			Modbus1_Send_Byte(modbus.sendbuf[j]);
		}
	
}

// 这个函数用于接收MODBUS-RTU数据，并将其存储在data缓冲区中，len是接收到的数据字节数。

// 函数用于解析MODBUS-RTU读指令返回的寄存器数据
// data: 存储MODBUS-RTU返回数据的缓冲区
// len: MODBUS-RTU返回数据的字节数
// resultArray: 存储解析后数据的数组
// resultArraySize: 存储解析后数据数组的大小
int parseMODBUSData(uint8_t* data, int len, uint16_t* resultArray, int resultArraySize) {
    
//	    if (len < 5) {
//        // 数据长度不足，解析失败
//        return 0;
//    }
//    
//    // 检查返回数据的字节数是否与Byte Count指定的数量一致
    uint8_t byteCount = data[2]; // Byte Count位于数据的第三个字节
//    if (len != byteCount + 5) {
//        // 数据长度不匹配，解析失败
//		printf("数据长度不匹配，解析失败\r\n");
//        return 0;
//    }
//    
//    // 确保resultArray有足够的空间来存储数据
//    if (byteCount / 2 > resultArraySize) {
//        // 如果resultArraySize不够大，需要处理错误情况
//        // 在这里你可以返回错误代码或执行其他错误处理操作
//		printf("如果resultArraySize不够大，存储\r\n");
//        return 0;
//    }
    
    // 解析数据并存储到resultArray数组中
    for (int i = 0, j = 0; i < byteCount; i += 2, j++) {
        // 按MODBUS-RTU协议，数据为高字节在前，低字节在后
        uint16_t highByte = data[3 + i];
        uint16_t lowByte = data[4 + i];  
        // 将两个字节合并成一个uint16_t数据
        resultArray[j] = (highByte << 8) | lowByte;
    }
    
    // 检查返回数据的字节数是否与Byte Count指定的数量一致
	//清除缓存
    memset(resultArray,0,sizeof((uint16_t*)resultArray));
    return 1; // 解析成功
}

/**
  ******************************************************************************
  * @brief   字节交换函数
			将给定的字节数组中的相邻两个字节进行交换，从而改变字节的顺序
  * @param   *array     数据块指针
  * @param   numBytes   字节数
  * @return  None
  * @note    
  ******************************************************************************
  */
void BigLittleEndianSwap(unsigned char *array, size_t numBytes)
{
	    // 使用 for 循环进行字节交换
    for (size_t i = 0; i < numBytes; i += 2) {
        unsigned char temp = array[i];
        array[i] = array[i + 1];
        array[i + 1] = temp;
    }
}
/**
  ******************************************************************************
  * @brief   大小端交换函数
				将大端转换为小端，或者将小段转换为大端
  * @param   *data     数据块指针
  * @param   size   字节数
  * @return  None
  * @note    
  ******************************************************************************
  */
void EndianSwap(void *data, int size) {
    unsigned char *bytes = (unsigned char *)data;

    for (size_t i = 0; i < size / 2; i++) {
        unsigned char temp = bytes[i];
        bytes[i] = bytes[size - 1 - i];
        bytes[size - 1 - i] = temp;
    }
}
//函数：实现字符串的连接，并将连接后的字符串指针作为结果返回
char *prependString(const char *prefix, const char *original) {
    // 计算新字符串的长度
    int newStringLength = strlen(prefix) + strlen(original) + 1;

    // 分配内存来存储新字符串
    char *newString = (char *)malloc(newStringLength * sizeof(char));
    
    if (newString == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // 将前缀字符串拷贝到新字符串中
    strcpy(newString, prefix);

    // 将原始字符串拼接到新字符串后面
    strcat(newString, original);

    return newString;
}


