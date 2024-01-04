/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
typedef struct 
	{
		uint8_t t;
		uint8_t DateGet_flag;//数据获取完毕标志�?
		uint8_t DateWrite_flag;//数据写入flash完毕标志�?
		uint8_t DateSend_flag;//数据发�?�完毕标志位
	}flag_bit;
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
	extern flag_bit flag;
	extern float VERSION;//�����汾
	#define _VERSION_  3.70
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
	//long long 类型结构�?
	typedef struct {
    uint32_t low;
    uint32_t high;
} long_long_t;
struct senser 
{
	int SlaveAddr;//从机地址
	int Delays;//延时时间
	int DivisorNum;//因子个数
	
	char Divisorname[10][20];////因子名称
	int RegisterAddress[10];//寄存器起始地�?
	int datenum[10];//读取数据数量
	int datetype[10];//数据类型
	int prasetype[10];//解析格式

	
	double Divisordouble[10];//因子数�?�double类型
	float Divisorfloat[10];//因子数�?�float类型
	int Divisorint[10];//因子数�?�int类型
	short Divisorshort[10];//因子数据short类型
	long long Divisorlonglong[10];//因子数据1ong long类型
};

typedef struct {
    int STIME;
    int RTIME;
    float VALUE;
    char IP[25];
    int PORT;
    float LONGITUDE;
    float LATITUDE;
    char MN[20];
    int BOAUD1, BOAUD2;
	float Volt;
    int COMMOD, SENSORNUM, AGPS, STATE;
	char card_4g[20];
	char card_nb[20];
	float version;
} Config;

extern struct senser  senser1; 
extern struct senser  rsenser1; 
extern struct senser  senser2; 
extern struct senser  rsenser2; 
extern uint8_t BLE_Flag ;
extern int rwork_state ;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
extern  int sensornum;
extern int rSamplingTime,rReportingTime;
extern  int max_page;
extern  int rsensornum;
extern struct senser  senser1; 
void send(struct senser seb);
void sensor1_init(struct senser* sen1,uint8_t* buff);
void sensor2_init(struct senser* sen2,uint8_t* buff);
void Sensor1_Date_Prase(struct senser* seb,uint8_t* buff,int t);
void Sensor2_Date_Prase(struct senser* seb,uint8_t* buff,int t);
void Sensor1_Date_Add(struct senser* seb,char* sendbuff);
void Sensor2_Date_Add(struct senser* seb,char* sendbuff);

Config parseConfig(const char* input);
int seekConfig(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define NET_LED_Pin GPIO_PIN_2
#define NET_LED_GPIO_Port GPIOE
#define BLE_DEBUG_Pin GPIO_PIN_4
#define BLE_DEBUG_GPIO_Port GPIOE
#define BLE_DEBUG_EXTI_IRQn EXTI4_IRQn
#define SENS1_PWR_EN_Pin GPIO_PIN_1
#define SENS1_PWR_EN_GPIO_Port GPIOC
#define PWR_LED_Pin GPIO_PIN_6
#define PWR_LED_GPIO_Port GPIOA
#define PWR_EN_4G_Pin GPIO_PIN_12
#define PWR_EN_4G_GPIO_Port GPIOB
#define ALM_LED_Pin GPIO_PIN_13
#define ALM_LED_GPIO_Port GPIOB
#define GET_PWR_Pin GPIO_PIN_11
#define GET_PWR_GPIO_Port GPIOD
#define BLE_DISCON_Pin GPIO_PIN_12
#define BLE_DISCON_GPIO_Port GPIOD
#define BLE_PWR_EN_Pin GPIO_PIN_13
#define BLE_PWR_EN_GPIO_Port GPIOD
#define BLE_STA_Pin GPIO_PIN_15
#define BLE_STA_GPIO_Port GPIOD
#define NB_PWR_EN_Pin GPIO_PIN_11
#define NB_PWR_EN_GPIO_Port GPIOA
#define SEN2_PWR_EN_Pin GPIO_PIN_12
#define SEN2_PWR_EN_GPIO_Port GPIOA
#define PWR_FLASH_Pin GPIO_PIN_0
#define PWR_FLASH_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
