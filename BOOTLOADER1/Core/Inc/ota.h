#ifndef __OTA_H__
#define __OTA_H__
#include "sys.h"  

  
	   
//void iap_load_app(u32 appxaddr);			//跳转到APP程序执行
void ota_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//在指定地址开始,写入bin
void iap_interface_load_app(uint32_t appxaddr);

unsigned short CRC16_IBM(unsigned char* data, unsigned int datalen);
void InvertUint16(unsigned short* dBuf, unsigned short* srcBuf);
int set_app2w25q128_backups(uint32_t appstart_addr,uint32_t backup_addr);
int set_app2stm32_backups(uint32_t appstart_addr,uint32_t backup_addr);
#endif



