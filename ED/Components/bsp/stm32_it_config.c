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
//	EXTI_InitTypeDef EXTI_InitStructure;
//	GPIO_InitTypeDef GPIO_InitStructure;  
  NVIC_InitTypeDef NVIC_InitStructure2;
	
	NVIC_SetVectorTable (NVIC_VectTab_FLASH, NVIC_VECTTAB_FLASH_OFFSET);
  
	/*RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);	  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;       
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;	 // ��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	     */
	
	
	/*GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1); 
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½����ж�
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure); 
	MRFI_CLEAR_IRQ_INT_FLAG();           */
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
      
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/*GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8); 
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½����ж�
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);   
      
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);   */ 
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;    
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 

  //TIM6_Configuration(ENABLE);
	
  // Configure one bit for preemption priority     //һ��λ��Ϊ��ռ���ȼ� ����Ϊ��һ��
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* ����P[A|B|C|D|E]0Ϊ�ж�Դ */
  NVIC_InitStructure2.NVIC_IRQChannel = EXTI15_10_IRQn;     //��10�ж�����
  NVIC_InitStructure2.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure2.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure2.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure2);
}




