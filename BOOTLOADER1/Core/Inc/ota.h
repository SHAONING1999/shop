#ifndef __OTA_H__
#define __OTA_H__
#include "sys.h"  

  
//#define FLASH_APP1_ADDR		0x08010000  	//第一个应用程序起始地址(存放在FLASH)
											//保留0X08000000~0X0800FFFF的空间为Bootloader使用(共64KB)	   
//void iap_load_app(u32 appxaddr);			//跳转到APP程序执行
void ota_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//在指定地址开始,写入bin
#endif



