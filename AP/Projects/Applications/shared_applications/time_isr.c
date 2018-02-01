/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   time_isr.c
// Description: 定时器中断服务函数所在文件.
// Author:      Leidi
// Version:     1.1
// Date:        2014-10-24
// History:     2014-10-24  Leidi 删除TIM4中有关定时清空0x69帧缓冲区的代码.
*****************************************************************************/

#include"bsp.h"
#include <string.h>
#include "nwk_layer.h"
#include "bui_pkg_fun.h"
#include "net_frm_proc.h"
#include "68_frm_proc.h"

extern volatile uint8_t reply_remoter_time_cnt;
extern volatile uint8_t remoter_frame_flag ;
/***************************************************************************
 * @fn          TIM6_IRQHandler
 *     
 * @brief       TIM6中断，100ms中断一次。用于处理遥控器事件，程序中每收到
 *              一次遥控器帧，会自动将计数器清零.转发遥控器信息，但是双击
 *              关键和三击关键信息不转发也不处理。
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void TIM6_IRQHandler(void)
{
	remoter_frame_flag = 1;
	TIM_ClearITPendingBit(TIM6 , TIM_FLAG_Update); 
}



