/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   gtway_frm_proc.h
// Description: ��������ͨ��֡.
// Author:      Leidi
// Version:     1.0
// Date:        2014-3-24
// History:     2014-10-24  Leidi (1) �ں���proc_net_msg()��,�ɽڵ��Ϸ���֡ԭ����
//                                    ����rebuild_69_frm()���Է�װΪ�ǹ㲥������ͨ
//                                    ��֡.���ڸ�Ϊ��Bui_bcast_gtway_msg_from_inner_msg()
//                                    һ�ɷ�װΪ�㲥������ͨ��֡.
//                                (2) �ں���proc_net_msg()��,ʹ��Send_gtway_msg_to_uart()
//                                    ����Ҫ�����֡������ȷ�Ĵ��ڣ�����ԭ�ȵĴ���.
//                                (3) �ں���proc_net_msg()��,ap_inner_proc()���ú�,
//                                    ���������.
//              2014-11-5   Leidi  �޸ĺ���net_frm_proc()��ֹ�޷����������еĽ���֡.
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
 * @brief       �ص��������ж������У�����ָ֡ʾ��ʶ��֡�Ķ�ȡ��Ӧ�ò���С�
 *     
 * @data        2015��08��07��
 *     
 * @param       lid - �ڵ�ID��
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
 * @brief       ������link������3�ε���SMPL_LinkListen�����Ѿ���������Ľڵ㣬
 *              ���3�ι�����û�нڵ����link�����򽫸ýڵ��CONNSTATE_JOINED
 *              ״̬����ΪCONNSTATE_FREE״̬���ô洢�ռ�Ԥ���������ڵ�ʹ�á�
 *     
 * @data        2015��08��07��
 *     
 * @param       void
 *     
 * @return      1 , �����ɹ�
 *              0 , ����ʧ��
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
 * @brief       ��ȡ����֡��
 *     
 * @data        2015��08��07��
 *     
 * @param       void
 *     
 * @return      1 , �ɹ�
 *              0 , ʧ��
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
		
        proc_net_msg(i, msg, len);  //�ײ��68֡
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
 * @brief       ����֡��������֡���ݿ�����Ĳ�ͬ��comm_type=0x00����0x01
 *              ��͸�������ڣ�comm_type == 0x04,���Ǽ������ڲ���Ҫ�����֡,
 *              ����������Ҫ͸�������ڡ�
 *     
 * @data        2015��08��07��
 *     
 * @param       lid  - �ڵ�ID��
 *              msg  - ֡�ֽ�����
 *              len  - ֡����
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
/********���ȣ���ED������Ӧ��֡���м�⣡********/
  memset(gtway_out_msg, 0x00, sizeof(gtway_out_msg));
  if(*msg==0x68&&*(msg+len-1)==0x16)
  {
    app_pkg_t in_pkg;
    bui_app_pkg(msg,&in_pkg);
/*�澯֡*/
    comm_type = in_pkg.ctr_code &0x0c;
      
    if((comm_type == 0x00) || (comm_type == 0x01))
    {
      /* ���ڵ��Ϸ����ڲ�֡���Ϊ����ͨ��֡ */
      gtway_out_msg_len = Bui_bcast_gtway_msg_from_inner_msg(msg, len, gtway_out_msg);           
#ifdef DEBUG
      put_multi_char_uart1(gtway_out_msg, gtway_out_msg_len);
#else
      /* ��֡������ȷ�Ĵ���*/
      Send_gtway_msg_to_uart(gtway_out_msg, gtway_out_msg_len);
#endif  
    }
    if(comm_type == 0x04)//�ڵ���ʼ���������͸��(�ڲ�����)
    {
      app_pkg_t in_pkg;
      bui_app_pkg(msg,&in_pkg);
      
      /* AP�ڲ����� */
      ap_inner_proc(&in_pkg, msg, &len);
      if(len!=0)
      {
        /* ���ڵ��Ϸ����ڲ�֡���Ϊ����ͨ��֡ */
        gtway_out_msg_len = Bui_bcast_gtway_msg_from_inner_msg(msg, len, gtway_out_msg);
              
#ifdef DEBUG
        put_multi_char_uart1(gtway_out_msg, gtway_out_msg_len);
#else
        /* ��֡������ȷ�Ĵ���. */
        Send_gtway_msg_to_uart(gtway_out_msg, gtway_out_msg_len);
#endif 
      }
    }
    else if(comm_type == 0x0C)
    {
      /* ���ڵ��Ϸ����ڲ�֡���Ϊ����ͨ��֡ */
      gtway_out_msg_len = Bui_bcast_gtway_msg_from_inner_msg(msg, len, gtway_out_msg);           
#ifdef DEBUG
      put_multi_char_uart1(gtway_out_msg, gtway_out_msg_len);
#else
      /* ��֡������ȷ�Ĵ���*/
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
 * @fn          proc_bcast_frm(��ʱû�ã��ڵ㲻�ᷢ�͹㲥֡)
 *     
 * @brief       ����㲥֡��
 *     
 * @data        2015��08��07��
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
 * @brief       ����ͬ��֡�������Ƶ���ϡ�ÿ3��ʱ�����ڼ��һ�Σ�
 *              (1)���䲻�ɹ�������û�б仯��������Ƶ
 *              (2)��ƵGPIO״̬��������������Ƶ
 *              (3)������㲥֡��������ݣ�����û�н������ݣ�������Ƶ��
 *     
 * @data        2015��08��07��
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
  static uint32_t frame_recv_differ = 0; //����������֡���á�1����û�н�����Ϊ��0��
  static uint8_t frame_transmission_count = 0;
  static uint8_t frame_receive_count = 0;
  uint8_t cur_radio_state = 0;
  bspIState_t intState;
	
  if(sendBroadFlag)
  {
    sendBroadFlag = 0;
//��ʱ�㲥���ݣ��ṩ���ݸ��ڵ㣬���Ա���ڵ�����    
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
      //�������յ����ݣ�˵���ڵ�Ҳ�յ������ݣ����Բ��㲥���ݣ�
      //������û���յ����ݣ�����Ҫ�㲥���ݡ�
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
        MRFI_Init(); //������Ƶģ��
        MRFI_RxOn(); 
				BSP_EXIT_CRITICAL_SECTION(intState);*/
				
      }
    }
    else
    {
    }
    
    if(real_time.second % 3 == 0)
    {
//���Ͳ��ɹ���������Ƶ  ;
//����ģʽ�¿��Ƹ��ٿ��ص�����״̬����������Ƶ    
      if(((frame_transmission_count == MRFI_getFrameTransmissionCount())&&(!frame_recv_differ)))
      {  
				BSP_ENTER_CRITICAL_SECTION(intState);
        MRFI_Init(); //������Ƶģ��
        MRFI_RxOn();     
        BSP_EXIT_CRITICAL_SECTION(intState);	 
      }
//ֻ�ܷ��ͣ����ܽ������ݣ�������Ƶ
      else if(((MRFI_getFrameTransmissionCount() - frame_transmission_count)>3) &&(!frame_recv_differ))
      {
				BSP_ENTER_CRITICAL_SECTION(intState);
        MRFI_Init(); //������Ƶģ��
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
//5���������û�������շ�����������Ƶģ��
      if(((frame_transmission_count == MRFI_getFrameTransmissionCount())||(frame_receive_count == nwk_getFrameReceiveCount())))
      {  
				BSP_ENTER_CRITICAL_SECTION(intState);
        MRFI_Init(); //������Ƶģ��
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


