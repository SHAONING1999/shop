/**
  ******************************************************************************
  * @file    IAP/IAP_Main/Src/menu.c 
  * @author  MCD Application Team

  * @brief   This file provides the software which contains the main menu routine.
  *          The main menu gives the options of:
  *             - downloading a new binary file, 
  *             - uploading internal flash memory,
  *             - executing the binary file already loaded 
  *             - configuring the write protection of the Flash sectors where the 
  *               user loads his binary file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/** @addtogroup STM32F4xx_IAP_Main
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "flash_if.h"
#include "menu.h"
#include "usart.h"
#include "stdio.h"
#include "usb_host.h"
#include "fatfs.h"
 

#pragma diag_suppress 870 //屏蔽中文乱码警告

//应用程序设置
#define NVIC_VectTab_RAM             ((uint32_t)0x20000000)
#define NVIC_VectTab_FLASH           ((uint32_t)0x8000000)
#define BOOT_SIZE						        	0x40000
#define BUFFSIZE						        	1024
#define ApplicationAddress	(NVIC_VectTab_FLASH + BOOT_SIZE) 



//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//关闭所有中断(但是不包括fault和NMI中断)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//开启所有中断
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(unsigned int addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

pFunction JumpToApplication;
uint32_t JumpAddress;
typedef  void (*iapfun)(void);
iapfun jump2app; 


uint32_t FlashProtection = 0;


/* Private function prototypes -----------------------------------------------*/

//关闭所有中断
void iap_interface_close_all_interrupt(void)
{
	SysTick->CTRL=0;
	SysTick->VAL=0x00;
	HAL_NVIC_DisableIRQ(USART1_IRQn);
	HAL_NVIC_DisableIRQ(USART6_IRQn);
	HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

//	HAL_NVIC_DisableIRQ(USART2_IRQn);
//	__HAL_RCC_USART2_CLK_DISABLE();
	
//	__HAL_RCC_USART6_CLK_DISABLE();
//	HAL_NVIC_DisableIRQ(USART6_IRQn);
	
//	__HAL_RCC_UART5_CLK_DISABLE();
//	HAL_NVIC_DisableIRQ(UART5_IRQn);
	


//	__HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
//	__HAL_UART_CLEAR_IDLEFLAG(&huart1);//清除标志位
//	HAL_UART_DMAStop(&huart1); //  停止DMA传输，防止


//	HAL_NVIC_EnableIRQ(USART1_IRQn);
//	HAL_NVIC_EnableIRQ(USART2_IRQn);
//	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
//	HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
//	HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
//	HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
//	HAL_SPI_MspDeInit(&hspi1);
	
}

void iap_interface_load_app(uint32_t appxaddr)
{
	jump2app=(iapfun)*(uint32_t*)(appxaddr+4);		//拷贝APP程序的复位中断函数地址
	MSR_MSP(*(uint32_t*)appxaddr);					//初始化APP堆栈指针,对APP程序的堆栈进行重构,就是说重新分配RAM
	jump2app();									//执行APP的复位中断函数,最终便会跳转到APP的main函数
}

void eraseFlashSectors(void) {
    FLASH_EraseInitTypeDef eraseInitStruct;
    uint32_t sectorError = 0;

    // 初始化 Flash 擦除结构体
    eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    // 擦除扇区5到扇区6
    HAL_FLASH_Unlock();
	
    eraseInitStruct.Sector = FLASH_SECTOR_6;
    eraseInitStruct.NbSectors = 1;
    HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError);

    eraseInitStruct.Sector = FLASH_SECTOR_7;
    eraseInitStruct.NbSectors = 1;
    HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError);

//    HAL_FLASH_Lock();
}

/* Private functions ---------------------------------------------------------*/



/**


  * @brief  Display the Main Menu on HyperTerminal
  * @param  None
  * @retval None
  */
