/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   gtway_frm_proc.h
// Description: 构建网外通信帧.
// Author:      Leidi
// Version:     1.0
// Date:        2014-3-24
// History:     2014-10-24  Leidi (1) 在函数proc_net_msg()中,由节点上发的帧原先由
//                                    函数rebuild_69_frm()尝试封装为非广播的网外通
//                                    信帧.现在改为由Bui_bcast_gtway_msg_from_inner_msg()
//                                    一律封装为广播的网外通信帧.
//                                (2) 在函数proc_net_msg()中,使用Send_gtway_msg_to_uart()
//                                    将需要输出的帧发往正确的串口，代替原先的代码.
//                                (3) 在函数proc_net_msg()中,ap_inner_proc()调用后,
//                                    将产生输出.
//              2014-11-5   Leidi  修改函数net_frm_proc()防止无法处理完所有的接收帧.
*****************************************************************************/ 

#include <string.h>
#include "bsp.h"
#include "bsp_leds.h"
#include "mrfi.h"
#include "mrfi_si4438_api.h"
#include "mrfi_board_defs.h"
#include "nwk_layer.h"
#include "mrfi_spi.h"
#include "net_frm_proc.h"    
#include "bui_pkg_fun.h"      
#include "net_list_proc.h  "  
#include "net_para_fun.h   "  
#include "net_status_fun.h "  
#include "68_frm_proc.h"
#include "ap_inner_fun.h"
#include "core_cm3.h"
#include "gtway_frm_proc.h"
#include "user_def_fun.h"    
#include "ed_tx_rx_buffer.h"


//#define MISSES_IN_A_ROW 3


volatile uint8_t net_frm_cur_num = 0;
volatile uint8_t net_ed_add_sem  = 0;
volatile uint8_t bro_cast_sem    = 0;
volatile net_frm_back_t net_frm_back = {0};
volatile uint8_t net_ed_num=0;
volatile uint8_t remoter_frame_flag = 0;
extern ap_time_t  real_time;
//extern uint8_t sendBroadFlag ;

 uint8_t sendBroadFlag ;
/* Runs in ISR context. Reading the frame should be done in the */
/* application thread not in the ISR thread. */
/***************************************************************************
 * @fn          sCB 
 *     
 * @brief       回调函数，中断中运行，产生帧指示标识。帧的读取在应用层进行。
 *     
 * @data        2015年08月07日
 *     
 * @param       lid - 节点ID号
 *     
 * @return      0
 ***************************************************************************
 */ 
uint8_t sCB(linkID_t lid,uint8_t frm_num)
{
  if (lid)
  {
    if(lid == SMPL_LINKID_USER_UUD)
    {
      bro_cast_sem++;
    }
    else
    {
      net_frm_back.isBack = 1;
      net_frm_back.lid = lid;
//			net_frm_back.frm_count=frm_num;
      net_frm_back.frm_count=Write_rx_ed_buffer(lid);
    }
  }
  else
  {
    net_ed_add_sem++;
  }

  return 0;  
}



/***************************************************************************
 * @fn          net_ed_link
 *     
 * @brief       集中器link监听。3次调用SMPL_LinkListen监听已经加入网络的节点，
 *              如果3次过后仍没有节点进行link请求，则将该节点的CONNSTATE_JOINED
 *              状态，变为CONNSTATE_FREE状态，该存储空间预留给其它节点使用。
 *     
 * @data        2015年08月07日
 *     
 * @param       void
 *     
 * @return      1 , 监听成功
 *              0 , 监听失败
 ***************************************************************************
 */ 
uint8_t net_ed_link(void)
{   
#ifdef DEBUG
  put_string_uart1("net_ed_link!!!"); 
#endif
	
  bspIState_t intState;
  if(net_ed_add_sem && (net_ed_num < NUM_CONNECTIONS+1))
  {
/* listen for a new connection */
    uint8_t k=4;
    linkID_t slid;
    while (1)
    { 
      k--;
      if (SWSN_LinkListen(&slid))
      {
        if(slid>net_ed_num)
           net_ed_num++;
        
        add_net_list(slid);
				nwk_resetUnlinkedED();
#ifdef DEBUG
        put_string_uart1("link_listen sucess!!!"); 
        print_all_ed_info(head_list);
#endif
        BSP_ENTER_CRITICAL_SECTION(intState);
        net_ed_add_sem--;
        BSP_EXIT_CRITICAL_SECTION(intState);
        return 1;
      }
      else if(k==0) 
      {
        nwk_resetUnlinkedED();
        
        BSP_ENTER_CRITICAL_SECTION(intState);
        net_ed_add_sem--;
        BSP_EXIT_CRITICAL_SECTION(intState);
        break;
      }
     }
   }
   return 0;
}

