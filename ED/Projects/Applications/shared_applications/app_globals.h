#ifndef _APP_GLOBALS_H
#define _APP_GLOBALS_H
#include "usart_share_info.h"

typedef struct
{
  uint8_t uart1_dma_flag;
  uint8_t uart2_dma_flag;
}globalflag_t;

typedef struct
{
  uint16_t precnt;   //DMA��һ֡����ʼλ��
  uint16_t curcnt;   //DMA��һ֡�Ľ���λ��
  volatile uint8_t  recvFlag;  
  volatile uint8_t  recvfrmcnt;  //���յ�֡������
  uint8_t  RecvData[1024];
}DMA_RecvData_t;


extern globalflag_t  globalFlag;

//�ṹ�����ļ�"usart_dma_proc.h"�ж��壬
//�������ļ�"usart_dma_proc.c"�ж���
extern DMA_RecvData_t usart2_recv_data ;
extern usart_buffer_t uart2_frm_rbuf ;
extern DMA_RecvData_t usart1_recv_data ;
extern usart_buffer_t uart1_frm_rbuf ;

#define  USART2_DMA_BUFFER_SIZE 1024
#define  USART1_DMA_BUFFER_SIZE 1024
#endif



