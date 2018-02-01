#include "bsp.h"
#include "mrfi_board_defs.h"
#include "stm32_it_config.h" 
#include "stm32_usart2_gtway.h"
#include "stm32f10x.h"

/***************************************************************************
 * @fn          NVIC_Configuration
 *     
 * @brief       ����������ж�����
 *     
 * @data        2015��08��04��
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
	
  // Configure one bit for preemption priority     //һ��λ��Ϊ��ռ���ȼ� ����Ϊ��һ��
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
  
  /* ����P[A|B|C|D|E]0Ϊ�ж�Դ */
  NVIC_InitStructure2.NVIC_IRQChannel = EXTI15_10_IRQn;     //��10�ж�����
  NVIC_InitStructure2.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure2.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure2.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure2);
}




