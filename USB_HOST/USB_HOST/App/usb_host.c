/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v1.0_Cube
  * @brief           : This file implements the USB Host
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostFS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */
 void MSC_Application(void)
 {
	 printf("U盘插入！\r\n");
	 
	 printf("U盘文件读取！\r\n");
	 
	 printf("U盘文件搬运\r\n");
	 
	 printf("新程序跳转\r\n");
 }
/* USER CODE END 0 */

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */
///* USB通讯数据处理 */
//void McUsbDataProcess(void)
//{
//  /* USB 主机过程：应在主循环中调用，运行主机协议栈 */
//  USBH_Process(&hUsbHostFS);
//  
//  switch(Appli_state)
//  {
//  case APPLICATION_START:
//    {
//      break;
//    }
//  case APPLICATION_READY:
//    {
//      MSC_Application();//U盘读写任务
//      Appli_state = APPLICATION_IDLE;
//      break;
//    }
//  case APPLICATION_DISCONNECT:
//    {
//      break;
//    }
//  case APPLICATION_IDLE:
//    {
//      break;
//    }
//  default:
//    {
//      break; 
//    }
//  }
//}


/* USER CODE END 1 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* USER CODE END USB_HOST_Init_PreTreatment */

  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostFS, USBH_UserProcess, HOST_FS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostFS, USBH_MSC_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostFS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */

  /* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * Background task
 */
void MX_USB_HOST_Process(void)
{
  /* USB Host Background task */
  USBH_Process(&hUsbHostFS);
}
/*

 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_1 */
  switch(id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
	printf("HOST_USER_SELECT_CONFIGURATION\r\n");
  break;
 
  case HOST_USER_DISCONNECTION:
  Appli_state = APPLICATION_DISCONNECT;
	printf("Appli_state = APPLICATION_DISCONNECT\r\n");
  break;
 
  case HOST_USER_CLASS_ACTIVE:
  Appli_state = APPLICATION_READY;
	printf("Appli_state = APPLICATION_READY\r\n");
  break;
 
  case HOST_USER_CONNECTION:
  Appli_state = APPLICATION_START;
	printf("Appli_state = APPLICATION_START\r\n");
  break;
 
  default:
  break;
  }
  /* USER CODE END CALL_BACK_1 */
}

/**
  * @}
  */

/**
  * @}
  */

