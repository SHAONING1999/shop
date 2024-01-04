/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
  *///串口接收不定长数据：https://blog.csdn.net/qq_41830158/article/details/121254705
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "w25qxx.h"
#include "spi.h"
#include "modbus.h"
#include "main.h"
#include "cmsis_os.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rtc.h"
#include "stm32f4xx_hal_rtc.h"
volatile uint8_t BC260_rec_flag = 0; //BC260一帧数据接收完成标志
volatile uint8_t EC20_rec_flag = 0; //BC260一帧数据接收完成标志
//串口1（蓝牙）（调试口）

volatile uint8_t rx1_len = 0;  //USART1接收一帧数据的长度
volatile uint8_t rec1_end_flag = 0; //一帧数据接收完成标志
uint8_t rx1_buffer[BUFFER_SIZE]={0};  //接收数据缓存数组
//串口2 （BC260）

volatile uint8_t rx2_len = 0;  //USART2接收一帧数据的长度
volatile uint8_t rec2_end_flag = 0; //一帧数据接收完成标志
uint8_t rx2_buffer[BUFFER_SIZE]={0};  //接收数据缓存数组
//串口3（传感器1）

//注意,读取USARTx->SR能避免莫名其妙的错误   	
uint8_t USART3_RX_BUF[USART3_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
uint16_t USART3_RX_STA=0;       //接收状态标记	
uint8_t USART3_aRxBuffer[USART3_RXBUFFERSIZE];//HAL库使用的串口接收缓冲
extern TIM_HandleTypeDef timer5handle;

//串口5 传感器2
//注意,读取USARTx->SR能避免莫名其妙的错误   	
uint8_t USART5_RX_BUF[USART5_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
uint16_t USART5_RX_STA=0;       //接收状态标记	
uint8_t USART5_aRxBuffer[USART5_RXBUFFERSIZE];//HAL库使用的串口接收缓冲
extern TIM_HandleTypeDef timer4handle;

uint8_t transbuffer[BUFFER_SIZE]={0};  //接收数据缓存数组
uint8_t RES;//串口接收缓冲区

//串口6（EC20）
volatile uint8_t rx6_len = 0;  //接收一帧数据的长度
volatile uint8_t rec6_end_flag = 0; //一帧数据接收完成标志
uint8_t rx6_buffer[BUFFER_SIZE]={0};  //接收数据缓存数组


extern volatile uint8_t Code_Versions[10]; //一帧数据接收完成标志
 //串口1重定向
 #if 1   
struct __FILE 
{ 
  int handle; 
}; 

FILE __stdout;  //定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
  x = x; 
} 
//重定义fputc函数
int fputc(int ch, FILE *f)
{ 	
  while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
  USART1->DR=(uint8_t)ch;      
  return ch;
}
#endif

 

/* USER CODE END 0 */

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart6_rx;
DMA_HandleTypeDef hdma_usart6_tx;

/* UART5 init function */
void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */
		int rboaud2;
  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */
  W25QXX_Read((uint8_t*)&rboaud2,BOAUD2_RATE_ADDR, sizeof(int));
  printf("传感器2波特率为:%d\r\n",rboaud2);
  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = rboaud2;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */
	HAL_UART_Receive_IT(&huart5, (uint8_t *)USART5_aRxBuffer, USART5_RXBUFFERSIZE);//接收一个字符该函数就会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
	clear_BUF(USART5_RX_BUF);

//	HAL_UART_Init(&huart5);//串口5波特率重初始化
  /* USER CODE END UART5_Init 2 */

}
/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */
	int rboaud1;
  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */
  W25QXX_Read((uint8_t*)&rboaud1,BOAUD1_RATE_ADDR, sizeof(int));
  printf("传感器1波特率:%d\r\n",rboaud1);

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = rboaud1;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */
  HAL_UART_Receive_IT(&huart3, (uint8_t *)USART3_aRxBuffer, USART3_RXBUFFERSIZE);//接收一个字符该函数就会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
  clear_BUF(USART3_RX_BUF);
