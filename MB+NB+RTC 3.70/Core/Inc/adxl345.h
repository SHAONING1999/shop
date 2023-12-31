#ifndef _adxl345_H
#define _adxl345_H

#include "main.h"


//寄存器地址映射
#define DEVICE_ID		0X00 //器件ID存储寄存器,器件实际ID：0XE5
#define THRESH_TAP		0X1D //敲击阀值
#define OFSX			0X1E //X轴失调
#define OFSY			0X1F //Y轴失调
#define OFSZ			0X20 //Z轴失调
#define DUR				0X21 //Tap的持续时间
#define Latent			0X22 //Tap的延迟时间
#define Window  		0X23 //Tap的时间窗
#define THRESH_ACK		0X24 //Activity的门限
#define THRESH_INACT	0X25 //lnactivity的门限
#define TIME_INACT		0X26 //Inactivity的时间
#define ACT_INACT_CTL	0X27 //Activity/Inactivity使能控制
#define THRESH_FF		0X28 //Free-Fall的门限
#define TIME_FF			0X29 //Free-Fall的时间
#define TAP_AXES		0X2A //Tap/Double Tap使能控制  
#define ACT_TAP_STATUS  0X2B //Activity/Tap中断轴指示 
#define BW_RATE			0X2C //采样率和功耗模式控制 
#define POWER_CTL		0X2D //工作模式控制 
#define INT_ENABLE		0X2E //中断使能控制
#define INT_MAP			0X2F //中断映射控制
#define INT_SOURCE  	0X30 //中断源指示
#define DATA_FORMAT		0X31 //数据格式控制
#define DATA_X0			0X32 //X轴数据
#define DATA_X1			0X33 //
#define DATA_Y0			0X34 //Y轴数据
#define DATA_Y1			0X35 //
#define DATA_Z0			0X36 //Z轴数据
#define DATA_Z1			0X37 //
#define FIFO_CTL		0X38 //FIFO控制
#define FIFO_STATUS		0X39 //FIFO状态


//0X0B TO OX1F Factory Reserved	 
//如果ALT ADDRESS脚(12脚)接地,ADXL地址为0X53(不包含最低位).
//如果接V3.3,则ADXL地�	肺�0X1D(不包含最低位).
//因为开发板接V3.3,所以转为读写地址后,为0X3B和0X3A(如果接GND,则为0XA7和0XA6)  
#define ADXL_READ    0XA7
#define ADXL_WRITE   0XA6

uint8_t ADXL345_Init(void); //初始化ADXL345
void ADXL345_RD_XYZ(short *x,short *y,short *z);//读取一次值
void ADXL345_RD_Avval(short *x,short *y,short *z);//读取平均值
void ADXL345_AUTO_Adjust(char *xval,char *yval,char *zval);//自动校准
void ADXL345_Read_Average(short *x,short *y,short *z,uint8_t times);//连续读取times次,取平均
int ADXL345_Get_Angle(float x,float y,float z,uint8_t dir);//获取角度数据

//ADXL345读
HAL_StatusTypeDef ADXL345_ReadByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//ADXL345写
HAL_StatusTypeDef ADXL345_WriteByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
#endif
