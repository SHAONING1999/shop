#ifndef __delay_H__
#define	__delay_H__

#include "main.h"

#define SYSCLK_MHZ    (SystemCoreClock/1000000) //系统时钟频率

void delay_us(uint16_t us);
void delay_ms(uint16_t ms);
void delay_s(uint16_t s);

#endif
