/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#define BUFFER_SIZE  1024  

extern volatile uint8_t BC260_rec_flag; //BC260���ձ�־λ
extern volatile uint8_t EC20_rec_flag; //BC260���ձ�־λ
extern volatile int rx1_len;  //����һ֡���ݵĳ���
extern volatile uint8_t rec1_end_flag; //һ֡���ݽ�����ɱ�־
extern uint8_t rx1_buffer[BUFFER_SIZE];  //�������ݻ�������

void Usart1_Handle(void);
void DMA_Usart1_Send(uint8_t *buf,uint8_t len);//���ڷ��ͷ�װ
void Usart1_IDLE(void);
void clear_BUF(uint8_t * usart);

extern volatile int rx2_len;  //����һ֡���ݵĳ���
extern volatile uint8_t rec2_end_flag; //һ֡���ݽ�����ɱ�־
extern uint8_t rx2_buffer[BUFFER_SIZE];  //�������ݻ�������

void Usart2_Handle(void);
void DMA_Usart2_Send(uint8_t *buf,uint8_t len);//���ڷ��ͷ�װ
void Usart2_IDLE(void);
/* USER CODE END Includes */

extern UART_HandleTypeDef huart5;

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

extern UART_HandleTypeDef huart6;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_UART5_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART6_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

