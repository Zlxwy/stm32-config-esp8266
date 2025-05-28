#ifndef __ESP8266CFG_H
#define __ESP8266CFG_H
#include "stm32f10x.h"                  // Device header

void usart3_printf(char* fmt, ...);
uint8_t get_TC_flag(void);
uint8_t* get_rx_pack(void);
void clear_rx_pack(void);
uint8_t check_res_state(const char *str);

void ESP8266_Init(void);
void command_init(void);
void esp8266_uart_init(void);
void esp8266_timer_init(void);

#endif
