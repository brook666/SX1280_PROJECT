/*******************************************
1、包含USART1、USART2、USART3需要的判断输入信息的标志
2、包含公共的检验crc的函数，时钟启动函数
3、中断复位函数
********************************************/

#ifndef USART_FRM_TYPE_H
#define USART_FRM_TYPE_H

#include <stdio.h>
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_dma.h"

#define MAX_DATA_LEN   2074
#define FRM_HEAD       0x69
#define FRM_END        0x17
#define MAX_LEN_FRM1   100
#define USART1_RXBUF_SIZE 5
#define USART2_RXBUF_SIZE 5
#define USART_RXBUF_SIZE 5

/// define FSM state
#define IDLE       0x00
#define VALID      0X01


enum app_usart_proc{
  APP_USART_FRAME_NOT_COMPLETE,
  APP_USART_FRM_HEAD_SUCCESS,
  APP_USART_FRM_END_SUCCESS,
  APP_USART_FRM_HEAD_ERR,
  APP_USART_FRM_END_ERR,  
  APP_USART_FRM_ERR,
  APP_USART_CRC_ERR,
  APP_USART_LEN_ERR,
  APP_USART_BUFFER_FULL_ERR,
};

typedef enum app_usart_proc app_usart_proc_t;


typedef struct
{
  uint8_t  rxbuf[1024];   //每一个缓冲区的大小
  uint8_t  rxbuf_vld;
  uint16_t rxbuf_len;    //缓冲区长度
  uint16_t rxbuf_payload_len;  //69帧中数据域长度
  uint8_t  alarm_info_flag;
  uint8_t  alarm_info[4];  
}usart_buffer_t;

uint8_t check_crc(uint8_t *frm, uint16_t len);
void DMA_ResetCurrDataCounter(DMA_Channel_TypeDef*,uint16_t );
#endif