/***************************************************************************
 * @fn          net_frm_proc
 *     
 * @brief       读取网络帧。
 *     
 * @data        2015年08月07日
 *     
 * @param       void
 *     
 * @return      1 , 成功
 *              0 , 失败
 ***************************************************************************
 */ 
uint8_t net_frm_proc(void)
{
  uint8_t rc = 0;
  uint8_t  msg[MAX_APP_PAYLOAD], len;
  uint8_t i=0;
  bspIState_t intState;
#ifdef DEBUG
  put_string_uart1("process_net_frm!!!");
#endif
  
  //if(SMPL_QFIND_APP_FRAME())
  if(net_frm_cur_num)
  {
  /* process all frames waiting */
    for(i=1; i<net_ed_num+1; i++)
//    for(i=sPersistInfo.nextLinkID-1; i>0; i--)
    {       
      rc= recv_user_msg(i, msg, &len); 
      if(1 ==rc )
      {
#ifdef SEGGER_DEBUG1 
        SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"APP packet:"RTT_CTRL_RESET"\n");        
        SEGGER_RTT_put_multi_char(msg,len); 
#endif 
		
        proc_net_msg(i, msg, len);  //底层的68帧
        BSP_ENTER_CRITICAL_SECTION(intState);
        net_frm_cur_num--;
        BSP_EXIT_CRITICAL_SECTION(intState);
        return 1;
      }
    }    
  }
  BSP_ENTER_CRITICAL_SECTION(intState);
  net_frm_cur_num--;
  BSP_EXIT_CRITICAL_SECTION(intState);
  
  return 0;    
    
}

/***************************************************************************
 * @fn          proc_net_msg
 *     
 * @brief       网络帧处理。网络帧根据控制码的不同，comm_type=0x00或者0x01
 *              则透传给串口；comm_type == 0x04,则是集中器内部需要处理的帧,
 *              部分数据需要透传给串口。
 *     
 * @data        2015年08月07日
 *     
 * @param       lid  - 节点ID号
 *              msg  - 帧字节数组
 *              len  - 帧长度
 *     
 * @return      void
 ***************************************************************************
 */ 
void proc_net_msg(linkID_t lid, uint8_t *msg, uint8_t len)
{

 uint8_t comm_type;
 uint8_t gtway_out_msg[300];
 uint8_t gtway_out_msg_len;
#ifdef DEBUG
  put_string_uart1("received message from  ");
  put_char_uart1(lid);
  put_string_uart1(":   \n");
#endif
/********首先，对ED发来的应答帧进行检测！********/
  memset(gtway_out_msg, 0x00, sizeof(gtway_out_msg));
  if(*msg==0x68&&*(msg+len-1)==0x16)
  {
    app_pkg_t in_pkg;
    bui_app_pkg(msg,&in_pkg);
/*告警帧*/
    comm_type = in_pkg.ctr_code &0x0c;
      
    if((comm_type == 0x00) || (comm_type == 0x01))
    {
      /* 将节点上发的内部帧打包为网外通信帧 */
      gtway_out_msg_len = Bui_bcast_gtway_msg_from_inner_msg(msg, len, gtway_out_msg);           
#ifdef DEBUG
      put_multi_char_uart1(gtway_out_msg, gtway_out_msg_len);
#else
      /* 将帧发往正确的串口*/
      Send_gtway_msg_to_uart(gtway_out_msg, gtway_out_msg_len);
#endif  
    }
    if(comm_type == 0x04)//节点访问集中器，不透传(内部处理)
    {
      app_pkg_t in_pkg;
      bui_app_pkg(msg,&in_pkg);
      
      /* AP内部处理 */
      ap_inner_proc(&in_pkg, msg, &len);
      if(len!=0)
      {
        /* 将节点上发的内部帧打包为网外通信帧 */
        gtway_out_msg_len = Bui_bcast_gtway_msg_from_inner_msg(msg, len, gtway_out_msg);
              
#ifdef DEBUG
        put_multi_char_uart1(gtway_out_msg, gtway_out_msg_len);
#else
        /* 将帧发往正确的串口. */
        Send_gtway_msg_to_uart(gtway_out_msg, gtway_out_msg_len);
#endif 
      }
    }
    else if(comm_type == 0x0C)
    {
      /* 将节点上发的内部帧打包为网外通信帧 */
      gtway_out_msg_len = Bui_bcast_gtway_msg_from_inner_msg(msg, len, gtway_out_msg);           
#ifdef DEBUG
      put_multi_char_uart1(gtway_out_msg, gtway_out_msg_len);
#else
      /* 将帧发往正确的串口*/
      Send_gtway_msg_to_uart(gtway_out_msg, gtway_out_msg_len);
#endif  
    }		
    else
    {
    }     
  }   
   
  return;

}

