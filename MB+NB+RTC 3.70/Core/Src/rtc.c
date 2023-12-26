/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include "stdio.h"
#include "usart.h"
#include "w25qxx.h"
HAL_StatusTypeDef RTC_Set_Time(uint16_t hour,uint16_t min,uint16_t sec,uint16_t ampm)
{
	RTC_TimeTypeDef GET_Time;
	
	GET_Time.Hours=hour;
	GET_Time.Minutes=min;
	GET_Time.Seconds=sec;
	GET_Time.TimeFormat=ampm;
	GET_Time.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
    GET_Time.StoreOperation=RTC_STOREOPERATION_RESET;
	return HAL_RTC_SetTime(&hrtc,&GET_Time,RTC_FORMAT_BCD);	
}

//RTC日期设置
//year,month,date:年(0~99),月(1~12),日(0~31)
//week:星期(1~7,0,非法!)
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
HAL_StatusTypeDef RTC_Set_Date(uint16_t year,uint16_t month,uint16_t date,uint16_t week)
{
	RTC_DateTypeDef GET_Date;
    
	GET_Date.Date=date;
	GET_Date.Month=month;
	GET_Date.WeekDay=week;
	GET_Date.Year=year;
	return HAL_RTC_SetDate(&hrtc,&GET_Date,RTC_FORMAT_BCD);
}
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
	__HAL_RCC_PWR_CLK_ENABLE();//使能电源时钟PWR
	HAL_PWR_EnableBkUpAccess();//取消备份区域写保护
	HAL_PWR_DisableBkUpAccess();
  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  //将RTC时间数据保存在备份区
	if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR0)!=0X5051)
	//是否第一次配置
	{ 
	 RTC_Set_Time(0x15,0x20,0x40,RTC_HOURFORMAT_24); 
	//设置时间 ,根据实际时间修改
	RTC_Set_Date(0x23,0x7,0x20,0x04); //设置日期
	 HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0X5051);
	//标记已经初始化过了
	 }
	//同时记得注释下面的TIME 和DATE初始化函数才行
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
//  sTime.Hours = 0;
//  sTime.Minutes = 0;
//  sTime.Seconds = 0;
//  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
//  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  sDate.WeekDay = RTC_WEEKDAY_THURSDAY;
//  sDate.Month = RTC_MONTH_JULY;
//  sDate.Date = 19;
//  sDate.Year = 23;

//  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
//  {
//    Error_Handler();
//  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0;
  sAlarm.AlarmTime.Minutes = 0;
  sAlarm.AlarmTime.Seconds = 0;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
//  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 60, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
//  {
//    Error_Handler();
//  }
  /* USER CODE BEGIN RTC_Init 2 */
  //设置是按秒唤醒是按小时唤醒是对133行代码进行设置！！！
	sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS;
	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();

    /* RTC interrupt Init */
    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* RTC interrupt Deinit */
    HAL_NVIC_DisableIRQ(RTC_WKUP_IRQn);
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

  //1：定时进入待机模式
  //2：定时闹钟唤醒
  void START_STANDBY(void)
  {
	 int iwdg_flag;
	 W25QXX_Read((uint8_t*)&iwdg_flag,IWDG_ADDR, sizeof(int));
	 if(iwdg_flag!=0)//正常运行一个周期
	 {	
		iwdg_flag=0;
		W25QXX_Write((uint8_t*)&iwdg_flag,IWDG_ADDR, sizeof(int));//标志位置0
		HAL_NVIC_SystemReset();//系统复位
	 }
	printf("Executing standby \r\n");
	GPIO_AnalogState_Config(); //设置IO口为模拟输入状态
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);//启用连接到PA.00的WakeUp Pin
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);  // 清除唤醒标记
	HAL_PWR_EnterSTANDBYMode();    //进入standby模式 
	printf("进入待机模式失败\r\n");

  }
  
  //设置IO口为模拟输入状态的函数，这个函数主要是为了在低功耗状态下，尽量控制IO上不必要的漏电流产生。
void GPIO_AnalogState_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    /*Set all GPIO in analog state to reduce power consumption*/
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
//	__HAL_RCC_GPIOD_CLK_ENABLE();
//    __HAL_RCC_GPIOE_CLK_ENABLE();
    
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = GPIO_PIN_All;
    
    HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);
    HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);
//	HAL_GPIO_Init(GPIOD,&GPIO_InitStruct);
//    HAL_GPIO_Init(GPIOE,&GPIO_InitStruct);

    
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
//	__HAL_RCC_GPIOD_CLK_DISABLE();
//    __HAL_RCC_GPIOE_CLK_DISABLE();
}

/* USER CODE END 1 */
