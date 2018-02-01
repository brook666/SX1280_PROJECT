#include "stm32_reset.h"

/***************************************************************************
 * @fn          BSP_RESET_BOARD
 *     
 * @brief       窗口看门狗直接复位集中器
 *     
 * @data        2015年08月04日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 

void BSP_RESET_BOARD(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE); 
  WWDG_SetPrescaler(WWDG_Prescaler_1);
  WWDG_SetWindowValue(0xff);
  WWDG_Enable(0x40);
}

/***************************************************************************
 * @fn          IWDG_Config
 *     
 * @brief       硬件看门狗配置
 *     
 * @data        2015年08月04日
 *     
 * @param       time - 看门狗复位时间，单位s
 *     
 * @return      void
 ***************************************************************************
 */ 

void IWDG_Config(unsigned char time)
{
  unsigned int delay_time=0;
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  if(time>=26)   
    time=26;
  delay_time=time*157;
  IWDG_SetReload(delay_time);
  IWDG_ReloadCounter();
  IWDG_Enable();
}


