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

extern volatile uint8_t BC260_rec_flag; //BC260鎺ユ敹鏍囧織�??
extern volatile uint8_t EC20_rec_flag; //BC260鎺ユ敹鏍囧織�??
extern volatile uint8_t rx1_len;  //鎺ユ敹涓?甯ф暟鎹殑闀垮害
extern volatile uint8_t rec1_end_flag; //�??甯ф暟鎹帴鏀跺畬鎴愭爣蹇?
extern uint8_t rx1_buffer[BUFFER_SIZE];  //鎺ユ敹鏁版嵁缂撳瓨鏁扮粍

void Usart1_Handle(void);
void DMA_Usart1_Send(uint8_t *buf,uint8_t len);//涓插彛鍙�??佸皝�??
void Usart1_IDLE(void);
void clear_BUF(uint8_t * usart);

extern volatile uint8_t rx2_len;  //鎺ユ敹涓?甯ф暟鎹殑闀垮害
extern volatile uint8_t rec2_end_flag; //�??甯ф暟鎹帴鏀跺畬鎴愭爣蹇?
extern uint8_t rx2_buffer[BUFFER_SIZE];  //鎺ユ敹鏁版嵁缂撳瓨鏁扮粍

void Usart2_Handle(void);
void DMA_Usart2_Send(uint8_t *buf,uint8_t len);//涓插彛鍙�??佸皝�??
void Usart2_IDLE(void);

#define USART2_REC_LEN  			200  	//瀹氫箟鏈?澶ф帴�?跺瓧鑺傛�? 50

	  	
extern uint8_t  USART2_RX_BUF[USART2_REC_LEN]; //鎺ユ敹缂撳啿,�??澶SART_REC_LEN涓瓧鑺?.鏈瓧鑺備负鎹㈣绗? 
extern uint16_t USART2_RX_STA;         		//鎺ユ敹鐘�??佹爣�??	
extern UART_HandleTypeDef UART2_Handler; //UART鍙ユ�?


#define USART3_RXBUFFERSIZE   1 //缂撳瓨澶у�?
#define RXBUFFERSIZE   1 //缂撳瓨澶у�?
#define USART3_REC_LEN  			200  	//瀹氫箟鏈?澶ф帴�?跺瓧鑺傛�? 50
extern uint16_t USART3_RX_STA;         		//鎺ユ敹鐘�??佹爣�??
extern uint8_t  USART3_RX_BUF[USART3_REC_LEN]; //鎺ユ敹缂撳啿,�??澶SART_REC_LEN涓瓧鑺?.鏈瓧鑺備负鎹㈣绗? 
extern uint8_t USART3_aRxBuffer[RXBUFFERSIZE];//HAL搴揢SART鎺ユ敹Buffer
extern void USART3_send_to_Meter(uint8_t *buf,uint8_t len) ;
extern void HAL_UART3_MspInit(UART_HandleTypeDef *huart);
void clear_BUF(uint8_t* usart);//涓插彛娓呯悊鍑芥�?
void Modbus1_Send_Byte(  uint8_t ch );//RS485-1(USART3)鍙戦?佸嚱�??
void Modbus2_Send_Byte(  uint8_t ch );//RS485-2(USART5)鍙戦?佸嚱�??
#define USART5_RXBUFFERSIZE   1 //缂撳瓨澶у�?
#define RXBUFFERSIZE   1 //缂撳瓨澶у�?
#define USART5_REC_LEN  			200  	//瀹氫箟鏈?澶ф帴�?跺瓧鑺傛�? 50
extern uint16_t USART5_RX_STA;         		//鎺ユ敹鐘�??佹爣�??
extern uint8_t  USART5_RX_BUF[USART5_REC_LEN]; //鎺ユ敹缂撳啿,�??澶SART_REC_LEN涓瓧鑺?.鏈瓧鑺備负鎹㈣绗? 
extern uint8_t USART5_aRxBuffer[RXBUFFERSIZE];//HAL搴揢SART鎺ユ敹Buffer
extern void USART5_send_to_Meter(uint8_t *buf,uint8_t len) ;
extern void HAL_UART5_MspInit(UART_HandleTypeDef *huart);

extern volatile uint8_t rx6_len;  //鎺ユ敹涓?甯ф暟鎹殑闀垮害
extern volatile uint8_t rec6_end_flag; //�??甯ф暟鎹帴鏀跺畬鎴愭爣蹇?
extern uint8_t rx6_buffer[BUFFER_SIZE];  //鎺ユ敹鏁版嵁缂撳瓨鏁扮粍

void Usart6_Handle(void);
void DMA_Usart6_Send(uint8_t *buf,uint8_t len);//涓插彛鍙�??佸皝�??
void Usart6_IDLE(void);

void UART_Tx(uint8_t *Tx_Buf,uint16_t TxCount);
/* USER CODE END Includes */

extern UART_HandleTypeDef huart5;

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

extern UART_HandleTypeDef huart3;

extern UART_HandleTypeDef huart6;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_UART5_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);
void MX_USART6_UART_Init(void);

/* USER CODE BEGIN Prototypes */
static char cmd_standby[] = "AT+STANDBY";
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

