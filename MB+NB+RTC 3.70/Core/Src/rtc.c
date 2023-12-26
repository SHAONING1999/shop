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

//RTC��������
//year,month,date:��(0~99),��(1~12),��(0~31)
//week:����(1~7,0,�Ƿ�!)
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ�� 
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
	__HAL_RCC_PWR_CLK_ENABLE();//ʹ�ܵ�Դʱ��PWR
	HAL_PWR_EnableBkUpAccess();//ȡ����������д����
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
  //��RTCʱ�����ݱ����ڱ�����
	if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR0)!=0X5051)
	//�Ƿ��һ������
	{ 
	 RTC_Set_Time(0x15,0x20,0x40,RTC_HOURFORMAT_24); 
	//����ʱ�� ,����ʵ��ʱ���޸�
	RTC_Set_Date(0x23,0x7,0x20,0x04); //��������
	 HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0X5051);
	//����Ѿ���ʼ������
	 }
	//ͬʱ�ǵ�ע�������TIME ��DATE��ʼ����������
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
  //�����ǰ��뻽���ǰ�Сʱ�����Ƕ�133�д���������ã�����
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

  //1����ʱ�������ģʽ
  //2����ʱ���ӻ���
  void START_STANDBY(void)
  {
	 int iwdg_flag;
	 W25QXX_Read((uint8_t*)&iwdg_flag,IWDG_ADDR, sizeof(int));
	 if(iwdg_flag!=0)//��������һ������
	 {	
		iwdg_flag=0;
		W25QXX_Write((uint8_t*)&iwdg_flag,IWDG_ADDR, sizeof(int));//��־λ��0
		HAL_NVIC_SystemReset();//ϵͳ��λ
	 }
	printf("Executing standby \r\n");
	GPIO_AnalogState_Config(); //����IO��Ϊģ������״̬
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);//�������ӵ�PA.00��WakeUp Pin
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);  // ������ѱ��
	HAL_PWR_EnterSTANDBYMode();    //����standbyģʽ 
	printf("�������ģʽʧ��\r\n");

  }
  
  //����IO��Ϊģ������״̬�ĺ��������������Ҫ��Ϊ���ڵ͹���״̬�£���������IO�ϲ���Ҫ��©����������
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
