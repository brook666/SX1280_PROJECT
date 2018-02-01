#include "stm32_rtc.h"
#include "delay.h"
#include "bsp_leds.h"
#include "ed_tx_rx_buffer.h"
#include "bsp.h"

volatile uint8_t bsp_reset_ap_config = 0;

extern volatile uint8_t sendBroadFlag ;
extern volatile uint8_t net_frm_cur_num;


uint8_t bsp_reset_check_flag = 0;

extern ap_time_t  real_time; 
/*extern uint16_t No_Commu_Sever_sec;
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
//		
//   Time_Get(&real_time);             //����ʱ��   
//    
//   sendBroadFlag = 1;
//		
////		No_Commu_Sever_sec++;		
////		Send_Verison_Frame_sec++;
//    
//    if((net_frm_cur_num == 0) && nwk_QfindAppFrame())
//    {
//      net_frm_cur_num++;
//    }
//    if(app_frm_num == 0)
//    {
//      Find_ed_info_in_rx_buffer();
//    }
//    
//    if(tx_fail_frm_num == 0)
//    {
//      Find_ed_info_in_tx_fail_buffer();
//    }
//    if(start_emergency_dispatch_in_rtc)
//    {
//      static uint8_t emergency_count = 0 ;
//      emergency_count++;
//      if(emergency_count == 1)
//      {
//        emergency_count = 0;
//        emergency_dispatch_flag = 1;
//      }
//    }
  }
  if(RTC_GetITStatus(RTC_IT_ALR))     //�����ж�
  {
    RTC_ClearITPendingBit(RTC_IT_ALR);  //�������ж�                              
  }
  RTC_ClearITPendingBit(RTC_IT_SEC);    //�������������жϱ�־                       
  RTC_WaitForLastTask();          //�ȴ�RTC�Ĵ����������
}


/***************************************************************************
 * @fn          EXTI9_5_IRQHandler
 *     
 * @brief       ���ü����������ͻָ��������������WIFIģ��
 *     
 * @data        2015��08��04��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 

void EXTI9_5_IRQHandler(void)
{
	uint8_t i,count;
	while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == RESET)
  {
    if(count == 30)
    {
      for(i=0;i<10;i++)
      {
        delay_ms(100);
      }
			bsp_reset_ap_config = 1;
    }
    delay_ms(100);
    count++;
#ifdef IWDG_START 
		IWDG_ReloadCounter();  
#endif		
  }
  EXTI_ClearITPendingBit(EXTI_Line8);
}







