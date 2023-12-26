#ifndef _adxl345_H
#define _adxl345_H

#include "main.h"


//�Ĵ�����ַӳ��
#define DEVICE_ID		0X00 //����ID�洢�Ĵ���,����ʵ��ID��0XE5
#define THRESH_TAP		0X1D //�û���ֵ
#define OFSX			0X1E //X��ʧ��
#define OFSY			0X1F //Y��ʧ��
#define OFSZ			0X20 //Z��ʧ��
#define DUR				0X21 //Tap�ĳ���ʱ��
#define Latent			0X22 //Tap���ӳ�ʱ��
#define Window  		0X23 //Tap��ʱ�䴰
#define THRESH_ACK		0X24 //Activity������
#define THRESH_INACT	0X25 //lnactivity������
#define TIME_INACT		0X26 //Inactivity��ʱ��
#define ACT_INACT_CTL	0X27 //Activity/Inactivityʹ�ܿ���
#define THRESH_FF		0X28 //Free-Fall������
#define TIME_FF			0X29 //Free-Fall��ʱ��
#define TAP_AXES		0X2A //Tap/Double Tapʹ�ܿ���  
#define ACT_TAP_STATUS  0X2B //Activity/Tap�ж���ָʾ 
#define BW_RATE			0X2C //�����ʺ͹���ģʽ���� 
#define POWER_CTL		0X2D //����ģʽ���� 
#define INT_ENABLE		0X2E //�ж�ʹ�ܿ���
#define INT_MAP			0X2F //�ж�ӳ�����
#define INT_SOURCE  	0X30 //�ж�Դָʾ
#define DATA_FORMAT		0X31 //���ݸ�ʽ����
#define DATA_X0			0X32 //X������
#define DATA_X1			0X33 //
#define DATA_Y0			0X34 //Y������
#define DATA_Y1			0X35 //
#define DATA_Z0			0X36 //Z������
#define DATA_Z1			0X37 //
#define FIFO_CTL		0X38 //FIFO����
#define FIFO_STATUS		0X39 //FIFO״̬


//0X0B TO OX1F Factory Reserved	 
//���ALT ADDRESS��(12��)�ӵ�,ADXL��ַΪ0X53(���������λ).
//�����V3.3,��ADXL���	�Ϊ0X1D(���������λ).
//��Ϊ�������V3.3,����תΪ��д��ַ��,Ϊ0X3B��0X3A(�����GND,��Ϊ0XA7��0XA6)  
#define ADXL_READ    0XA7
#define ADXL_WRITE   0XA6

uint8_t ADXL345_Init(void); //��ʼ��ADXL345
void ADXL345_RD_XYZ(short *x,short *y,short *z);//��ȡһ��ֵ
void ADXL345_RD_Avval(short *x,short *y,short *z);//��ȡƽ��ֵ
void ADXL345_AUTO_Adjust(char *xval,char *yval,char *zval);//�Զ�У׼
void ADXL345_Read_Average(short *x,short *y,short *z,uint8_t times);//������ȡtimes��,ȡƽ��
int ADXL345_Get_Angle(float x,float y,float z,uint8_t dir);//��ȡ�Ƕ�����

//ADXL345��
HAL_StatusTypeDef ADXL345_ReadByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//ADXL345д
HAL_StatusTypeDef ADXL345_WriteByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
#endif
