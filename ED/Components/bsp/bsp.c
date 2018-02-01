/**************************************************************************************************
  Revised:        $Date: 2007-07-06 11:19:00 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

**************************************************************************************************/

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *   BSP (Board Support Package)
 *   Top-level BSP code file.
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"
#include "bsp_leds.h"
#include "delay.h"
#include "stm32_reset.h"
#include "stm32_rtc.h"
#include "usart2_dma_proc.h"

/***************************************************************************
 * @fn          TIM1_NVIC_Configuration
 *     
 * @brief       TIM1�ж����ò�ʹ���жϡ�
 *              TIM1�ж���Ϊ��3�飬��ռ���ȼ�4����Ӧ���ȼ�0
 *     
 * @data        2015��08��04��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 

void TIM1_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	NVIC_SetVectorTable (NVIC_VectTab_FLASH, NVIC_VECTTAB_FLASH_OFFSET);
                     
  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;    
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


/***************************************************************************
 * @fn          TIM1_Configuration
 *     
 * @brief       TIM1�������ã��ж�����1s
 *     
 * @data        2015��08��04��
 *     
 * @param       NewState - ENABLE/DISABLEʹ�ܻ�ر�TIM1
 *     
 * @return      void
 ***************************************************************************
 */ 

void TIM1_Configuration(FunctionalState NewState)
{   
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);
  TIM_DeInit(TIM1);
  TIM_TimeBaseStructure.TIM_Period=5000;          /* �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
  TIM_TimeBaseStructure.TIM_Prescaler= (7200 - 1);            /* ʱ��Ԥ��Ƶ�� 72M/72 */
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;     /* ������Ƶ */
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* ���ϼ���ģʽ */
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 1;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  TIM_ClearFlag(TIM1, TIM_FLAG_Update);                     /* �������жϱ�־ */
  TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
  TIM_Cmd(TIM1, NewState);
  TIM1_NVIC_Configuration();    
}



/***************************************************************************
 * @fn          TIM5_NVIC_Configuration
 *     
 * @brief       TIM5�ж����ò�ʹ���жϡ�
 *              TIM5�ж���Ϊ��3�飬��ռ���ȼ�4����Ӧ���ȼ�1
 *     
 * @data        2017��04��17��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 

void TIM5_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	NVIC_SetVectorTable (NVIC_VectTab_FLASH, NVIC_VECTTAB_FLASH_OFFSET);                     
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;    
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/***************************************************************************
 * @fn          TIM5_Configuration
 *     
 * @brief       TIM5��������   ����0.5ms
 *     
 * @data        2015��08��04��
 *     
 * @return      void
 ***************************************************************************
 */ 

void TIM5_Configuration(void)
{   
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5 , ENABLE);
  TIM_DeInit(TIM5);
  TIM_TimeBaseStructure.TIM_Period=5000;          /* �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
  TIM_TimeBaseStructure.TIM_Prescaler= (7200 - 1);            /* ʱ��Ԥ��Ƶ�� 72M/7200 */
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;     /* ������Ƶ */
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* ���ϼ���ģʽ */
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  TIM_ClearFlag(TIM5, TIM_FLAG_Update);                     /* �������жϱ�־ */
  TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE);
  TIM_Cmd(TIM5, ENABLE);
   
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5 , ENABLE);		 /*����*/    
}

/**************************************************************************************************
 * @fn          EXIT_PC10_Config
 *
 * @brief       Initialize EXIT.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void EXIT_PC10_Config(void)
{
   EXTI_InitTypeDef EXTI_InitStructure;
	
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	 NVIC_Configuration();
	
	 /* EXTI line gpio config(PC10) */	

	/* EXTI line(PC10) mode config */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource10);      //C10 ����ΪEXIT������
  EXTI_InitStructure.EXTI_Line = EXTI_Line10;                       //��10
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;              //�ж�ģʽ
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;           //�����ش����ж�
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);     
}

/***************************************************************************
 * @fn          TIM6_NVIC_Configuration
 *     
 * @brief       TIM6�ж����ò�ʹ���жϡ�
 *              TIM6�ж���Ϊ��3�飬��ռ���ȼ�4����Ӧ���ȼ�1
 *     
 * @data        2015��08��04��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 

void TIM6_NVIC_Configuration(FunctionalState NewState)
{
  NVIC_InitTypeDef NVIC_InitStructure;    
	
	NVIC_SetVectorTable (NVIC_VectTab_FLASH, NVIC_VECTTAB_FLASH_OFFSET);
	
  NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;   
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
  NVIC_Init(&NVIC_InitStructure);
}

/***************************************************************************
 * @fn          TIM6_Configuration
 *     
 * @brief       TIM6�������ã��ж�����100ms
 *     
 * @data        2015��08��04��
 *     
 * @param       NewState - ENABLE/DISABLEʹ�ܻ�ر�TIM6
 *     
 * @return      void
 ***************************************************************************
 */ 
void TIM6_Configuration(FunctionalState NewState)
{   
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6 , ENABLE);
  TIM_DeInit(TIM6);
  TIM_TimeBaseStructure.TIM_Period=3000;          /* �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ) */
  /* �ۼ� TIM_Period��Ƶ�ʺ����һ�����»����ж� */
  TIM_TimeBaseStructure.TIM_Prescaler= (7200 - 1);  
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;     /* ������Ƶ */
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; /* ���ϼ���ģʽ */
  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
  TIM_ClearFlag(TIM6, TIM_FLAG_Update);                     /* �������жϱ�־ */
  TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE);
  TIM_Cmd(TIM6, NewState);
  TIM6_NVIC_Configuration(NewState);    
}



/*******************************end**********************/



/**************************************************************************************************
 * @fn          BSP_Init
 *
 * @brief       Initialize the board and drivers.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void BSP_Init(void)
{
  BSP_INIT_BOARD();
  BSP_BEEP_Init();

//  bsp_initleds();
	
  delay_init(72);
  Init_RTC();
	
	Usart2_Configuration(115200,ENABLE);
	USART2_NVIC_Configuration(ENABLE);
	Usart2_DMA_Configuration();
	
	Usart1_Configuration(115200,ENABLE);
	USART1_NVIC_Configuration(ENABLE);
	Usart1_DMA_Configuration();
  /*-------------------------------------------------------------
   *  Run time integrity checks.  Perform only if asserts
   *  are enabled.
   */
#ifdef BSP_ASSERTS_ARE_ON
  /* verify endianess is correctly specified */
  {
    uint16_t test = 0x00AA; /* first storage byte of 'test' is non-zero for little endian */
    BSP_ASSERT(!(*((uint8_t *)&test)) == !BSP_LITTLE_ENDIAN); /* endianess mismatch */
  }
#endif
		Sx1280_GPIO_Inital();
}


/* ================================================================================================
 *                                        C Code Includes
 * ================================================================================================
 */
#ifdef BSP_BOARD_C
#include BSP_BOARD_C
#endif


/* ************************************************************************************************
 *                                   Compile Time Integrity Checks
 * ************************************************************************************************
 */
BSP_STATIC_ASSERT( sizeof(  uint8_t ) == 1 );
BSP_STATIC_ASSERT( sizeof(   int8_t ) == 1 );
BSP_STATIC_ASSERT( sizeof( uint16_t ) == 2 );
BSP_STATIC_ASSERT( sizeof(  int16_t ) == 2 );
BSP_STATIC_ASSERT( sizeof( uint32_t ) == 4 );
BSP_STATIC_ASSERT( sizeof(  int32_t ) == 4 );



/**************************************************************************************************
 */
