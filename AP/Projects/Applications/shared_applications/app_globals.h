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
  uint16_t precnt;   //DMA中一帧的起始位置
  uint16_t curcnt;   //DMA中一帧的结束位置
  volatile uint8_t  recvFlag;  
  volatile uint8_t  recvfrmcnt;  //接收到帧的数量
  uint8_t  RecvData[1024];
}DMA_RecvData_t;


extern globalflag_t  globalFlag;

//结构体在文件"usart_dma_proc.h"中定义，
//变量在文件"usart_dma_proc.c"中定义
extern DMA_RecvData_t usart2_recv_data ;
extern usart_buffer_t uart2_frm_rbuf ;


#define  USART2_DMA_BUFFER_SIZE 1024

#endif



