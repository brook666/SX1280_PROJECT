/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   time_isr.c
// Description: ��ʱ���жϷ����������ļ�.
// Author:      Leidi
// Version:     1.1
// Date:        2014-10-24
// History:     2014-10-24  Leidi ɾ��TIM4���йض�ʱ���0x69֡�������Ĵ���.
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
 * @brief       TIM6�жϣ�100ms�ж�һ�Ρ����ڴ���ң�����¼���������ÿ�յ�
 *              һ��ң����֡�����Զ�������������.ת��ң������Ϣ������˫��
 *              �ؼ��������ؼ���Ϣ��ת��Ҳ������
 *     
 * @data        2015��08��05��
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



