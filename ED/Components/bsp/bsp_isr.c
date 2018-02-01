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
 * @brief       RTC中断服务函数，每秒中断一次
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void RTC_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_SEC))     //秒钟中断
  {
		
    Time_Get(&real_time);             //更新时间   
    
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
  if(RTC_GetITStatus(RTC_IT_ALR))     //闹钟中断
  {
    RTC_ClearITPendingBit(RTC_IT_ALR);  //清闹钟中断                              
  }
  RTC_ClearITPendingBit(RTC_IT_SEC);    //清除溢出，秒钟中断标志                       
  RTC_WaitForLastTask();          //等待RTC寄存器操作完成
}







