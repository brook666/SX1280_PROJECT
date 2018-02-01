/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   stm32_flash.h
// Description: BSP.
//              STM32 ����������ͷ�ļ�.
// Author:      Leidi
// Version:     1.0
// Date:        2014-11-26
// History:     2014-11-26  Leidi ��ʼ�汾����.
*****************************************************************************/

#ifndef __STM32_BEEP_H
#define __STM32_BEEP_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "stm32f10x.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void BSP_BEEP_Init(void);
void BSP_TURN_ON_BEEP(void);
void BSP_TURN_OFF_BEEP(void);

#endif
/**************************************************************************************************
 */