//	W25QXX_Read((uint8_t*)&rboaud1,BOAUD1_RATE_ADDR, sizeof(int));
//	printf("sensor1 boaud:%d\r\n",rboaud1);
//	huart3.Init.BaudRate = rboaud1;
//	HAL_UART_Init(&huart3);//串口3波特率重初始化
  /* USER CODE END USART3_Init 2 */

}
/* USART6 init function */

void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspInit 0 */

  /* USER CODE END UART5_MspInit 0 */
    /* UART5 clock enable */
    __HAL_RCC_UART5_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**UART5 GPIO Configuration
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* UART5 interrupt Init */
    HAL_NVIC_SetPriority(UART5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspInit 1 */

  /* USER CODE END UART5_MspInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA2_Stream7;
    hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream5;
    hdma_usart2_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Stream6;
    hdma_usart2_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart2_tx);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
  else if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspInit 0 */

  /* USER CODE END USART6_MspInit 0 */
    /* USART6 clock enable */
    __HAL_RCC_USART6_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART6 DMA Init */
    /* USART6_RX Init */
    hdma_usart6_rx.Instance = DMA2_Stream1;
    hdma_usart6_rx.Init.Channel = DMA_CHANNEL_5;
    hdma_usart6_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart6_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_rx.Init.Mode = DMA_NORMAL;
    hdma_usart6_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart6_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart6_rx);

    /* USART6_TX Init */
    hdma_usart6_tx.Instance = DMA2_Stream6;
    hdma_usart6_tx.Init.Channel = DMA_CHANNEL_5;
    hdma_usart6_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart6_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart6_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart6_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart6_tx.Init.Mode = DMA_NORMAL;
    hdma_usart6_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart6_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart6_tx);

    /* USART6 interrupt Init */
    HAL_NVIC_SetPriority(USART6_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);
  /* USER CODE BEGIN USART6_MspInit 1 */

  /* USER CODE END USART6_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART5)
  {
  /* USER CODE BEGIN UART5_MspDeInit 0 */

  /* USER CODE END UART5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART5_CLK_DISABLE();

    /**UART5 GPIO Configuration
    PC12     ------> UART5_TX
    PD2     ------> UART5_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

    /* UART5 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART5_IRQn);
  /* USER CODE BEGIN UART5_MspDeInit 1 */

  /* USER CODE END UART5_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART6)
  {
  /* USER CODE BEGIN USART6_MspDeInit 0 */

  /* USER CODE END USART6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART6_CLK_DISABLE();

    /**USART6 GPIO Configuration
    PC6     ------> USART6_TX
    PC7     ------> USART6_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);

    /* USART6 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART6 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART6_IRQn);
  /* USER CODE BEGIN USART6_MspDeInit 1 */

  /* USER CODE END USART6_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void Usart1_IDLE(void)      //USART1的IDLE接收
{   
	uint32_t tmp_flag = 0;   
	uint32_t temp;
   	tmp_flag =__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE); //获取IDLE标志位
   if((tmp_flag != RESET))//idle标志被置位
   {
   		__HAL_UART_CLEAR_IDLEFLAG(&huart1);//清除标志位
   		HAL_UART_DMAStop(&huart1); //  停止DMA传输，防止
   		temp  =  __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);// 获取DMA中未传输的数据个数   
   		rx1_len =  BUFFER_SIZE - temp; //总计数减去未传输的数据个数，得到已经接收的数据个数
   		rec1_end_flag = 1;	// 接受完成标志位置1	
   }
}
void Usart1_Handle()     //USART1对接收的一帧数据进行处理
{
	unsigned char* p1;
	int urSamplingTime,urReportingtime;
	int uSamplingTime,uReportingtime;
	uint8_t mesbuf[200];//报文缓冲
	
	//参数配置
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+STime=");//在字符串中寻找AT+STime=，找到后返回A的位置
	if (p1 != NULL)
	{		
		uSamplingTime=atoi(( const char*)p1+9);//字符串转int
		if(uSamplingTime>=60)
		{
		W25QXX_Write((uint8_t*)&uSamplingTime, SamplingTime_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&urSamplingTime,SamplingTime_ADDR, sizeof(int));
		HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, urSamplingTime, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);//设置采样时间
		printf("采样周期=%d秒\r\n",uSamplingTime);	
		p1=NULL;
		}
		else
		{
			printf("采样周期小于最快频率60秒，请重新设置！！！\r\n");	
			p1=NULL;
		}
		
	}
	//采样上报周期设置
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+RTime=");//在字符串中寻找AT+RTime=，找到后返回A的位置
	if (p1 != NULL)
	{
		uReportingtime=atoi(( const char*)p1+9);//字符串转int
		
		W25QXX_Read((uint8_t*)&urSamplingTime,SamplingTime_ADDR, sizeof(int));
		uint8_t max_page=uReportingtime/urSamplingTime;
		if(max_page<=10)
		{
		W25QXX_Write((uint8_t*)&uReportingtime, Reportingtime_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&urReportingtime,Reportingtime_ADDR, sizeof(int));
		printf("上报周期=%d秒\r\n",urReportingtime);
		p1=NULL;
		}
		else
		{
			printf("采集周期/上报周期=%d，过大，影响系统平衡，请重新设置\r\n",max_page);
			p1=NULL;
		}

	}
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+CLOSE");//在字符串中寻找AT+CLOSE，找到后返回A的位置
	if (p1 != NULL)
	{//蓝牙断电，标志位清零
		printf("蓝牙关闭！\r\n");
		HAL_GPIO_WritePin(GET_PWR_GPIO_Port,BLE_DISCON_Pin,GPIO_PIN_RESET);//断开连接
		HAL_GPIO_WritePin(BLE_PWR_EN_GPIO_Port, BLE_PWR_EN_Pin, GPIO_PIN_RESET);//关闭电源
		HAL_NVIC_SystemReset();//系统复位
		p1=NULL;
	}
	//电池电压报警阈值设置
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+VALUE=");//在字符串中寻找AT+value=，找到后返回A的位置
	if (p1 != NULL)
	{	
		float f=atof((const char*)(p1+9));//接收字符串转浮点数给共用体的浮点成员
		W25Q128_Write_float(f,PWR_Tvalue_ADDR);
		printf("低电压报警阈值%.2f",W25Q128_Read_float(PWR_Tvalue_ADDR));
		p1=NULL;
	}
		//传感器个数配置配置
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+SENSORS=");//在字符串中寻找AT+MES=，找到后返回A的位置
	if (p1 != NULL)
	{
		int u1sensornum=0, u1rsensornum=0;;
		sscanf((const char*)p1,"AT+SENSORS=%d",&u1sensornum);	
		W25QXX_Write((uint8_t*)&u1sensornum, SENSORNUM_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&u1rsensornum,SENSORNUM_ADDR, sizeof(int));
		if(u1rsensornum==1)
		printf("传感器1配置使用\r\n");
		else if(u1rsensornum==2)
		printf("传感器2配置使用\r\n");
		else if(u1rsensornum==3)
		printf("传感器1,2同时配置使用\r\n");
		else if(u1rsensornum>3||0<u1rsensornum)
		printf("传感器配置错误，请重新配置！\r\n");
		p1=NULL;
	}
	//传感器2报文输送配置
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+MES2=");//在字符串中寻找AT+MES=，找到后返回A的位置
	if (p1 != NULL)
	{
		struct senser  u1rsenser2; 
		sscanf((const char*)p1,"AT+MES2=slave:%d;delay:%d;divenum:%d",&u1rsenser2.SlaveAddr,&u1rsenser2.Delays,&u1rsenser2.DivisorNum);	
		printf("传感器2从机地址：%d,延时时间：%d,因子数量：%d\r\n",u1rsenser2.SlaveAddr,u1rsenser2.Delays,u1rsenser2.DivisorNum);
		while(*p1++!='<')
		{}
		char* p2;
		p2=strtok((char*)p1,";");//字符分割
		for(int i=0;i<=u1rsenser2.DivisorNum;i++)
		{
		sscanf((const char*)p2,"%d,%d,%d,%d,%19s;",&u1rsenser2.RegisterAddress[i],&u1rsenser2.datenum [i],&u1rsenser2.datetype[i],&u1rsenser2.prasetype[i],u1rsenser2.Divisorname[i]);
		p2=strtok(NULL,";");

		}
		W25QXX_Write((uint8_t*)&u1rsenser2,SNE2DATE_ADDR,sizeof(u1rsenser2));//把报文信息写入flash

		W25QXX_Read((uint8_t*)&u1rsenser2,SNE2DATE_ADDR, sizeof(u1rsenser2));
		for(int i=0;i<u1rsenser2.DivisorNum;i++)
		{
		printf("因子名称:%s\r\n",u1rsenser2.Divisorname[i]);
		printf("寄存器起始地址:%d\r\n",u1rsenser2.RegisterAddress[i]);
		printf("数据个数:%d\r\n",u1rsenser2.datenum[i]);
		printf("数据类型:");
		switch (u1rsenser2.datetype[i])
		{
			case 0:printf("short\r\n"); 		break;
			case 1:printf("int:\r\n"); 			break;
			case 2:printf("float:\r\n"); 		break;
			case 3:printf("double:\r\n"); 		break;		
			case 4:printf("long long:\r\n"); 	break;		
		}
		printf("解析方式:");
		switch (u1rsenser2.prasetype[i])
		{
			case 0:printf("小端\r\n"); 		break;
			case 1:printf("小端交换\r\n"); 	break;
			case 2:printf("大端\r\n"); 		break;
			case 3:printf("大端交换\r\n"); 	break;		
		}
		printf("\r\n");
		}
		p1=NULL;
	}
		//传感器1报文输送配置
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+MES1=");//在字符串中寻找AT+MES=，找到后返回A的位置
	if (p1 != NULL)
	{
		struct senser  u1rsenser1; 
		sscanf((const char*)p1,"AT+MES1=slave:%d;delay:%d;divenum:%d",&senser1.SlaveAddr,&senser1.Delays,&senser1.DivisorNum);	
		printf("传感器1从机地址：%d,延时时间：%d,因子数量：%d\r\n",senser1.SlaveAddr,senser1.Delays,senser1.DivisorNum);
		while(*p1++!='<')
		{}
		char* p2;
		p2=strtok((char*)p1,";");//字符分割
		for(int i=0;i<=senser1.DivisorNum;i++)
		{
		sscanf((const char*)p2,"%d,%d,%d,%d,%19s;",&senser1.RegisterAddress[i],&senser1.datenum [i],&senser1.datetype[i],&senser1.prasetype[i],senser1.Divisorname[i]);
		p2=strtok(NULL,";");

		}
		W25QXX_Write((uint8_t*)&senser1,SNE1DATE_ADDR,sizeof(senser1));//把报文信息写入flash

		W25QXX_Read((uint8_t*)&u1rsenser1,SNE1DATE_ADDR, sizeof(u1rsenser1));
		for(int i=0;i<u1rsenser1.DivisorNum;i++)
		{
		printf("因子名称:%s\r\n",u1rsenser1.Divisorname[i]);
		printf("寄存器起始地址:%d\r\n",u1rsenser1.RegisterAddress[i]);
		printf("数据个数:%d\r\n",u1rsenser1.datenum[i]);
		printf("数据类型:");
		switch (u1rsenser1.datetype[i])
		{
			case 0:printf("short\r\n"); 		break;
			case 1:printf("int:\r\n"); 			break;
			case 2:printf("float:\r\n"); 		break;
			case 3:printf("double:\r\n"); 		break;		
			case 4:printf("long long:\r\n"); 	break;	
		}
		printf("解析方式:");
		switch (u1rsenser1.prasetype[i])
		{
			case 0:printf("小端\r\n"); 		break;
			case 1:printf("小端交换\r\n"); 	break;
			case 2:printf("大端\r\n"); 		break;
			case 3:printf("大端交换\r\n"); 	break;		
		}
		printf("\r\n");
		}
		p1=NULL;
	}
	//数据上传IP,端口号设置
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+IP=");//在字符串中寻找AT+IP=，找到后返回A的位置
	if (p1 != NULL)
	{
	    W25QXX_Write((p1+6),IP_ADDR,40);//把ip信息写入flash
		W25QXX_Read(mesbuf,IP_ADDR,40);
		printf("IP:%s\r\n",mesbuf);
		p1=NULL;
	}
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+PORT=");//在字符串中寻找AT+IP=，找到后返回A的位置
	if (p1 != NULL)
	{
	    int port;
		sscanf((const char*)p1,"AT+PORT=%d",&port);//字符串中提取浮点数
	    W25QXX_Write((uint8_t*)&port,PORT_ADDR,sizeof(int));//把ip信息写入flash
		W25QXX_Read((uint8_t*)&port,PORT_ADDR,sizeof(int));
		printf("端口号:%d\r\n",port);
		p1=NULL;
	}
		//设备GPS定位开启控制
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+AGPS");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		int gps_fun;
		sscanf((const char*)p1,"AT+AGPS%d",&gps_fun);//字符串中提取浮点数
		W25QXX_Write((uint8_t*)&gps_fun,AGPS_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&gps_fun,AGPS_ADDR, sizeof(int));
		if(gps_fun==1)
		{printf("开启GPS自动定位\r\n");}
		else if(gps_fun==0)
		{printf("关闭GPS自动定位\r\n");}
		else if(gps_fun!=0&&gps_fun!=1)
		{printf("GPS自动定位配置错误，请重新配置！\r\n");}
		p1=NULL;
	}
	//设备GPS定位经纬度设置
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+GPS=");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		float gps_lonngitude;
		float gps_latitude;
		sscanf((const char*)p1,"AT+GPS=%f-%f",&gps_lonngitude,&gps_latitude);//字符串中提取浮点数
		W25Q128_Write_float(gps_lonngitude,LONGITUDE_ADDR);//把经度信息写入flash
		W25Q128_Write_float(gps_latitude,LATITUDE_ADDR);//把纬度信息写入flash
		printf("设备经度:%f\r\n",W25Q128_Read_float(LONGITUDE_ADDR));
		printf("设备纬度:%f\r\n",W25Q128_Read_float(LATITUDE_ADDR));
		p1=NULL;
	}
	//设备波特率设置
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+BOAUD=");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		int boaud1,boaud2,rboaud1,rboaud2;
		sscanf((const char*)p1,"AT+BOAUD=%d,%d",&boaud1,&boaud2);
		W25QXX_Write((uint8_t*)&boaud1,BOAUD1_RATE_ADDR, sizeof(int));
		W25QXX_Write((uint8_t*)&boaud2,BOAUD2_RATE_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&rboaud1,BOAUD1_RATE_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&rboaud2,BOAUD2_RATE_ADDR, sizeof(int));
		printf("传感器1波特率:%d\r\n",rboaud1);
		printf("传感器2波特率:%d\r\n",rboaud2);
		p1=NULL;
	}
		//设备通讯方式设置
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+COMMOD=");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		int mod,rmod;
		sscanf((const char*)p1,"AT+COMMOD=%d",&mod);
		W25QXX_Write((uint8_t*)&mod,COMMOD_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&rmod,COMMOD_ADDR, sizeof(int));
		if(mod==1)
		printf("设备通讯模式:NB\r\n");
		else if(mod==2)
		printf("设备通讯模式:4G\r\n");
		p1=NULL;
	}
			//设备工作方式设置
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+STATE=");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		int state,rstate;
		sscanf((const char*)p1,"AT+STATE=%d",&state);
		W25QXX_Write((uint8_t*)&state,WORK_STATE_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&rstate,WORK_STATE_ADDR, sizeof(int));
		if(rstate==1)
			printf("设备工作模式:不待机连续工作\r\n");
		else if(rstate==2)
			printf("设备通讯模式:待机间歇工作\r\n");
		p1=NULL;
	}
			//设备MN编号设置
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+MN=");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		char mn[20],rmn[20];
		sscanf((const char*)p1,"AT+MN=%19s",mn);
		W25QXX_Write((uint8_t*)mn,MN_ADDR, 20*sizeof(char));
		W25QXX_Read((uint8_t*)rmn,MN_ADDR, 20*sizeof(char));
		printf("设备MN号:%s\r\n",mn);
		p1=NULL;
	}
	//设备补发页数清零设置
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+REISSUE=");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		int res=0,rres=100;
		sscanf((const char*)p1,"AT+REISSUE=%d",&res);
		W25QXX_Write((uint8_t*)&res,REISSUE_PAG_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&rres,REISSUE_PAG_ADDR, sizeof(int));
		printf("设备补发页数:%d\r\n",rres);
		p1=NULL;
	}
	//开启蓝牙监测模式
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+MONITOR=");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		int res=-1;
		sscanf((const char*)p1,"AT+MONITOR=%d",&res);
		if(res==0)
		{
			printf("关闭蓝牙报文监测模式\r\n");
			W25QXX_Write((uint8_t*)&res,MONITOR_ADDR, sizeof(int));
			
		}
		else if(res==1)
		{
			printf("开启蓝牙报文监测模式\r\n");
			W25QXX_Write((uint8_t*)&res,MONITOR_ADDR, sizeof(int));
		}
		else if(res!=0&&res!=1)
		{
			printf("配置错误，请检查配置报文正确性！\r\n");
		}
		p1=NULL;
	}
	
	
	
	//参数查询
	//采样周期查询 
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+STime?");//在字符串中寻找AT+STime=，找到后返回A的位置
	if (p1 != NULL)
	{		
		W25QXX_Read((uint8_t*)&urSamplingTime,SamplingTime_ADDR, sizeof(int));
		printf("采样周期=%d\r\n",urSamplingTime);
		
		p1=NULL;
	}
	//上报周期查询
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+RTime?");//在字符串中寻找AT+RTime=，找到后返回A的位置
	if (p1 != NULL)
	{
		W25QXX_Read((uint8_t*)&urReportingtime,Reportingtime_ADDR, sizeof(int));
		printf("上报周期=%d\r\n",urReportingtime);
		p1=NULL;

	}
	//电池电压报警阈值查询
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+VALUE?");//在字符串中寻找AT+value=，找到后返回A的位置
	if (p1 != NULL)
	{	
		printf("电池低电量报警阈值%.2f\r\n",W25Q128_Read_float(PWR_Tvalue_ADDR));
		p1=NULL;
	}
	//数据上传IP,端口号查询
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+IP?");//在字符串中寻找AT+IP=，找到后返回A的位置
	if (p1 != NULL)
	{
		W25QXX_Read(mesbuf,IP_ADDR,40);
		printf("%s\r\n",mesbuf);
		p1=NULL;
	}
	//设备GPS定位经纬度查询
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+GPS?");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		printf("经度:%f\r\n",W25Q128_Read_float(LONGITUDE_ADDR));
		printf("纬度:%f\r\n",W25Q128_Read_float(LATITUDE_ADDR));
		p1=NULL;
	}
	//设备物联网卡号查询
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+CIMI?");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		uint8_t cmmi1[22]={0},cmmi2[22]={0};
		W25QXX_Read(cmmi1,BC260_CMMI_ADDR,20);
		W25QXX_Read(cmmi2,EC20_CMMI_ADDR,20);
		printf("NB-IOT：%s\r\n ",cmmi1);
		printf("4G-CAT1：%s\r\n ",cmmi2);
		p1=NULL;
	}
	//设备软件版本查询
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+VERSIONS?");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		printf("代码版本:%f\r\n",VERSION);
		p1=NULL;
	}
		//设备RTC时间查询
		p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+RTC?");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		RTC_TimeTypeDef Time = {0};
		RTC_DateTypeDef Date = {0};
		HAL_RTC_GetTime (&hrtc,&Time,RTC_FORMAT_BIN);
		HAL_RTC_GetDate (&hrtc,&Date,RTC_FORMAT_BIN);
		printf("%d年%d月%d日%d时%d分%d秒\r\n",(Date.Year-48),Date.Month ,Date.Date ,Time.Hours ,Time.Minutes ,Time.Seconds );
		p1=NULL;
	}
		//设备故障码查询
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+ERROR?");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		
		p1=NULL;
	}
		//设备信号强度查询
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+CSQ?");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		
		p1=NULL;
	}
		//设备波特率查询
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+BOAUD?");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
		int rboaud1,rboaud2;
		W25QXX_Read((uint8_t*)&rboaud1,BOAUD1_RATE_ADDR, sizeof(int));
		W25QXX_Read((uint8_t*)&rboaud2,BOAUD2_RATE_ADDR, sizeof(int));
		printf("传感器1波特率:%d\r\n",rboaud1);
		printf("传感器2波特率:%d\r\n",rboaud2);
		p1=NULL;
	}
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+SET");//
	if (p1 != NULL)
	{
	 parseConfig(( const char*)rx1_buffer);//配置信息解析并存储
	}
	p1 = (unsigned char*)strstr(( const char*)rx1_buffer, "AT+SEEK");//在字符串中寻找AT+GPS=，找到后返回A的位置
	if (p1 != NULL)
	{
	 seekConfig();
	}
	rx1_len = 0;//清除计数
	rec1_end_flag = 0;//清除接收结束标志位
	memset(rx1_buffer,0,100);//清除缓存
	HAL_UART_Receive_DMA(&huart1,rx1_buffer,BUFFER_SIZE);//重新打开DMA接收
}

void DMA_Usart1_Send(uint8_t *buf,uint8_t len) //串口发送封装
{   
	if(HAL_UART_Transmit_DMA(&huart1,buf,len)!= HAL_OK) //判断是否发送正常，如果出现异常则进入异常中断函数
	{
  		Error_Handler();
 	}
}


void Usart2_IDLE(void)      //USART2的IDLE接收
{   
	uint32_t tmp_flag = 0;   
	uint32_t temp;
   	tmp_flag =__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE); //获取IDLE标志位
   if((tmp_flag != RESET))//idle标志被置位
   {
   		__HAL_UART_CLEAR_IDLEFLAG(&huart2);//清除标志位
   		HAL_UART_DMAStop(&huart2); //  停止DMA传输，防止
   		temp  =  __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);// 获取DMA中未传输的数据个数   
   		rx2_len =  BUFFER_SIZE - temp; //总计数减去未传输的数据个数，得到已经接收的数据个数
   		rec2_end_flag = 1;	// 接受完成标志位置1	
		BC260_rec_flag=1;	//BC260标志位置1
   }
}

void Usart2_Handle()     //USART1对接收的一帧数据进行处理
{ 
	// DMA_Usart1_Send(rx2_buffer, rx2_len);  //将串口2接收到的数据通过串口1打印出来
   uint16_t i=strlen((const char*)rx2_buffer);       //i(此次发送数据的长度)
	vPortEnterCritical();
	printf("USART2接收：%s\r\n",rx2_buffer);  //串口1打印显示串口2发送的结果s
	vPortExitCritical();
   HAL_UART_Receive_DMA(&huart2,rx2_buffer,BUFFER_SIZE);//重新打开DMA接收
	rx2_len = 0;//清除计数
   rec2_end_flag = 0;//清除接收结束标志位
}

void DMA_Usart2_Send(uint8_t *buf,uint8_t len) //串口发送封装
{   
	if(HAL_UART_Transmit_DMA(&huart2,buf,len)!= HAL_OK) //判断是否发送正常，如果出现异常则进入异常中断函数
	{
  		Error_Handler();
 	}
}

void clear_BUF(uint8_t* usart)
{
		if(usart==rx2_buffer)
	{
		memset(rx2_buffer,0,100);
		BC260_rec_flag=0;
	}
			if(usart==rx6_buffer)
	{
		memset(rx6_buffer,0,100);
		EC20_rec_flag=0;
	}
		if(usart==USART3_RX_BUF)
	{
		USART3_RX_STA=0;//清除接收标志位
		memset(USART3_RX_BUF,0,USART3_REC_LEN);//清除缓存
	}
	if(usart==USART5_RX_BUF)
	{
		USART5_RX_STA=0;//清除接收标志位
		memset(USART5_RX_BUF,0,USART5_REC_LEN);//清除缓存
	}
}

void USART3_send_to_Meter(uint8_t *buf,uint8_t len) 
{
		HAL_UART_Transmit(&huart3,buf,len,1000);//串口3发送数据
}

void USART5_send_to_Meter(uint8_t *buf,uint8_t len) 
{
		HAL_UART_Transmit(&huart5,buf,len,1000);//串口3发送数据
}


void Modbus1_Send_Byte(  uint8_t ch )
{
	/* 发送一个字节数据到USART3 */
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xff);	
}

