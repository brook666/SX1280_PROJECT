#include "stm32_rtc.h"
#include "delay.h"
#include "bsp_leds.h"
//#include "ed_tx_rx_buffer.h"
#include "bsp.h"
#include "ed_tx_fail_buffer_fun.h"
#include "nwk_QMgmt.h"

volatile uint8_t bsp_reset_ap_config = 0;

//volatile uint8_t sendBroadFlag = 0;
extern volatile uint8_t net_frm_cur_num;


//uint8_t bsp_reset_check_flag = 0;

extern ap_time_t  real_time; 
/*
extern uint16_t No_Commu_Sever_sec;
extern uint8_t Send_Verison_Frame_sec;*/
/***************************************************************************
 * @fn          RTC_IRQHandler
 *     
 * @brief       RTC�жϷ�������ÿ���ж�һ��
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void RTC_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_SEC))     //�����ж�
  {
		
    Time_Get(&real_time);             //����ʱ��   
    
//    sendBroadFlag = 1;
//		
//		No_Commu_Sever_sec++;		
//		Send_Verison_Frame_sec++;
    
    if((net_frm_cur_num == 0) && nwk_QfindAppFrame())
    {
      net_frm_cur_num++;
    }
//    if(app_frm_num == 0)
//    {
//      Find_ed_info_in_rx_buffer();
//    }
    
    if(tx_fail_frm_num == 0)
    {
      Find_ed_info_in_tx_fail_buffer();
    }
    if(start_emergency_dispatch_in_rtc)  //
    {
      static uint8_t emergency_count = 0 ;
      emergency_count++;
      if(emergency_count == 1)
      {
        emergency_count = 0;
        emergency_dispatch_flag = 1;
      }
    }
  }
  if(RTC_GetITStatus(RTC_IT_ALR))     //�����ж�
  {
    RTC_ClearITPendingBit(RTC_IT_ALR);  //�������ж�                              
  }
  RTC_ClearITPendingBit(RTC_IT_SEC);    //�������������жϱ�־                       
  RTC_WaitForLastTask();          //�ȴ�RTC�Ĵ����������
}







