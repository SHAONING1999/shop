#ifndef __OTA_H__
#define __OTA_H__
#include "sys.h"  

  
//#define FLASH_APP1_ADDR		0x08010000  	//��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)
											//����0X08000000~0X0800FFFF�Ŀռ�ΪBootloaderʹ��(��64KB)	   
//void iap_load_app(u32 appxaddr);			//��ת��APP����ִ��
void ota_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//��ָ����ַ��ʼ,д��bin
#endif



