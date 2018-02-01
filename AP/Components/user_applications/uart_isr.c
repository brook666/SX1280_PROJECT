/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   uart_isr.c
// Description: User APP.
//              ���ڽ����жϷ�����.���ڽ���֡������ʱ���жϷ�����.���ڻ�
//              ȡһ��������֡.
// Author:      Leidi
// Version:     1.0
// Date:        2014-9-26
// History:     2014-9-26   Leidi   ��ԭʼ����������������ļ�.
//                                  ���������.�޸ĵ�����Ϣ.
//                                  ʹ�ú궨��MAX_FRAME_LEN.
//              2014-11-10  Leidi   �Ƴ�ͷ�ļ�stm8s_uart.h.
//                                  ����������õ�ͷ�ļ�user_app_config.h.
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
#define MAX_FRAME_LEN  40 //������յ����֡��

/* ���ڽ���״̬����״̬���� */
#define IDLE                     0x00
#define GET_FRM_TO_BUF1          0x05
#define GET_FRM_TO_BUF2          0x06

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
/* ���ڽ��ջ����� */
uint8_t  uart_frm_rbuf1[MAX_FRAME_LEN]={0};
uint8_t  uart_frm_rbuf2[MAX_FRAME_LEN]={0};

/* ���ڽ��ջ�������Ч��־ */
uint8_t  uart_frm_rbuf1_vld = 0;
uint8_t  uart_frm_rbuf2_vld = 0;

/* ���ڽ��ջ�������Ч���� */
uint8_t  uart_frm_rbuf1_rxlen=0;
uint8_t  uart_frm_rbuf2_rxlen=0;

/* ------------------------------------------------------------------------------------------------
 *                                            Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static  uint8_t  uart_time_cnt;     //֡������ʱ���жϴ���
static  uint8_t  cur_state = IDLE;  //���ڽ���״̬��״̬
static  uint8_t  frm_data_cnt = 0;  //���ջ������������ֽ���


/**************************************************************************************************
 * @fn          USART1_RXISR
 *
 * @brief       ���ڽ����жϷ�����,���ڽ����ڽ��յ��������γ�һ��������֡,������ȫ�ֻ�������.ʹ��
 *              ˫�������.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
BSP_ISR_FUNCTION(USART1_RXISR, USART1_RX_IRQn + 2)
{
  uint8_t  rec_data;
  
  /* ������ʱ�� */
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
            ERR_OUTPUT_LOW(); //���ͳ���ָʾλ�������ڵ�����ź�
            uart_frm_rbuf1[frm_data_cnt++] = rec_data; 
            cur_state = GET_FRM_TO_BUF1;
          }
          else if(!uart_frm_rbuf2_vld)
          {
            ERR_OUTPUT_LOW(); //���ͳ���ָʾλ�������ڵ�����ź�
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
          USART_RX_DISABLE(); //���ڽ��ս�ֹ
          RDY_OUTPUT_LOW();   //���Ϳ���ָʾλ���ڵ�æ
          cur_state = IDLE;  
          uart_frm_rbuf1_vld = TRUE;
          uart_frm_rbuf1[frm_data_cnt] =  rec_data;   
          uart_frm_rbuf1_rxlen = MAX_FRAME_LEN;   
          frm_data_cnt= 0;
          TIM3_DISABLE();     //ֹͣ��ʱ��
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
          USART_RX_DISABLE(); //���ڽ��ս�ֹ
          RDY_OUTPUT_LOW();   //���Ϳ���ָʾλ���ڵ�æ
          cur_state = IDLE;  
          uart_frm_rbuf2_vld = TRUE;
          uart_frm_rbuf2[frm_data_cnt] =  rec_data;   
          uart_frm_rbuf2_rxlen = MAX_FRAME_LEN;   
          frm_data_cnt= 0;
          TIM3_DISABLE();     //ֹͣ��ʱ��
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
 * @brief       ֡������ʱ���жϷ�����,����ȷ�����Դ��ڵ����ݸú�ʱ��֡.����ʱ�������Ӧ��ʱ��
 *              ��ʼ������������.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
BSP_ISR_FUNCTION(TIM3_IRQHandler, TIM3_UPD_OVF_TRG_BRK_IRQn + 2)
{
  TIM3_RESET_COUNT(); //��λ��ʱ������
  
  uart_time_cnt++;
  if(uart_time_cnt==1)
  {
    cur_state = IDLE;  
    uart_time_cnt=0;
    
    if(!uart_frm_rbuf1_vld)
    {
      uart_frm_rbuf1_vld = TRUE;
      USART_RX_DISABLE(); //���ڽ��ս�ֹ
      RDY_OUTPUT_LOW();   //���Ϳ���ָʾλ���ڵ�æ
      uart_frm_rbuf1_rxlen = frm_data_cnt;
    }
    else if(!uart_frm_rbuf2_vld)
    {
      uart_frm_rbuf2_vld = TRUE;
      USART_RX_DISABLE(); //���ڽ��ս�ֹ
      RDY_OUTPUT_LOW();   //���Ϳ���ָʾλ���ڵ�æ
      uart_frm_rbuf2_rxlen = frm_data_cnt;
    }
    else
    {
      
    } 
    frm_data_cnt= 0;
    TIM3_DISABLE(); //�رն�ʱ��
  }
  TIM3_CLEAN_UPDATE_INTERRUPT_FLAG(); //�����ʱ�������жϱ�־
}

/**************************************************************************************************
 */
