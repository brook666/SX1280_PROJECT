#ifndef STM32_CC_UART2_H
#define STM32_CC_UART2_H

#include "stm32f10x.h"
#include <stdio.h>
#include <stdint.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
#include "usart_share_info.h"
#include "app_globals.h"
#include "delay.h"
#include "bsp_config.h"

void Usart2_Configuration(uint32_t BaudRate,FunctionalState NewState);
void put_string_uart2( uint8_t *ptr);
void put_char_uart2(uint8_t send_data);
void put_multi_char_uart2(uint8_t* multi_char, uint16_t len);
void put_hex_uart2(uint8_t send_data);
void put_hex_uart1(uint8_t send_data);
void put_multi_hex_uart2(uint8_t *send_data, uint16_t len);
void put_multi_hex_uart1(uint8_t *send_data, uint16_t len);
//char get_hex_uart2(uint8_t *rec_data);
//uint8_t Get_hex_uart2(uint8_t *rec_data);
void USART2_NVIC_Configuration(FunctionalState NewState);
void USART1_NVIC_Configuration(FunctionalState NewState);
void Usart1_Configuration(uint32_t BaudRate,FunctionalState NewState);
void TIM2_Configuration(void);
#endif


