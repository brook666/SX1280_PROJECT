#ifndef _USART_DMA_PROC_H
#define _USART_DMA_PROC_H

#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
#include "usart_share_info.h"
#include "stm32_usart2_gtway.h"
#include "app_globals.h"
#include "delay.h"
#include "bsp_config.h"
#include "bsp.h"

void Usart2_DMA_Configuration(void);
void Usart1_DMA_Configuration(void);

void TIM4_Configuration(void);
void TIM2_Configuration(void);
void read_Usart2_DMA_FIFO(usart_buffer_t*);
void read_Usart1_DMA_FIFO(usart_buffer_t*);
app_usart_proc_t check_usart2_frame_head(uint16_t * , uint16_t *  );
app_usart_proc_t check_usart2_frame_end_and_crc(uint16_t *);
void usart2_err_proc(usart_buffer_t*);
uint8_t check_usart_recv_frm(void);
uint8_t usart_getFrameReceiveCount(void);
void Iterates_usart2_buffer(void);
void Iterates_usart1_buffer(void);

#endif


