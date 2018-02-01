/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   main_swsn_ed.c
// Description: 主函数所在源文件.
// Author:      Leidi
// Version:     1.2
// Date:        2012-3-24
*****************************************************************************/

#include "swsn_lib.h"
#include "mrfi_board_defs.h"
#include "usart2_dma_proc.h"
#include "stm32_usart2_gtway.h"
#include "swsn_proc.h"
#include "ed_tx_fail_buffer_fun.h"

extern volatile uint8_t bsp_reset_ap_config ;
extern volatile uint8_t reset_system_flag;
uint8_t hex_data[10]={1,2,3,4,5,6,7,8,9,0};

globalflag_t     globalFlag = {0};

/**************************************************************************************************
 * @fn          main
 *
 * @brief       节点模块的主函数.
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
int main (void) 
{
 //版级资源初始化
  BSP_Init();   

#ifdef IWDG_START 
  IWDG_Config(20);
#endif 
	
  swsn_info_init();   
  put_multi_hex_uart2(hex_data, 10);
//registerAppFuncDomain(0x80,ed_intra_proc);
	
  while(1)
  {  
    if(reset_system_flag)
    {
      reset_system_flag = 0;
      BSP_RESET_BOARD(); 
    }
     swsn_app_proc();
		 Iterates_usart2_buffer();
		 Iterates_usart1_buffer();
		 Iterates_tx_fail_ed_buffer(); //处理发送失败的数据
#ifdef IWDG_START 
    IWDG_ReloadCounter();  
#endif 
		
  }
  
}




