#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "flash.h"
#include "ota.h" 


u32 iapbuf[512]; 	//2K�ֽڻ���  
//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
void ota_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	unsigned char* p1;
	for(int t=0;t<=500;t++)
	printf("%s",appbuf);
	p1 = (unsigned char*)strstr(( const char*)appbuf, "success\"}");//��λ������λ��
 {
	u32 t;
	u16 i=0;
	u32 temp;
	 p1=p1+8;
	u8 *dfu=p1;//���ڽ�������
	 for(int t=0;t<=256;t++)
	 printf("%s",(uint8_t*)p1++);
	for(t=0;t<appsize;t+=10)
	{			
		//�Ѵ��ڽ��յ���char����ת����u32������iapbuf����
		temp=(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu+=4;//ƫ��4���ֽ�
		iapbuf[i++]=temp;
		
//		if(i==appsize)//���������ɣ�ִ��д�����
//		{
//			STMFLASH_Write(appxaddr,iapbuf,appsize);
//			printf("%x\r\n",(uint8_t)appbuf);
//		}
		
	} 
 }
 memset(iapbuf,0,sizeof(iapbuf));
}

