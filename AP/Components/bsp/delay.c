#include "delay.h"
#include "stm32_reset.h"
#include "smpl_config.h"

static uint32_t ms_value,us_value; 

/***************************************************************************
 * @fn          delay_init
 *     
 * @brief       初始化嘀嗒定时器
 *     
 * @data        2015年08月04日
 *     
 * @param       SYSCLK-主时钟频率(如主时钟频率为72MHz，则SYSCLK = 72)
 *     
 * @return      void
 ***************************************************************************
 */ 

void delay_init(u8 SYSCLK)
{
  SysTick->CTRL&=0xFFFFFFF8;//选择外部始终 HCLK/8
  us_value=SYSCLK/8;
  ms_value=us_value*1000;
}


/***************************************************************************
 * @fn          delay_s
 *     
 * @brief       定时n秒
 *     
 * @data        2015年08月04日
 *     
 * @param       ns - n秒
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
 * @brief       n ms定时
 *     
 * @data        2015年08月04日
 *     
 * @param       nms-需要定时的毫秒数
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
 * @brief       定时n微秒
 *     
 * @data        2015年08月04日
 *     
 * @param       nus - n微秒
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