void Modbus2_Send_Byte(  uint8_t ch )
{
	/* 发送一个字节数据到USART5 */
    HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, 0xff);	
}
//4G模块串口驱动


void Usart6_IDLE(void)      //USART6的IDLE接收
{   
	uint32_t tmp_flag = 0;   
	uint32_t temp;
   	tmp_flag =__HAL_UART_GET_FLAG(&huart6,UART_FLAG_IDLE); //获取IDLE标志位
   if((tmp_flag != RESET))//idle标志被置位
   {
   		__HAL_UART_CLEAR_IDLEFLAG(&huart6);//清除标志位
   		HAL_UART_DMAStop(&huart6); //  停止DMA传输，防止
   		temp  =  __HAL_DMA_GET_COUNTER(&hdma_usart6_rx);// 获取DMA中未传输的数据个数   
   		rx6_len =  BUFFER_SIZE - temp; //总计数减去未传输的数据个数，得到已经接收的数据个数
   		rec6_end_flag = 1;	// 接受完成标志位置1	
		EC20_rec_flag=1;	//EC20标志位置1
   }
}

void Usart6_Handle()     //USART1对接收的一帧数据进行处理
{
   uint16_t i=strlen((const char*)rx6_buffer);       //i(此次发送数据的长度)

	vPortEnterCritical();
	printf("USART6接收：%s\r\n",rx6_buffer);  //串口1打印显示串口2发送的结果
	//   DMA_Usart6_Send(rx6_buffer, rx6_len);  //将接收到的数据回发给发送端
	vPortExitCritical();
	HAL_UART_Receive_DMA(&huart6,rx6_buffer,BUFFER_SIZE);//重新打开DMA接收
	rx6_len = 0;//清除计数
	rec6_end_flag = 0;//清除接收结束标志位
	
}

void DMA_Usart6_Send(uint8_t *buf,uint8_t len) //串口发送封装
{   
	if(HAL_UART_Transmit_DMA(&huart6,buf,len)!= HAL_OK) //判断是否发送正常，如果出现异常则进入异常中断函数
	{
  		Error_Handler();
 	}
}

/* USER CODE END 1 */
