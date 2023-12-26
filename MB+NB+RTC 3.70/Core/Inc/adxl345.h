#ifndef _adxl345_H
#define _adxl345_H

#include "main.h"


//¼Ä´æÆ÷µØÖ·Ó³Éä
#define DEVICE_ID		0X00 //Æ÷¼şID´æ´¢¼Ä´æÆ÷,Æ÷¼şÊµ¼ÊID£º0XE5
#define THRESH_TAP		0X1D //ÇÃ»÷·§Öµ
#define OFSX			0X1E //XÖáÊ§µ÷
#define OFSY			0X1F //YÖáÊ§µ÷
#define OFSZ			0X20 //ZÖáÊ§µ÷
#define DUR				0X21 //TapµÄ³ÖĞøÊ±¼ä
#define Latent			0X22 //TapµÄÑÓ³ÙÊ±¼ä
#define Window  		0X23 //TapµÄÊ±¼ä´°
#define THRESH_ACK		0X24 //ActivityµÄÃÅÏŞ
#define THRESH_INACT	0X25 //lnactivityµÄÃÅÏŞ
#define TIME_INACT		0X26 //InactivityµÄÊ±¼ä
#define ACT_INACT_CTL	0X27 //Activity/InactivityÊ¹ÄÜ¿ØÖÆ
#define THRESH_FF		0X28 //Free-FallµÄÃÅÏŞ
#define TIME_FF			0X29 //Free-FallµÄÊ±¼ä
#define TAP_AXES		0X2A //Tap/Double TapÊ¹ÄÜ¿ØÖÆ  
#define ACT_TAP_STATUS  0X2B //Activity/TapÖĞ¶ÏÖáÖ¸Ê¾ 
#define BW_RATE			0X2C //²ÉÑùÂÊºÍ¹¦ºÄÄ£Ê½¿ØÖÆ 
#define POWER_CTL		0X2D //¹¤×÷Ä£Ê½¿ØÖÆ 
#define INT_ENABLE		0X2E //ÖĞ¶ÏÊ¹ÄÜ¿ØÖÆ
#define INT_MAP			0X2F //ÖĞ¶ÏÓ³Éä¿ØÖÆ
#define INT_SOURCE  	0X30 //ÖĞ¶ÏÔ´Ö¸Ê¾
#define DATA_FORMAT		0X31 //Êı¾İ¸ñÊ½¿ØÖÆ
#define DATA_X0			0X32 //XÖáÊı¾İ
#define DATA_X1			0X33 //
#define DATA_Y0			0X34 //YÖáÊı¾İ
#define DATA_Y1			0X35 //
#define DATA_Z0			0X36 //ZÖáÊı¾İ
#define DATA_Z1			0X37 //
#define FIFO_CTL		0X38 //FIFO¿ØÖÆ
#define FIFO_STATUS		0X39 //FIFO×´Ì¬


//0X0B TO OX1F Factory Reserved	 
//Èç¹ûALT ADDRESS½Å(12½Å)½ÓµØ,ADXLµØÖ·Îª0X53(²»°üº¬×îµÍÎ»).
//Èç¹û½ÓV3.3,ÔòADXLµØÖ	·Îª0X1D(²»°üº¬×îµÍÎ»).
//ÒòÎª¿ª·¢°å½ÓV3.3,ËùÒÔ×ªÎª¶ÁĞ´µØÖ·ºó,Îª0X3BºÍ0X3A(Èç¹û½ÓGND,ÔòÎª0XA7ºÍ0XA6)  
#define ADXL_READ    0XA7
#define ADXL_WRITE   0XA6

uint8_t ADXL345_Init(void); //³õÊ¼»¯ADXL345
void ADXL345_RD_XYZ(short *x,short *y,short *z);//¶ÁÈ¡Ò»´ÎÖµ
void ADXL345_RD_Avval(short *x,short *y,short *z);//¶ÁÈ¡Æ½¾ùÖµ
void ADXL345_AUTO_Adjust(char *xval,char *yval,char *zval);//×Ô¶¯Ğ£×¼
void ADXL345_Read_Average(short *x,short *y,short *z,uint8_t times);//Á¬Ğø¶ÁÈ¡times´Î,È¡Æ½¾ù
int ADXL345_Get_Angle(float x,float y,float z,uint8_t dir);//»ñÈ¡½Ç¶ÈÊı¾İ

//ADXL345¶Á
HAL_StatusTypeDef ADXL345_ReadByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
//ADXL345Ğ´
HAL_StatusTypeDef ADXL345_WriteByte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
#endif