int Main_Menu(void)
{
//    FRESULT fres; 			// FatFs函数常用结果码
//    uint32_t byteswrite;	//写入字节数
	uint32_t bytesread;		//读取字节数
	uint8_t datebuff[BUFFSIZE]; //待写入缓存
	FATFS FatfsUDisk; 		// u盘逻辑驱动器的文件系统对象
	FIL myFile; 			// 文件对象
	unsigned long filesize;			//文件大小
	
	//挂载U盘
	 if( f_mount(&FatfsUDisk, (TCHAR const*)USBHPath, 0) != FR_OK)
    {
//        Error_Handler(); //FatFs Initialization Error
		printf("挂载U盘失败\r\n");
		return 0;
    }
    else
    {
		printf("挂载U盘成功\r\n");
	}
	//打开bin文件
	 if(f_open(&myFile, "APP.bin",  FA_READ) != FR_OK)
	{
//		Error_Handler(); //'STM32.TXT' file Open for write Error
		printf("未找到APP.bin文件\r\n");
		return 0;
	}
	else
	{
		printf("成功获取APP.bin文件\r\n");
	}

	
    /* Test if any sector of Flash memory where user application will be loaded is write protected */
//    FlashProtection = FLASH_If_GetWriteProtectionStatus();//获取FLASH锁状态
    
    /* 取消写保护 */
    if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_DISABLE) == HAL_OK)
    {
		printf("关闭flash写保护...\r\n");
		/* 启动byte loading选项 */
		printf("擦除flash...\r\n");
		// 擦除Flash0x8040000往后128K的内存
		eraseFlashSectors();
		printf("擦除flash完成\r\n");
	}
	else
	{
		printf("Error: 失能flash写保护失败...\r\n");
	}

//			fres = f_read(&myFile, datebuff, 1024, (void *)&bytesread);
//			for(int i=0;i<1023;i++)
//			printf("0x%02X ",datebuff[i]);
	
			//获取文件大小
			filesize=f_size(&myFile);
			printf("APP SIZE:%ld\r\n",filesize);
			uint32_t currentAddress = ApplicationAddress;//flash当前写入地址
			// 读取并写入新的固件到Flash
			//FR_OK: 这部分判断确保 f_read 函数执行成功	
			//bytesread > 0确保确实读取到了文件的内容	,读取到的字节数为零，可能表示已经到达文件末尾
			while (f_read(&myFile, datebuff, BUFFSIZE, &bytesread) == FR_OK && bytesread > 0)
			{
			//依次写入flash
				for (uint32_t i = 0; i < bytesread; i += 4) 
				{
				uint32_t data = *(uint32_t*)&datebuff[i];
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, currentAddress, data);
				currentAddress  += 4;
					
				}
				// 处理升级进度，例如打印百分比
				uint32_t progress = ((currentAddress-ApplicationAddress) * 100) / filesize;
				printf("升级进度: %d%%\r\n", progress);
			}
			
////		//获取文件大小
////	filesize=f_size(&myFile);
////	printf("升级文件大小为：%d\r\n",filesize);
//	currentAddress = ApplicationAddress;//flash当前写入地址
//	// 读取并写入新的固件到Flash
//	//FR_OK: 这部分判断确保 f_read 函数执行成功	
//	//bytesread > 0确保确实读取到了文件的内容	,读取到的字节数为零，可能表示已经到达文件末尾
//   while (f_read(&myFile, datebuff, BUFFSIZE, &bytesread) == FR_OK && bytesread > 0)
//	{
//		//依次写入flash
//        for (uint32_t i = 0; i < bytesread; i += 4) {
//            uint32_t data = *(uint32_t*)&datebuff[i];
//            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ApplicationAddress, data);
//            currentAddress  += 4;
//        }
//		
//		// 处理升级进度，例如打印百分比
//        uint32_t progress = ((currentAddress-ApplicationAddress) * 100) / filesize;
//        printf("升级进度: %d%%\n", progress);
//	
//	}


//	/* 开启写保护 */
//	if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_ENABLE) == HAL_OK)
//	{
//		printf("使能flash写保护...\r\n");
//		/* 启动byte loading选项 */
////		HAL_FLASH_OB_Launch();//写入选项字节
//	}
//	else
//	{
//		printf("Error: flash写保护使能失败...\r\n");
//	}

	//关闭文件
	f_close(&myFile);
	printf("关闭文件...\r\n");
	//卸载U盘
	f_mount(NULL, "", 0);
	printf("卸载U盘...\r\n");
	/* 程序跳转 */
	iap_interface_close_all_interrupt();//关闭所有中断
	/* 程序跳转 */
	printf("bootloader跳转APP\r\n");
	iap_interface_load_app(ApplicationAddress);
	return 0;
}

/**
  * @}
  */
