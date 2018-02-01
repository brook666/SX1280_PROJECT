#include "bsp.h"
#include "stm32f10x.h" 
#include "usart_share_info.h"
#include "app_globals.h"
#include "usart2_dma_proc.h"

//uint8_t usart2_cnt = 0;

/***************************************************************************
 * @fn          USART2_IRQHandler
 *     
 * @brief       接收到一个Byte的数据后，产生串口中断。关闭串口中断，使能定时器.
 *     
 * @data        2015年08月05日
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


