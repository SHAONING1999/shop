#ifndef __OTA_H__
#define __OTA_H__
#include "sys.h"  

  
//#define FLASH_APP1_ADDR		0x08010000  	//��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)
											//����0X08000000~0X0800FFFF�Ŀռ�ΪBootloaderʹ��(��64KB)	   
//void iap_load_app(u32 appxaddr);			//��ת��APP����ִ��
void ota_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//��ָ����ַ��ʼ,д��bin
void iap_interface_load_app(uint32_t appxaddr);

unsigned short CRC16_IBM(unsigned char* data, unsigned int datalen);
void InvertUint16(unsigned short* dBuf, unsigned short* srcBuf);
#endif



