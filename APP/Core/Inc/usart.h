/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#define BUFFER_SIZE  256  
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */
extern volatile uint8_t BC260_rec_flag; //BC260接收标志�?
extern volatile uint8_t EC20_rec_flag; //BC260接收标志�?
extern volatile uint8_t rx1_len;  //接收�?帧数据的长度
extern volatile uint8_t rec1_end_flag; //�?帧数据接收完成标�?
extern uint8_t rx1_buffer[BUFFER_SIZE];  //接收数据缓存数组

void Usart1_Handle(void);
void DMA_Usart1_Send(uint8_t *buf,uint8_t len);//串口发�?�封�?
void Usart1_IDLE(void);
void clear_BUF(uint8_t * usart);

extern volatile uint8_t rx2_len;  //接收�?帧数据的长度
extern volatile uint8_t rec2_end_flag; //�?帧数据接收完成标�?
extern uint8_t rx2_buffer[BUFFER_SIZE];  //接收数据缓存数组

void Usart2_Handle(void);
void DMA_Usart2_Send(uint8_t *buf,uint8_t len);//串口发�?�封�?
void Usart2_IDLE(void);

#define USART2_REC_LEN  			200  	//定义�?大接收字节数 50

	  	
extern uint8_t  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,�?大USART_REC_LEN个字�?.末字节为换行�? 
extern uint16_t USART2_RX_STA;         		//接收状�?�标�?	
extern UART_HandleTypeDef UART2_Handler; //UART句柄
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

