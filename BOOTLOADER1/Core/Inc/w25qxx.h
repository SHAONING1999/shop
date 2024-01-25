#ifndef __W25QXX_H
#define __W25QXX_H
#include "main.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//W25QXX驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/4/13
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//W25X系列/Q系列芯片列表	   
//W25Q80  ID  0XEF13
//W25Q16  ID  0XEF14
//W25Q32  ID  0XEF15
//W25Q64  ID  0XEF16	
//W25Q128 ID  0XEF17	
//W25Q256 ID  0XEF18
#define W25Q80 	0XEF13
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17
#define W25Q256 0XEF18

#define NM25Q80 	0X5213
#define NM25Q16 	0X5214
#define NM25Q32 	0X5215
#define NM25Q64 	0X5216
#define NM25Q128	0X5217
#define NM25Q256 	0X5218


extern uint16_t W25QXX_TYPE;					//定义W25QXX芯片型号		   

#define	W25QXX_CS 		PBout(14)  		//W25QXX的片选信号

////////////////////////////////////////////////////////////////////////////////// 
//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg1		0x05 
#define W25X_ReadStatusReg2		0x35 
#define W25X_ReadStatusReg3		0x15 
#define W25X_WriteStatusReg1    0x01 
#define W25X_WriteStatusReg2    0x31 
#define W25X_WriteStatusReg3    0x11 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 
#define W25X_Enable4ByteAddr    0xB7
#define W25X_Exit4ByteAddr      0xE9


//数据保存地址
#define FLASH_SIZE	16*1024*1024//flash容量16M
#define   W25QXX_BACKUP_ADDR        	 0    									  //备份分包大小（字节）
#define REISSUE_MESSAGE_ADDR	FLASH_SIZE-8388608//补发数据保存地址

#define SNE1DATE_ADDR		    FLASH_SIZE-4000// 传感器1配置信息保存地址
#define SNE2DATE_ADDR		    FLASH_SIZE-2500// 传感器2配置信息保存地址

#define BC260_CMMI_ADDR			FLASH_SIZE-800//设备NB物联网卡号保存地址
#define EC20_CMMI_ADDR			FLASH_SIZE-770//设备4G物联网卡号保存地址
#define REISSUE_PAG_ADDR		FLASH_SIZE-700//数据补发页数保存地址
#define MONITOR_ADDR			FLASH_SIZE-690//监控模式flag
#define IWDG_ADDR				FLASH_SIZE-680////看门狗标志位保存地址
#define COMMOD_ADDR				FLASH_SIZE-660////通讯模式标志位保存地址
#define MN_ADDR					FLASH_SIZE-650////设备MN号保存地址
#define WORK_STATE_ADDR			FLASH_SIZE-590//设备故障码保存地址
#define BOAUD1_RATE_ADDR		FLASH_SIZE-585//设备1波特率保存地址
#define BOAUD2_RATE_ADDR		FLASH_SIZE-580//设备2波特率保存地址
#define SENSORNUM_ADDR			FLASH_SIZE-570//传感器个数保存地址
#define LONGITUDE_ADDR			FLASH_SIZE-520//经度信息保存地址
#define LATITUDE_ADDR			FLASH_SIZE-510//纬度信息保存地址
#define AGPS_ADDR				FLASH_SIZE-500//GPS开关控制保存地址
#define IP_ADDR					FLASH_SIZE-100//ip地址的保存地址
#define APP_SIZE_ADDR		    FLASH_SIZE-70// APP程序大小保存地址
#define PORT_ADDR				FLASH_SIZE-50//端口号保存地址
#define PWR_Tvalue_ADDR			FLASH_SIZE-40//电池低电压报警阈值保存地址
#define Datewritepage_ADDR		FLASH_SIZE-20//写入数据页数数据的保存地址
#define SamplingTime_ADDR		FLASH_SIZE-10//采样周期的保存地址
#define Reportingtime_ADDR		FLASH_SIZE-6//上报周期的保存地址




void W25QXX_Init(void);
uint16_t  W25QXX_ReadID(void);  	    		//读取FLASH ID
uint8_t W25QXX_ReadSR(uint8_t regno);             //读取状态寄存器 
void W25QXX_4ByteAddr_Enable(void);     //使能4字节地址模式
void W25QXX_Write_SR(uint8_t regno,uint8_t sr);   //写状态寄存器
void W25QXX_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);//按页写入数据
void W25QXX_Write_Enable(void);  		//写使能 
void W25QXX_Write_Disable(void);		//写保护
void W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);   //读取flash
void W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);//写入flash
void W25QXX_Erase_Chip(void);    	  	//整片擦除
void W25QXX_Erase_Sector(uint32_t Dst_Addr);	//扇区擦除
void W25QXX_Wait_Busy(void);           	//等待空闲
void W25QXX_PowerDown(void);        	//进入掉电模式
void W25QXX_WAKEUP(void);				//唤醒
int W25Q128_Write_float(float f,int addr);//往flash存入浮点数
float W25Q128_Read_float(int addr);//从flash取出浮点数
int countDataInCharArray(char *arr) ;

//浮点数共用体
typedef union
{
	float f;
	char ch[4];
}DataU;

extern DataU DataTrans;
#endif
