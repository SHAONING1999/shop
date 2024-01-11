#ifndef __SERIALCOM_H__
#define __SERIALCOM_H__

#include "stm32f4xx_hal.h"
#include "common.h"
#include "flash_if.h"
#include "menu.h"
#include "ymodem.h"
#include "usart.h"

#define Serial_COM          USART1  //移植封装，不要改
#define UartHandle          huart1 //定向于USART1，想修改串口改这个
void SerialCOM_Init(void);




















#endif