/***************************************************************************
 * @fn          proc_bcast_frm(暂时没用，节点不会发送广播帧)
 *     
 * @brief       处理广播帧。
 *     
 * @data        2015年08月07日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void proc_bcast_frm(void)
{
  if(bro_cast_sem)
  {
    uint8_t bcast_msg[MAX_APP_PAYLOAD];
    uint8_t bcast_msg_len;
    
#ifdef DEBUG 
    put_string_uart1("Receive the BCast frame !");
#endif
    
    if( recv_bcast_msg(bcast_msg, &bcast_msg_len))
    {
#ifdef DEBUG  
      put_string_uart1("Receive success!");
#endif
      
      if((bcast_msg[0] == 0x68) && (bcast_msg[bcast_msg_len - 1] == 0x16))
      {
        proc_net_msg(SMPL_LINKID_USER_UUD, bcast_msg, bcast_msg_len);
      }
    }
    bro_cast_sem--;
  }
}

/***************************************************************************
 * @fn          send_broad_time_sync
 *     
 * @brief       发送同步帧并检测射频故障。每3个时间周期检测一次，
 *              (1)发射不成功，接收没有变化，重启射频
 *              (2)射频GPIO状态不正常，重启射频
 *              (3)发射除广播帧以外的数据，但是没有接收数据，重启射频。
 *     
 * @data        2015年08月07日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 

void send_broad_time_sync(void)
{
  uint8_t msg[50];
  uint8_t len;
  static uint32_t frame_recv_differ = 0; //接收了数据帧，置‘1’；没有接收则为‘0’
  static uint8_t frame_transmission_count = 0;
  static uint8_t frame_receive_count = 0;
  uint8_t cur_radio_state = 0;
  bspIState_t intState;
	
  if(sendBroadFlag)
  {
    sendBroadFlag = 0;
//定时广播数据，提供数据给节点，可以避免节点死机    
    if(real_time.second % 1 == 0)
    {
      cur_radio_state =  Mrfi_GetCurrentState();
      if((cur_radio_state == MRFI_RADIO_STATE_RX) &&\
          !MRFI_IRQ_INT_IS_ENABLED() )
      {
        BSP_RESET_BOARD();
      }
      else if(cur_radio_state != MRFI_RADIO_STATE_RX)
      {
        MRFI_RxOn();
      }
      else
      {
      }
      //集中器收到数据，说明节点也收到了数据，可以不广播数据；
      //集中器没有收到数据，则需要广播数据。
      if(frame_receive_count != nwk_getFrameReceiveCount())
      {
        frame_recv_differ ++;
        frame_receive_count = nwk_getFrameReceiveCount();
      }
      else
      {
        len = bui_time_sync_pkg(msg);
        send_bcast_pkg(msg,len);
				/*BSP_ENTER_CRITICAL_SECTION(intState);
        MRFI_Init(); //重启射频模块
        MRFI_RxOn(); 
				BSP_EXIT_CRITICAL_SECTION(intState);*/
				
      }
    }
    else
    {
    }
    
    if(real_time.second % 3 == 0)
    {
//发送不成功，重启射频  ;
//接收模式下控制高速开关的引脚状态出错，重启射频    
      if(((frame_transmission_count == MRFI_getFrameTransmissionCount())&&(!frame_recv_differ)))
      {  
				BSP_ENTER_CRITICAL_SECTION(intState);
        MRFI_Init(); //重启射频模块
        MRFI_RxOn();     
        BSP_EXIT_CRITICAL_SECTION(intState);	 
      }
//只能发送，不能接收数据，重启射频
      else if(((MRFI_getFrameTransmissionCount() - frame_transmission_count)>3) &&(!frame_recv_differ))
      {
				BSP_ENTER_CRITICAL_SECTION(intState);
        MRFI_Init(); //重启射频模块
        MRFI_RxOn();     
        BSP_EXIT_CRITICAL_SECTION(intState);		
        frame_transmission_count = MRFI_getFrameTransmissionCount();				
      }
      else
      {
        frame_transmission_count = MRFI_getFrameTransmissionCount();
      }
      frame_recv_differ = 0;
			frame_transmission_count = MRFI_getFrameTransmissionCount();
    }
    else
    {
    }
		
    if((real_time.minute % 5 == 0)&&(real_time.second % 59 == 0))
    {
//5分钟内如果没有数据收发，则重启射频模块
      if(((frame_transmission_count == MRFI_getFrameTransmissionCount())||(frame_receive_count == nwk_getFrameReceiveCount())))
      {  
				BSP_ENTER_CRITICAL_SECTION(intState);
        MRFI_Init(); //重启射频模块
        MRFI_RxOn();     
        BSP_EXIT_CRITICAL_SECTION(intState);	
      }

      frame_recv_differ = 0;
			frame_transmission_count = MRFI_getFrameTransmissionCount();
			frame_receive_count = nwk_getFrameReceiveCount();
    }
    else
    {
    }		
  }
  else
  {
  }
}


