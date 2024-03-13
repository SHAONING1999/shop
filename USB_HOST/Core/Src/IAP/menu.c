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
 

#pragma diag_suppress 870 //�����������뾯��

//Ӧ�ó�������
#define NVIC_VectTab_RAM             ((uint32_t)0x20000000)
#define NVIC_VectTab_FLASH           ((uint32_t)0x8000000)
#define BOOT_SIZE						        	0x40000
#define BUFFSIZE						        	1024
#define ApplicationAddress	(NVIC_VectTab_FLASH + BOOT_SIZE) 



//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//�ر������ж�(���ǲ�����fault��NMI�ж�)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//���������ж�
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//����ջ����ַ
//addr:ջ����ַ
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

//�ر������ж�
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
//	__HAL_UART_CLEAR_IDLEFLAG(&huart1);//�����־λ
//	HAL_UART_DMAStop(&huart1); //  ֹͣDMA���䣬��ֹ


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
	jump2app=(iapfun)*(uint32_t*)(appxaddr+4);		//����APP����ĸ�λ�жϺ�����ַ
	MSR_MSP(*(uint32_t*)appxaddr);					//��ʼ��APP��ջָ��,��APP����Ķ�ջ�����ع�,����˵���·���RAM
	jump2app();									//ִ��APP�ĸ�λ�жϺ���,���ձ����ת��APP��main����
}

void eraseFlashSectors(void) {
    FLASH_EraseInitTypeDef eraseInitStruct;
    uint32_t sectorError = 0;

    // ��ʼ�� Flash �����ṹ��
    eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    // ��������5������6
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
//    FRESULT fres; 			// FatFs�������ý����
//    uint32_t byteswrite;	//д���ֽ���
	uint32_t bytesread;		//��ȡ�ֽ���
	uint8_t datebuff[BUFFSIZE]; //��д�뻺��
	FATFS FatfsUDisk; 		// u���߼����������ļ�ϵͳ����
	FIL myFile; 			// �ļ�����
	unsigned long filesize;			//�ļ���С
	
	//����U��
	 if( f_mount(&FatfsUDisk, (TCHAR const*)USBHPath, 0) != FR_OK)
    {
//        Error_Handler(); //FatFs Initialization Error
		printf("����U��ʧ��\r\n");
		return 0;
    }
    else
    {
		printf("����U�̳ɹ�\r\n");
	}
	//��bin�ļ�
	 if(f_open(&myFile, "APP.bin",  FA_READ) != FR_OK)
	{
//		Error_Handler(); //'STM32.TXT' file Open for write Error
		printf("δ�ҵ�APP.bin�ļ�\r\n");
		return 0;
	}
	else
	{
		printf("�ɹ���ȡAPP.bin�ļ�\r\n");
	}

	
    /* Test if any sector of Flash memory where user application will be loaded is write protected */
//    FlashProtection = FLASH_If_GetWriteProtectionStatus();//��ȡFLASH��״̬
    
    /* ȡ��д���� */
    if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_DISABLE) == HAL_OK)
    {
		printf("�ر�flashд����...\r\n");
		/* ����byte loadingѡ�� */
		printf("����flash...\r\n");
		// ����Flash0x8040000����128K���ڴ�
		eraseFlashSectors();
		printf("����flash���\r\n");
	}
	else
	{
		printf("Error: ʧ��flashд����ʧ��...\r\n");
	}

//			fres = f_read(&myFile, datebuff, 1024, (void *)&bytesread);
//			for(int i=0;i<1023;i++)
//			printf("0x%02X ",datebuff[i]);
	
			//��ȡ�ļ���С
			filesize=f_size(&myFile);
			printf("APP SIZE:%ld\r\n",filesize);
			uint32_t currentAddress = ApplicationAddress;//flash��ǰд���ַ
			// ��ȡ��д���µĹ̼���Flash
			//FR_OK: �ⲿ���ж�ȷ�� f_read ����ִ�гɹ�	
			//bytesread > 0ȷ��ȷʵ��ȡ�����ļ�������	,��ȡ�����ֽ���Ϊ�㣬���ܱ�ʾ�Ѿ������ļ�ĩβ
			while (f_read(&myFile, datebuff, BUFFSIZE, &bytesread) == FR_OK && bytesread > 0)
			{
			//����д��flash
				for (uint32_t i = 0; i < bytesread; i += 4) 
				{
				uint32_t data = *(uint32_t*)&datebuff[i];
				HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, currentAddress, data);
				currentAddress  += 4;
					
				}
				// �����������ȣ������ӡ�ٷֱ�
				uint32_t progress = ((currentAddress-ApplicationAddress) * 100) / filesize;
				printf("��������: %d%%\r\n", progress);
			}
			
////		//��ȡ�ļ���С
////	filesize=f_size(&myFile);
////	printf("�����ļ���СΪ��%d\r\n",filesize);
//	currentAddress = ApplicationAddress;//flash��ǰд���ַ
//	// ��ȡ��д���µĹ̼���Flash
//	//FR_OK: �ⲿ���ж�ȷ�� f_read ����ִ�гɹ�	
//	//bytesread > 0ȷ��ȷʵ��ȡ�����ļ�������	,��ȡ�����ֽ���Ϊ�㣬���ܱ�ʾ�Ѿ������ļ�ĩβ
//   while (f_read(&myFile, datebuff, BUFFSIZE, &bytesread) == FR_OK && bytesread > 0)
//	{
//		//����д��flash
//        for (uint32_t i = 0; i < bytesread; i += 4) {
//            uint32_t data = *(uint32_t*)&datebuff[i];
//            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, ApplicationAddress, data);
//            currentAddress  += 4;
//        }
//		
//		// �����������ȣ������ӡ�ٷֱ�
//        uint32_t progress = ((currentAddress-ApplicationAddress) * 100) / filesize;
//        printf("��������: %d%%\n", progress);
//	
//	}


//	/* ����д���� */
//	if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_ENABLE) == HAL_OK)
//	{
//		printf("ʹ��flashд����...\r\n");
//		/* ����byte loadingѡ�� */
////		HAL_FLASH_OB_Launch();//д��ѡ���ֽ�
//	}
//	else
//	{
//		printf("Error: flashд����ʹ��ʧ��...\r\n");
//	}

	//�ر��ļ�
	f_close(&myFile);
	printf("�ر��ļ�...\r\n");
	//ж��U��
	f_mount(NULL, "", 0);
	printf("ж��U��...\r\n");
	/* ������ת */
	iap_interface_close_all_interrupt();//�ر������ж�
	/* ������ת */
	printf("bootloader��תAPP\r\n");
	iap_interface_load_app(ApplicationAddress);
	return 0;
}

/**
  * @}
  */
