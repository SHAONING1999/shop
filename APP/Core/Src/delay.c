#include "delay.h"

void delay_us(uint16_t us)
{
	uint32_t Tdata = us*SYSCLK_MHZ/5;
	for(uint32_t i=0;i<Tdata;i++)
	{
		__NOP();
	}
}

void delay_ms(uint16_t ms)
{ 
	while( ms-- != 0)
	{
		delay_us(1000);	
	}
}

void delay_s(uint16_t s)
{ 	  
	while( s-- != 0)
	{
		delay_ms(1000);	
	}
}
