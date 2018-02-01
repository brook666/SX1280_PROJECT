#include "bsp.h"
#include "stm32f10x.h" 
#include "usart_share_info.h"
#include "app_globals.h"
#include "usart2_dma_proc.h"
#include "usart1_dma_proc.h"
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

#ifdef TIME_COUNT	
  uint16_t test_arry[1]; 
  uint8_t time_info[2];				
	test_arry[0]=TIM_GetCounter(TIM6);
	time_info[0]=((test_arry[0]>>8));
	time_info[1]=((test_arry[0]));
#ifdef SEGGER_DEBUG
 SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"usart2 INTERRUPT HAPPENED"RTT_CTRL_RESET"\n");
	SEGGER_RTT_put_multi_char((uint8_t *)(time_info),2);
#endif 
#endif
  globalFlag.uart2_dma_flag = 1;
  USART2_NVIC_Configuration(DISABLE);  
  TIM4_Configuration();
}


void USART1_IRQHandler() 
{
  globalFlag.uart1_dma_flag = 1;
  USART1_NVIC_Configuration(DISABLE); 
	
  TIM2_Configuration();
}

