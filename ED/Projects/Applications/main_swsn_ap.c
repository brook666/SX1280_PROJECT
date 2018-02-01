/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ed_buffer_fun.c
// Description: 网关主函数所在文件.
// Author:      Zengjia
// Version:     1.0
// Date:        2012-3-24
// History:     2014-10-21  Leidi 使用函数Iterates_tx_ed_buffer()遍历节点发送缓冲区.
//              2014-10-24  Leidi 删除全局变量resvr_pkg[MAX_RESVR_PKG]和resvr_pkg_num.
//              2014-12-02  Leidi 让蜂鸣器响一下表示系统启动完成.
*****************************************************************************/
#include <string.h>
#include <stdio.h>
#include "bsp.h"
#include "bsp_leds.h"
#include "stm32_usart2_gtway.h"
#include "nwk_layer.h"
#include "net_frm_proc.h"
#include "net_status_fun.h"
#include "net_list_proc.h"
#include "uart_frm_proc.h"
#include "ap_inner_fun.h"
#include "delay.h"
#include "net_freq_agility.h"     
#include "stm32_reset.h"
#include "firmware_info_proc.h"
#include "67_frm_proc.h"
#include "ed_tx_rx_buffer.h"
#include "wifi_module.h"
#include "user_def_fun.h"
#include "usart2_dma_proc.h"
#include "usart1_dma_proc.h"
#include "usart_share_info.h"
#include "stm32_rtc.h"
#include "mrfi.h"
#include "Sx1280.h"


#ifndef APP_AUTO_ACK
#error ERROR: Must define the macro APP_AUTO_ACK for this application.
#endif

globalflag_t     globalFlag = {0};

// extern  globle extern
extern volatile uint8_t net_frm_cur_num;
extern volatile uint8_t net_ed_add_sem;
extern volatile uint8_t bro_cast_sem;
extern volatile uint8_t bsp_reset_ap_config ;
extern volatile uint8_t net_ed_num;

extern  ap_time_t  real_time; 
extern uint8_t bsp_reset_check_flag ;
extern int32_t remoter_press_num ;

mrfiPacket_t  myPacket;
uint32_t band_rate=115200;
uint32_t ResetTimes=0;

uint8_t array[13] = {0x0C,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B};

int main (void)
{  
#ifdef SEGGER_DEBUG
  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"POWER ON!"RTT_CTRL_RESET"\n");
#endif  
  
  BSP_Init();

  
#ifdef IWDG_START 
  IWDG_Config(40);
#endif  
  MRFI_Init();
  
  MRFI_RxOn(); //开启接收

	
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"System running..."RTT_CTRL_RESET"\n");
#endif



	
  while(1)
  {  
		
  	memcpy(myPacket.frame,array,13);		

//    MRFI_Transmit(&myPacket, MRFI_TX_TYPE_CCA);
		
		delay_ms(5000);
		
    if(bsp_reset_ap_config)
    {      
      bsp_reset_ap_config=0;  
//    restoreFactorySettings();                  
      BSP_RESET_BOARD(); 
    }   
    
#ifdef FREQUENCY_AGILITY 
    checkChangeChannel();
#endif
#ifdef IWDG_START 
    IWDG_ReloadCounter();  
#endif    
 }
}

