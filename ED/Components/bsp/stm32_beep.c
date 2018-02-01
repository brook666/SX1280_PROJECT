/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   stm32_beep.c
// Description: BSP.
//              STM32 蜂鸣器驱动源文件.
// Author:      Leidi
// Version:     1.0
// Date:        2014-11-26
// History:     2014-11-26  Leidi 初始版本建立.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "stm32_beep.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define RCC_GPIO_BEEP       RCC_APB2Periph_GPIOC  //蜂鸣器使用的GPIO外设
#define GPIO_BEEP_PORT      GPIOC                 //蜂鸣器使用的GPIO组
#define GPIO_BEEP_PIN       GPIO_Pin_1            //蜂鸣器使用的GPIO管脚
#define GPIO_BEEP_PIN_ON    Bit_SET               //开启蜂鸣器时的GPIO状态
#define GPIO_BEEP_PIN_OFF   Bit_RESET             //关闭蜂鸣器时的GPIO状态

/**************************************************************************************************
 * @fn          BSP_BEEP_Init
 *
 * @brief       初始化蜂鸣器使用的GPIO.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void BSP_BEEP_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_GPIO_BEEP, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_BEEP_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIO_BEEP_PORT, &GPIO_InitStructure);
}

/**************************************************************************************************
 * @fn          BSP_TURN_ON_BEEP
 *
 * @brief       开启蜂鸣器.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void BSP_TURN_ON_BEEP(void)
{
  GPIO_WriteBit(GPIO_BEEP_PORT, GPIO_BEEP_PIN, GPIO_BEEP_PIN_ON);
}

/**************************************************************************************************
 * @fn          BSP_TURN_OFF_BEEP
 *
 * @brief       关闭蜂鸣器.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void BSP_TURN_OFF_BEEP(void)
{
  GPIO_WriteBit(GPIO_BEEP_PORT, GPIO_BEEP_PIN, GPIO_BEEP_PIN_OFF);
}
/**************************************************************************************************
 */
 
