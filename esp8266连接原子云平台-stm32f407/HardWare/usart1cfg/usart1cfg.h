#ifndef __USART1CFG_H
#define __USART1CFG_H
#include "stm32f4xx.h"
#include "sys.h"

void USART1_Config(uint32_t BoudRate);
void usart1_printf(char* fmt, ...);

#endif // #ifndef __USART1CFG_H
