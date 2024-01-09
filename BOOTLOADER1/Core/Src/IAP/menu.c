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
#include "common.h"
#include "flash_if.h"
#include "menu.h"
#include "ymodem.h"
#include "ota.h"
#include "flash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction JumpToApplication;
uint32_t JumpAddress;
uint32_t FlashProtection = 0;
uint8_t aFileName[FILE_NAME_LENGTH];

/* Private function prototypes -----------------------------------------------*/
void SerialDownload(void);
void SerialUpload(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  通过串口下载文件
  * @param  None
  * @retval None
  */
void SerialDownload(void)
{
  uint8_t number[11] = {0};
  uint32_t size = 0;
  COM_StatusTypeDef result;

  Serial_PutString((uint8_t *)"等待文件发送 ... (输入 'a' 中止)\n\r");
  result = Ymodem_Receive( &size );
  if (result == COM_OK)
  {
	//打印文件信息
    Serial_PutString((uint8_t *)"\n\n\r 编程成功!\n\r--------------------------------\r\n 文件名: ");
    Serial_PutString(aFileName);
    Int2Str(number, size);
    Serial_PutString((uint8_t *)"\n\r 大小: ");
    Serial_PutString(number);
    Serial_PutString((uint8_t *)" Bytes\r\n");
    Serial_PutString((uint8_t *)"-------------------\n");
  }
  else if (result == COM_LIMIT)
  {
    Serial_PutString((uint8_t *)"\n\n\r文件大小高于允许的内存空间\n\r");
  }
  else if (result == COM_DATA)
  {
    Serial_PutString((uint8_t *)"\n\n\r验证失败\n\r");
  }
  else if (result == COM_ABORT)
  {
    Serial_PutString((uint8_t *)"\r\n\n由用户终止\n\r");
  }
  else
  {
    Serial_PutString((uint8_t *)"\n\r接收文件失败!\n\r");
  }
}

/**
  * @brief  通过串口上传文件
  * @param  None
  * @retval None
  */
void SerialUpload(void)
{
  uint8_t status = 0;

  Serial_PutString((uint8_t *)"\n\n\r选择接收文件\n\r");

  HAL_UART_Receive(&UartHandle, &status, 1, RX_TIMEOUT);
//  if ( status == CRC16)
//  {
    /* 通过ymodem协议传输flash文件 */
    status = Ymodem_Transmit((uint8_t*)APPLICATION_ADDRESS, (const uint8_t*)"Flash.bin", USER_FLASH_SIZE);

    if (status != 0)
    {
      Serial_PutString((uint8_t *)"\n\r传输文件时发生错误\n\r");
    }
    else
    {
      Serial_PutString((uint8_t *)"\n\r文件上传成功\n\r");
    }
//  }
}

/**
  * @brief  Display the Main Menu on HyperTerminal
  * @param  None
  * @retval None
  */
void Main_Menu(void)
{
  uint8_t key = 0;

  Serial_PutString((uint8_t *)"\r\n======================================================================");
  Serial_PutString((uint8_t *)"\r\n=              (C) COPYRIGHT 2016 STMicroelectronics                 =");
  Serial_PutString((uint8_t *)"\r\n=                                                                    =");
  Serial_PutString((uint8_t *)"\r\n=          STM32F4xx In-Application Programming Application          =");
  Serial_PutString((uint8_t *)"\r\n=                                                                    =");
  Serial_PutString((uint8_t *)"\r\n=                       By MCD Application Team                      =");
  Serial_PutString((uint8_t *)"\r\n======================================================================");
  Serial_PutString((uint8_t *)"\r\n\r\n");

 

    /* Test if any sector of Flash memory where user application will be loaded is write protected */
    FlashProtection = FLASH_If_GetWriteProtectionStatus();
    
    Serial_PutString((uint8_t *)"\r\n=================== Main Menu ============================\r\n\n");
    Serial_PutString((uint8_t *)"  下载文件到内部Flash----------------------------------- 1\r\n\n");
    Serial_PutString((uint8_t *)"  从内部Flash上传文件----------------------------------- 2\r\n\n");
    Serial_PutString((uint8_t *)"  执行加载的应用程序------------------------------------ 3\r\n\n");

    if(FlashProtection != FLASHIF_PROTECTION_NONE)
    {
      Serial_PutString((uint8_t *)"  关闭flash写保护--------------------------------------------- 4\r\n\n");
    }
    else
    {
      Serial_PutString((uint8_t *)"  打开flash写保护-------------------------------------------- 4\r\n\n");
    }
    Serial_PutString((uint8_t *)"==========================================================\r\n\n");
 while (1)
  {
    /* 清除串口的收发器 */
    __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
	
    /* 接收一个命令字节。 */
    HAL_UART_Receive(&UartHandle, &key, 1, RX_TIMEOUT);

    switch (key)
    {
    case '1' :
      /* Download user application in the Flash */
      SerialDownload();//串口下载更新flash。
      break;
    case '2' :
      /* 从Flash上传用户应用程序 */
      SerialUpload();
      break;
    case '3' :
      Serial_PutString((uint8_t *)"开始执行APP程序......\r\n\n");
      /* execute the new program */
	  iap_interface_close_all_interrupt();//关闭所有中断
      JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);//180行获取APP固件的复位函数地址。
      /* Jump to user application */
      printf("bootloader跳转APP\r\n");
	  iap_interface_load_app(ApplicationAddress);
      break;
    case '4' :
      if (FlashProtection != FLASHIF_PROTECTION_NONE)
      {
        /* 取消写保护 */
        if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_DISABLE) == HAL_OK)
        {
          Serial_PutString((uint8_t *)"关闭flash写保护...\r\n");
          Serial_PutString((uint8_t *)"系统即将重启...\r\n");
          /* 启动byte loading选项 */
          HAL_FLASH_OB_Launch();
          /* 解锁flash */
          HAL_FLASH_Unlock();
        }
        else
        {
          Serial_PutString((uint8_t *)"Error: 关闭flash写保护失败...\r\n");
        }
      }
      else
      {
        if (FLASH_If_WriteProtectionConfig(OB_WRPSTATE_ENABLE) == HAL_OK)
        {
          Serial_PutString((uint8_t *)"使能flash写保护...\r\n");
          Serial_PutString((uint8_t *)"系统即将重启...\r\n");
          /* 启动byte loading选项 */
          HAL_FLASH_OB_Launch();
        }
        else
        {
          Serial_PutString((uint8_t *)"Error: flash写保护使能失败...\r\n");
        }
      }
      break;
    default:
	Serial_PutString((uint8_t *)"Invalid Number ! ==> 数字应该是： 1, 2, 3 or 4\r");
	break;
    }
  }
}

/**
  * @}
  */
