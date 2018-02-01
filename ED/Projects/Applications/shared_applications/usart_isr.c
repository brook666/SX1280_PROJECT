#include "bsp.h"
#include "stm32f10x.h" 
#include "usart_share_info.h"
#include "app_globals.h"
#include "usart2_dma_proc.h"

//uint8_t usart2_cnt = 0;

/***************************************************************************
 * @fn          USART2_IRQHandler
 *     
 * @brief       ���յ�һ��Byte�����ݺ󣬲��������жϡ��رմ����жϣ�ʹ�ܶ�ʱ��.
 *     
 * @data        2015��08��05��
 *     
 * @param       
 *     
 * @return      
 ***************************************************************************
 */ 

void USART2_IRQHandler() 
{

  globalFlag.uart2_dma_flag = 1;
  USART2_NVIC_Configuration(DISABLE);  
  TIM4_Configuration();
}


