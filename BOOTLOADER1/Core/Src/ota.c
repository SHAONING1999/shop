#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "flash.h"
#include "ota.h" 


u32 iapbuf[512]; 	//2K字节缓存  
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void ota_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	unsigned char* p1;
	for(int t=0;t<=500;t++)
	printf("%s",appbuf);
	p1 = (unsigned char*)strstr(( const char*)appbuf, "success\"}");//定位到代码位置
 {
	u32 t;
	u16 i=0;
	u32 temp;
	 p1=p1+8;
	u8 *dfu=p1;//串口接收数据
	 for(int t=0;t<=256;t++)
	 printf("%s",(uint8_t*)p1++);
	for(t=0;t<appsize;t+=10)
	{			
		//把串口接收到的char类型转换成u32缓存在iapbuf里面
		temp=(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu+=4;//偏移4个字节
		iapbuf[i++]=temp;
		
//		if(i==appsize)//如果接收完成，执行写入操作
//		{
//			STMFLASH_Write(appxaddr,iapbuf,appsize);
//			printf("%x\r\n",(uint8_t)appbuf);
//		}
		
	} 
 }
 memset(iapbuf,0,sizeof(iapbuf));
}

