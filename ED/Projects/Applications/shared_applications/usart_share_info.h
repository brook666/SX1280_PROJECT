/*******************************************
1������USART1��USART2��USART3��Ҫ���ж�������Ϣ�ı�־
2�����������ļ���crc�ĺ�����ʱ����������
3���жϸ�λ����
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
  uint8_t  rxbuf[1024];   //ÿһ���������Ĵ�С
  uint8_t  rxbuf_vld;
  uint16_t rxbuf_len;    //����������
  uint16_t rxbuf_payload_len;  //69֡�������򳤶�
  uint8_t  alarm_info_flag;
  uint8_t  alarm_info[4];  
}usart_buffer_t;

uint8_t check_crc(uint8_t *frm, uint16_t len);
void DMA_ResetCurrDataCounter(DMA_Channel_TypeDef*,uint16_t );
#endif















