#include "SerialCOM.h"
#include "usart.h"

/**
  * @brief  Initialize the IAP: Configure USART.
  * @param  None
  * @retval None
  */
void SerialCOM_Init(void)
{
	UartHandle.Instance=Serial_COM;					    
	UartHandle.Init.BaudRate=115200;				   
	UartHandle.Init.WordLength=UART_WORDLENGTH_8B;  
	UartHandle.Init.StopBits=UART_STOPBITS_1;	    
	UartHandle.Init.Parity=UART_PARITY_NONE;		   			
	UartHandle.Init.HwFlowCtl=UART_HWCONTROL_NONE;		
	UartHandle.Init.Mode=UART_MODE_TX_RX;		  			
	HAL_UART_Init(&UartHandle);	
}

/**
  * @brief  UARTµ×²ã³õÊ¼»¯
  * @param  None
  * @retval None
  */
//void HAL_UART_MspInit(UART_HandleTypeDef *huart)
//{
//	GPIO_InitTypeDef GPIO_InitStruct;
//	
//	if(huart->Instance==USART1)					//debug
//  {
//    __HAL_RCC_USART1_CLK_ENABLE();
//    __HAL_RCC_GPIOC_CLK_ENABLE();

//    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_PULLUP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
//    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//  }
//	else if(huart->Instance==USART3)			//USR
//	{
//    __HAL_RCC_USART3_CLK_ENABLE();
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    /**USART3 GPIO Configuration
//    PB10     ------> USART3_TX
//    PB11     ------> USART3_RX
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_PULLUP;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//    /* USART3 interrupt Init */
////    HAL_NVIC_SetPriority(USART3_IRQn, 4, 0);
////    HAL_NVIC_EnableIRQ(USART3_IRQn);
//	}
//}
