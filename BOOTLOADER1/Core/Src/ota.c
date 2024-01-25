#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "flash.h"
#include "ota.h" 
#include "w25qxx.h" 
#pragma diag_suppress 870 //屏蔽中文乱码警告
#define   BACKUP_SIZE        	         	 512    							//备份分包大小（字节）
#define   BACKUP_TOTALSIZE        	         64*1024							//APP程序最大大小64K
u32 iapbuf[512]; 	//2K字节缓存  
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void ota_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	unsigned char* p1;
	p1 = (unsigned char*)strstr(( const char*)appbuf, "success\"}");//定位到代码位置
 {
	u32 t;
	u16 i=0;
	u32 temp;
	 p1=p1+9;
	u8 *dfu=p1;//串口接收数据
	for(t=0;t<appsize;t+=4)
	{			
		//把串口接收到的char类型转换成u32缓存在iapbuf里面
		temp=(u32)dfu[3]<<24;   
		temp|=(u32)dfu[2]<<16;    
		temp|=(u32)dfu[1]<<8;
		temp|=(u32)dfu[0];	  
		dfu+=4;//偏移4个字节
		iapbuf[i++]=temp;
			
	} 
	if(i==(appsize/4))//如果接收完成，执行写入操作
		{
			printf("写入%d个字节到内部flash \r\n",appsize);
//			for(int num=1;num<=appsize;num++)
//			{
//			printf("0x%02X ",p1[num-1]);
//			}
//			printf("\r\n ");
			STMFLASH_Write(appxaddr,iapbuf,i);
			
		}
 memset(iapbuf,0,sizeof(iapbuf));
}
 }

typedef  void (*iapfun)(void);
iapfun jump2app; 

void iap_interface_load_app(uint32_t appxaddr)
{
	jump2app=(iapfun)*(uint32_t*)(appxaddr+4);		//拷贝APP程序的复位中断函数地址
	MSR_MSP(*(uint32_t*)appxaddr);					//初始化APP堆栈指针,对APP程序的堆栈进行重构,就是说重新分配RAM
	jump2app();									//执行APP的复位中断函数,最终便会跳转到APP的main函数
}

//高低字节颠倒函数
void InvertUint16(unsigned short* dBuf, unsigned short* srcBuf)
{
    int i;
    unsigned short tmp[4] = { 0 };

    for (i = 0; i < 16; i++)
    {
        if (srcBuf[0] & (1 << i))
            tmp[0] |= 1 << (15 - i);
    }	
    dBuf[0] = tmp[0];
}


//CRC16-IBM函数
unsigned short CRC16_IBM(unsigned char* data, unsigned int datalen)
{
    unsigned short wCRCin = 0x0000;
    unsigned short wCPoly = 0x8005;

    InvertUint16(&wCPoly, &wCPoly);
    while (datalen--)
    {
        wCRCin ^= *(data++);
        for (int i = 0; i < 8; i++)
        {
            if (wCRCin & 0x01)
                wCRCin = (wCRCin >> 1) ^ wCPoly;
            else
                wCRCin = wCRCin >> 1;
        }
    }
    return (wCRCin);
}

//从指定地址读取当前程序大小
////参数 ：
//appsizeaddr APP程序大小存放地址
	



//把当前程序备份到外部flash
//参数 ：
//start_addr 	APP程序起始地址
//backup_addr	外部falsh备份起始地址
//size 			备份程序大小
int set_app2w25q128_backups(uint32_t appstart_addr,uint32_t backup_addr)
{	
//	int totalsize;
//	W25QXX_Read((uint8_t*)&size, APP_SIZE_ADDR, sizeof(int));//获取目前固件大小，以读取备份
	printf(" 开始备份固件，大小为%d字节\r\n",BACKUP_TOTALSIZE);
	//计算写入次数
	uint32_t backupbuff32[128];
	uint8_t backupbuff8[512];
	int num=0;
	num=(BACKUP_TOTALSIZE/BACKUP_SIZE);
	printf(" 共%d包\r\n",num);
	for(int i=1;i<=num;i++)
	{
	//从内部flash读取程序数据
	printf(" \r\n备份第%d包\r\n",i);
	STMFLASH_Read(appstart_addr+(i-1)*BACKUP_SIZE,backupbuff32,BACKUP_SIZE/4);
	//把4字节转化为1字节
	memcpy(backupbuff8,backupbuff32,BACKUP_SIZE);
	//写入外部flash
	W25QXX_Write(backupbuff8,backup_addr+(i-1)*BACKUP_SIZE,BACKUP_SIZE);
	for(int t=0;t<BACKUP_SIZE;t++)
	{printf(" 0x%02X",*(backupbuff8+t));}
	}
	printf("\r\n固件备份成功\r\n");
	return 1;
}

//把外部flash的备份程序写到内部flash
//参数 ：
//start_addr 	APP程序起始地址
//backup_addr	外部falsh备份起始地址
int set_app2stm32_backups(uint32_t appstart_addr,uint32_t backup_addr)
{	
//	int totalsize;
//	W25QXX_Read((uint8_t*)&size, APP_SIZE_ADDR, sizeof(int));//获取目前固件大小，以读取备份
	printf(" 开始回溯固件，大小为%d字节\r\n",BACKUP_TOTALSIZE);
	uint32_t backupbuff32[128];
	uint8_t  backupbuff8[512];
	int num=0;
	num=(BACKUP_TOTALSIZE/BACKUP_SIZE);
	printf(" 共%d包\r\n",num);
	for(int i=1;i<=num;i++)
	{
	//从内部flash读取程序数据
	printf(" \r\n回溯写入第%d包\r\n",i);
	W25QXX_Read(backupbuff8,backup_addr+(i-1)*BACKUP_SIZE,BACKUP_SIZE);
	for(int t=0;t<BACKUP_SIZE;t++)
	{printf(" 0x%02X",*(backupbuff8+t));}
	//把4字节转化为1字节
	memcpy(backupbuff32,backupbuff8,BACKUP_SIZE);
	//写入内部flash
	STMFLASH_Write(appstart_addr+(i-1)*BACKUP_SIZE,backupbuff32,BACKUP_SIZE/4);
	}
	printf("\r\n固件回溯成功\r\n");
	return 1;
}