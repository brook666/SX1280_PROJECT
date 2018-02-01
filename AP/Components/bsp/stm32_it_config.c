#include "bsp.h"
#include "mrfi_board_defs.h"
#include "stm32_it_config.h" 
#include "stm32_usart2_gtway.h"
#include "stm32f10x.h"

/***************************************************************************
 * @fn          NVIC_Configuration
 *     
 * @brief       所有外设的中断配置
 *     
 * @data        2015年08月04日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 	 
  NVIC_InitTypeDef NVIC_InitStructure2;
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;    
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
	
  // Configure one bit for preemption priority     //一个位作为抢占优先级 设置为第一组
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
  
  /* 配置P[A|B|C|D|E]0为中断源 */
  NVIC_InitStructure2.NVIC_IRQChannel = EXTI15_10_IRQn;     //线10中断向量
  NVIC_InitStructure2.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure2.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure2.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure2);
}




