/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   uart_isr.c
// Description: User APP.
//              串口接收中断服务函数.串口接收帧间间隔定时器中断服务函数.用于获
//              取一条完整的帧.
// Author:      Leidi
// Version:     1.0
// Date:        2014-9-26
// History:     2014-9-26   Leidi   由原始代码整理而建立此文件.
//                                  理冗余代码.修改调试信息.
//                                  使用宏定义MAX_FRAME_LEN.
//              2014-11-10  Leidi   移除头文件stm8s_uart.h.
//                                  添加用于配置的头文件user_app_config.h.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "user_app_config.h"
#include "bsp.h"
#include "bsp_relay_defs.h"
#include <string.h>

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAX_FRAME_LEN  40 //允许接收的最大帧长

/* 串口接收状态机的状态定义 */
#define IDLE                     0x00
#define GET_FRM_TO_BUF1          0x05
#define GET_FRM_TO_BUF2          0x06

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
/* 串口接收缓冲区 */
uint8_t  uart_frm_rbuf1[MAX_FRAME_LEN]={0};
uint8_t  uart_frm_rbuf2[MAX_FRAME_LEN]={0};

/* 串口接收缓冲区有效标志 */
uint8_t  uart_frm_rbuf1_vld = 0;
uint8_t  uart_frm_rbuf2_vld = 0;

/* 串口接收缓冲区有效长度 */
uint8_t  uart_frm_rbuf1_rxlen=0;
uint8_t  uart_frm_rbuf2_rxlen=0;

/* ------------------------------------------------------------------------------------------------
 *                                            Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static  uint8_t  uart_time_cnt;     //帧间间隔定时器中断次数
static  uint8_t  cur_state = IDLE;  //串口接收状态机状态
static  uint8_t  frm_data_cnt = 0;  //接收缓冲区中已有字节数


/**************************************************************************************************
 * @fn          USART1_RXISR
 *
 * @brief       串口接收中断服务函数,用于将串口接收到的数据形成一条完整的帧,并放入全局缓冲区中.使用
 *              双缓冲机制.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
BSP_ISR_FUNCTION(USART1_RXISR, USART1_RX_IRQn + 2)
{
  uint8_t  rec_data;
  
  /* 启动定时器 */
  TIM3_RESET_COUNT();
  TIM3_ENABLE();
 
  switch(cur_state)
  {
    case IDLE:
    {
      frm_data_cnt = 0;   
      if(RECEIVE_BYTE_FROM_UART(&rec_data))
      {
          if(!uart_frm_rbuf1_vld)
          {
            ERR_OUTPUT_LOW(); //发送出错指示位：撤销节点出错信号
            uart_frm_rbuf1[frm_data_cnt++] = rec_data; 
            cur_state = GET_FRM_TO_BUF1;
          }
          else if(!uart_frm_rbuf2_vld)
          {
            ERR_OUTPUT_LOW(); //发送出错指示位：撤销节点出错信号
            uart_frm_rbuf2[frm_data_cnt++] = rec_data;
            cur_state = GET_FRM_TO_BUF2;
          }
          else
          {
            cur_state = IDLE;
            PRINT_DEBUG_MSG("Uart buff OVERFLOW./n");  
          }
      }
      else
      {
        //Uart_Init();
        cur_state = IDLE;
        PRINT_DEBUG_MSG("Uart receive byte ERROR.\n");  
      }
      break;
    }
    case GET_FRM_TO_BUF1:
    { 
      if(RECEIVE_BYTE_FROM_UART(&rec_data))
      {
        if(frm_data_cnt == MAX_FRAME_LEN-1)
        {
          USART_RX_DISABLE(); //串口接收禁止
          RDY_OUTPUT_LOW();   //发送空闲指示位：节点忙
          cur_state = IDLE;  
          uart_frm_rbuf1_vld = TRUE;
          uart_frm_rbuf1[frm_data_cnt] =  rec_data;   
          uart_frm_rbuf1_rxlen = MAX_FRAME_LEN;   
          frm_data_cnt= 0;
          TIM3_DISABLE();     //停止定时器
        }
        else if(frm_data_cnt > MAX_FRAME_LEN-1)
        {
          cur_state = IDLE;
          PRINT_DEBUG_MSG("Uart buff_1 OVERFLOW.\n");  
        }
        else
        {
          uart_frm_rbuf1[frm_data_cnt] =  rec_data;     
          frm_data_cnt++;
          cur_state = GET_FRM_TO_BUF1; 
        }
      }
      else
      {
        //Uart_Init();
        cur_state = IDLE;
        PRINT_DEBUG_MSG("Uart receive byte ERROR.\n");  
      }
      break;
    }
    case GET_FRM_TO_BUF2:
    { 
      if(RECEIVE_BYTE_FROM_UART(&rec_data))
      {
        if(frm_data_cnt == MAX_FRAME_LEN-1)
        {
          USART_RX_DISABLE(); //串口接收禁止
          RDY_OUTPUT_LOW();   //发送空闲指示位：节点忙
          cur_state = IDLE;  
          uart_frm_rbuf2_vld = TRUE;
          uart_frm_rbuf2[frm_data_cnt] =  rec_data;   
          uart_frm_rbuf2_rxlen = MAX_FRAME_LEN;   
          frm_data_cnt= 0;
          TIM3_DISABLE();     //停止定时器
        }
        else if(frm_data_cnt > MAX_FRAME_LEN-1)
        {
           cur_state = IDLE;
           PRINT_DEBUG_MSG("Uart buff_2 OVERFLOW.\n");
        }
        else
        {
          uart_frm_rbuf2[frm_data_cnt] =  rec_data;     
          frm_data_cnt++;
          cur_state = GET_FRM_TO_BUF2; 
        }
      }
      else
      {
        //Uart_Init();
        cur_state = IDLE;
        PRINT_DEBUG_MSG("Uart receive byte ERROR.\n");
      }
      break;
    }
  }       
}

/**************************************************************************************************
 * @fn          TIM1_IRQHandler
 *
 * @brief       帧间间隔定时器中断服务函数,用于确定来自串口的数据该何时成帧.具体时间可在相应定时器
 *              初始化函数中配置.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
BSP_ISR_FUNCTION(TIM3_IRQHandler, TIM3_UPD_OVF_TRG_BRK_IRQn + 2)
{
  TIM3_RESET_COUNT(); //复位定时器计数
  
  uart_time_cnt++;
  if(uart_time_cnt==1)
  {
    cur_state = IDLE;  
    uart_time_cnt=0;
    
    if(!uart_frm_rbuf1_vld)
    {
      uart_frm_rbuf1_vld = TRUE;
      USART_RX_DISABLE(); //串口接收禁止
      RDY_OUTPUT_LOW();   //发送空闲指示位：节点忙
      uart_frm_rbuf1_rxlen = frm_data_cnt;
    }
    else if(!uart_frm_rbuf2_vld)
    {
      uart_frm_rbuf2_vld = TRUE;
      USART_RX_DISABLE(); //串口接收禁止
      RDY_OUTPUT_LOW();   //发送空闲指示位：节点忙
      uart_frm_rbuf2_rxlen = frm_data_cnt;
    }
    else
    {
      
    } 
    frm_data_cnt= 0;
    TIM3_DISABLE(); //关闭定时器
  }
  TIM3_CLEAN_UPDATE_INTERRUPT_FLAG(); //清除定时器更新中断标志
}

/**************************************************************************************************
 */
