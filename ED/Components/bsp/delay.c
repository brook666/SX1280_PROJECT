#include "delay.h"
#include "stm32_reset.h"
#include "smpl_config.h"

static uint32_t ms_value,us_value; 

/***************************************************************************
 * @fn          delay_init
 *     
 * @brief       ��ʼ����શ�ʱ��
 *     
 * @data        2015��08��04��
 *     
 * @param       SYSCLK-��ʱ��Ƶ��(����ʱ��Ƶ��Ϊ72MHz����SYSCLK = 72)
 *     
 * @return      void
 ***************************************************************************
 */ 

void delay_init(u8 SYSCLK)
{
  SysTick->CTRL&=0xFFFFFFF8;//ѡ���ⲿʼ�� HCLK/8
  us_value=SYSCLK/8;
  ms_value=us_value*1000;
}


/***************************************************************************
 * @fn          delay_s
 *     
 * @brief       ��ʱn��
 *     
 * @data        2015��08��04��
 *     
 * @param       ns - n��
 *     
 * @return      void
 ***************************************************************************
 */ 

void delay_s(u32 ns)
{
  int i;
  for(i=0;i<ns;i++)
  {
    delay_ms(1000);
#ifdef IWDG_START 
    IWDG_ReloadCounter();  
#endif
  }
}

/***************************************************************************
 * @fn          delay_ms
 *     
 * @brief       n ms��ʱ
 *     
 * @data        2015��08��04��
 *     
 * @param       nms-��Ҫ��ʱ�ĺ�����
 *     
 * @return      void
 ***************************************************************************
 */ 

void delay_ms(u32 nms)
{
  uint32_t temp;  
  SysTick->LOAD = (uint32_t)nms*ms_value;  
  SysTick->VAL = 0x00;  
  SysTick->CTRL |= 0x01;  
  SysTick->CTRL |= 0x01;  
  do  
  {  
    temp = SysTick->CTRL;  
  }  
  while(temp&0x01&&!(temp&(1<<16)));  
  SysTick->CTRL &= 0xFFFFFFF8;  
  SysTick->VAL = 0x00; 
}

/***************************************************************************
 * @fn          delay_us
 *     
 * @brief       ��ʱn΢��
 *     
 * @data        2015��08��04��
 *     
 * @param       nus - n΢��
 *     
 * @return      void
 ***************************************************************************
 */ 

void delay_us(u32 nus)
{
  uint32_t temp;  
  SysTick->LOAD = (uint32_t)nus*us_value;  
  SysTick->VAL = 0x00;  
  SysTick->CTRL |= 0x01;  
  do  
  {  
      temp = SysTick->CTRL;  
  }  
  while(temp&0x01&&!(temp&(1<<16)));  
  SysTick->CTRL &= 0xFFFFFFF8;  
  SysTick->VAL = 0x00;
}




