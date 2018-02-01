/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   uart_frm_proc.c
// Description: User APP.
//              ����Ӵ��������֡.
// Author:      Leidi
// Version:     1.0
// Date:        2012-3-24
// History:     2014-9-29   Leidi �޸ĺ���uart_frm_proc(),ȥ��������֡�޹ص����.
//              2014-9-30   Leidi �޸ĺ���proc_command_frm(),ʹ������app_frame_t.
//                                ���ͷ�ļ�user_def_fun.h.
//                                ʹ�ú궨��MISSES_IN_A_ROW��ʾ��AP���͵�����Դ���.
//                                ����proc_alarm_frm(),proc_net_ack_frm(),send_net_pkg(),send_bcast_pkg()�Ĵ���.
//                                �������õ�ȫ�ֱ���send_status,data_frm_cnt,data_buffer[100],data_cnt.
//                                �������õ�ͷ�ļ�.
//                                ��˽�еĺ���ԭ�ʹ�.h�ļ������뱾�ļ�����static����.
//                                ������ȫ�ֱ���last_tx_frm_num,�ֱ���proc_times_for_uart_frm_rbuf1/2,
//                                �Ա��ڵ��ú���bui_data_msg()ǰ���·���֡������.
//              2014-10-28  Leidi �޸ĺ���SMPL_SendOpt()�Ĳ���ΪSMPL_TXOPTION_ACKREQ.
//                                ���û�ж����NEED_REPLY_FRAME,proc_data_frm()����ݷ��ͳɹ��������ERR���ŵ�ƽ.
//              2014-11-10  Leidi �Ƴ�ͷ�ļ�stm8s_uart.h.
//                                ����������õ�ͷ�ļ�user_app_config.h.
//              2015-07-29  Leidi ɾ������ͷ�ļ�nwk_globals.h,nwk_types.h.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "uart_frm_proc.h"
#include "net_para_fun.h"
#include "net_status_fun.h"
#include "nwk_api.h"
#include "nwk.h"
#include "nwk_types.h"
#include "bsp.h"
#include <string.h>
#include "ed_frm_proc.h"
#include "69_frm_proc.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MISSES_IN_A_ROW 3  //��AP���͵�����Դ���


/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern linkID_t sLinkID1;

extern uint8_t last_tx_frm_num;//��һ�η��͵�֡��

uint8_t proc_times_for_uart_frm_rbuf1;//ͬһ���ڻ���1������Ĵ���
uint8_t proc_times_for_uart_frm_rbuf2;//ͬһ���ڻ���2������Ĵ���

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */

static void proc_data_frm(uint8_t *,uint8_t);
//static void proc_command_frm( uint8_t *,uint8_t); //��Ӧ������δʹ��
//static void proc_alarm_frm( uint8_t *,uint8_t);   //��Ӧ������δʹ��
//static void proc_net_ack_frm(uint8_t *,uint8_t);  //��Ӧ������δʹ��


/**************************************************************************************************
 * @fn          uart_frm_proc
 *
 * @brief       ����Ӵ������������.
 *
 * @param       str - ����������ֽ�����
 *              len - �ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void uart_frm_proc(uint8_t *str, uint8_t len)
{
  /* ͸��ģ�齫�������������ȫ��<����֡>���� */
  proc_data_frm(str, len);
  
  return;    
}

/**************************************************************************************************
 * @fn          uart_frm_proc
 *
 * @brief       �������Դ��ڵ���������ͨ��֡(0x69֡).
 *
 * @param       str - ���������֡
 *              len - ���������֡��
 *
 * @return      none
 **************************************************************************************************/

void uart2_frm_proc(uint8_t *str,uint16_t len)
{
  ed_pkg_t in_pkg;
  uint8_t trans_direction;
  uint8_t ack_msg[30];
  uint8_t ack_msg_len;
  uint8_t frame_type;
  
  /* ������ͨ��֡����װ������ͨ��֡�ṹ�� */
  bui_ed_pkg(str,&in_pkg);
  
  /* �������ͨ��֡�е�������Ϣ�Ƿ���ȷ */
//  if(gtway_info_check(&in_pkg) == 0)
//  {
//    return;
//  }
  
  /* �������ͨ��֡�������еĴ��䷽���Ƿ���ȷ */
  //������(S) �ֻ�(M) ����(G) ����(C)
  trans_direction = (in_pkg.ctr_code>>8)&0xff;
  switch(trans_direction)
  {
    case 0x00: return;  //M->S(X)
    case 0x01: return;  //S->M(X)
    case 0x02: return;  //G->S(X)
    case 0x03: return;  //S->G
    case 0x04: return;   //M->G
    case 0x05: return;  //G->M(X)
    case 0x06: return;   //M->S->G
    case 0x07: return;  //G->S->M(X)
//    case 0x08: put_multi_hex_uart1(str,len);//��������ʱ������ת��
//               return;  //M->C
    case 0x09: //TCP_Send_Filter(str,len);//��������ʱ������ת��
               return;  //C->M
    case 0x0a: break;   //C->G
    case 0x0b: return;  //G->C(X)
//    case 0x0c: put_multi_hex_uart1(str,len);//��������ʱ������ת��
//               return;  //M->S->C
    case 0x0d: //TCP_Send_Filter(str,len);//��������ʱ������ת��
               return;  //C->S->M
    case 0x0e: break;   //Multicast
    case 0x0f: break;   //Broadcast
		case 0x10: return;
		case 0x11: return;
		case 0x12: break;
		case 0x13: return;
    default: return;
  }

  /* �������ͨ��֡�Ƿ���ҪӦ�� */
  if(in_pkg.ctr_code & 0x4000)
  {
    /* �ڴ����֡ǰ�Ƚ���Ӧ���� */
    ack_msg_len = bui_ed_ack(&in_pkg, ack_msg);
    Send_ed_msg_to_uart(ack_msg, ack_msg_len);
  }

  /* �������ͨ��֡�������е�֡���� */
  frame_type = (in_pkg.ctr_code>>8) & 0x20;
  if(frame_type == 0x00)    //����֡
  {
    proc_69_data_frm(&in_pkg);
  }
  else if(frame_type == 0x20)//Ӧ��֡
  {
    /* ����Ӧ��֡(δʵ��) */
  }
  
  return;
}


/**************************************************************************************************
 * @fn          proc_data_frm
 *
 * @brief       ����Ӵ�����������ݲ��γ�<����֡>����.
 *
 * @param       str - ����������ֽ�����
 *              len - �ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_data_frm(uint8_t* str, uint8_t len)
{
  uint8_t msg[MAX_APP_PAYLOAD];
  uint8_t msg_len;

  /* ����Ƿ�Ϊ���ش�֡,ֻ�з��ش�֡��֡�Ųŵ��� */
  if((proc_times_for_uart_frm_rbuf1 == 0)
   &&(proc_times_for_uart_frm_rbuf2 == 0))
  {
    last_tx_frm_num++;//��һ��ʹ�õ�֡�ż�1��Ϊ��ǰ֡��
  }
  
  /* �������������ݹ�����<����֡>�ĸ��� */
  msg_len = bui_data_msg(str, len, msg);
  
  /* ��AP ��������֡  */
  if(SMPL_SUCCESS == send_net_pkg(sLinkID1, msg, msg_len))
  {
//    PRINT_DEBUG_MSG("Send data frame to AP success!\n");
  }
  else
  {
 //   PRINT_DEBUG_MSG("Send data frame to AP failed!\n");
  }
}


/**************************************************************************************************
 * @fn          proc_command_frm
 *
 * @brief       ����Ӵ��������<����֡>.
 *
 * @param       str - ����������ֽ�����
 *              len - �ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_command_frm(uint8_t* str, uint8_t len)
{
  app_frame_t   in_frame;
  comm_type_t   comm_type;

//  uint8_t alarm_info[4];
  
  if(APP_SUCCESS == bui_app_frame(str, len, &in_frame))
  {
    comm_type   = (comm_type_t)in_frame.ctr_code.comm_type;    //ͨ�ŷ�ʽ
    
    if(comm_type == LOCAL_COMM)//���ʱ��ڵ�
    {
      uint8_t appIndex = 0;
      for(appIndex=0;appIndex<MAX_APP_FUNC;appIndex++)
      {
        if(in_frame.data[1] == appFuncDomain[appIndex].commandId_DI2)
        {
          appFuncDomain[appIndex].pFunx(&in_frame);
          break;
        }
      }
      if(appIndex == MAX_APP_FUNC)
      {
//        alarm_info[0]=0x00;
//        alarm_info[1]=0x01;
//        alarm_info[0]=0x01;
//        alarm_info[1]=0x08;
//        out_msg_len = bui_alarm_msg(&in_frame, alarm_info, 4,out_msg);
      }

//      PRINT_DEBUG_MSG("Order frame execute result:");
 //     PRINT_DEBUG_BYTES(out_msg, out_msg_len);
      
      //SEND_BYTES_TO_UART(out_msg, out_msg_len);	
			
    }
    else if((comm_type == NO_TRANSPARENT_COMM) || (comm_type == TRANSPARENT_COMM))//���ʼ�����
    {
      /* ��AP��������֡ */
      if(SMPL_SUCCESS == send_net_pkg(sLinkID1, str, len))
      {
//        PRINT_DEBUG_MSG("Send order frame to AP success!\n");
      }
      else
      {
  //      PRINT_DEBUG_MSG("Send order frame to AP failed!\n");
      }
    }
  }
  else //�Ƿ�֡:֡��ʽ����ȷ
  {
//    PRINT_DEBUG_MSG("Recv illegal msg:");
 //   PRINT_DEBUG_BYTES(str, len);
  }
}


/**************************************************************************************************
 * @fn          proc_alarm_frm
 *
 * @brief       ����Ӵ��������<�澯֡>����AP����.
 *
 * @param       str - ����������ֽ�����
 *              len - �ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_alarm_frm(uint8_t* str, uint8_t len)
{
  uint8_t out_msg[MAX_APP_PAYLOAD];
  uint8_t out_msg_len;
  uint8_t alarm_info[4];
  
  /* ��AP���͸澯֡ */
  if(SMPL_SUCCESS == send_net_pkg(sLinkID1, str, len))
  {
//    PRINT_DEBUG_MSG("Send alarm frame to AP success!\n");
  }
  else
  {
//    PRINT_DEBUG_MSG("Send alarm frame to AP failed!\n");
    
    alarm_info[0]=0x00;
    alarm_info[1]=0x01;
    alarm_info[2]=0x01;
    alarm_info[3]=0x08;
    out_msg_len = bui_alarm_msg(NULL, alarm_info, 4, out_msg);
    
//    SEND_BYTES_TO_UART(out_msg, out_msg_len);
    put_multi_hex_uart1(out_msg, out_msg_len);			
    
//    PRINT_DEBUG_BYTES(out_msg, out_msg_len);
  }
}


/**************************************************************************************************
 * @fn          proc_alarm_frm
 *
 * @brief       ����Ӵ��������<Ӧ��֡>����AP����.
 *
 * @param       str - ����������ֽ�����
 *              len - �ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_net_ack_frm( uint8_t* str, uint8_t len)
{
  uint8_t out_msg[MAX_APP_PAYLOAD];
  uint8_t out_msg_len;
  uint8_t alarm_info[4];
  
  /* ��AP����Ӧ��֡ */
  if(SMPL_SUCCESS == send_net_pkg(sLinkID1, str, len))
  {
//    PRINT_DEBUG_MSG("Send reply frame to AP success!\n");
  }
  else
  {
//    PRINT_DEBUG_MSG("Send reply frame to AP failed!\n");
    
    alarm_info[0]=0x00;
    alarm_info[1]=0x01;
    alarm_info[2]=0x01;
    alarm_info[3]=0x08;
    out_msg_len = bui_alarm_msg(NULL, alarm_info, 2, out_msg);
    
//    SEND_BYTES_TO_UART(out_msg, out_msg_len);
    put_multi_hex_uart1(out_msg, out_msg_len);    
//    PRINT_DEBUG_BYTES(out_msg, out_msg_len);
  }
}


/**************************************************************************************************
 * @fn          send_net_pkg
 *
 * @brief       ��ָ�������ӱ�ʶ��(LID)��AP��������.����Դ�����MISSES_IN_A_ROW����.
 *
 * @param       lid     - ���ӱ�ʶ��(LID)
 *              string  - ������ֽ�����
 *              tx_len  - �ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
smplStatus_t send_net_pkg(linkID_t lid, uint8_t *string, uint8_t tx_len)
{
  uint8_t     misses;
  smplStatus_t rc;

//  PRINT_DEBUG_MSG("Sending msg to AP...\n");
  
  /* get radio ready...awakens in idle state */
  SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
  
  /* Try sending message MISSES_IN_A_ROW times looking for ack */
  for (misses = 0; misses < MISSES_IN_A_ROW; misses++)
  {
   // PRINT_DEBUG_MSG("Trying send ");
//    PRINT_DEBUG_BYTES(&misses, 1);
   // PRINT_DEBUG_MSG(" times...");
    
    //rc = SMPL_SendOpt(lid, string, tx_len, SMPL_TXOPTION_ACKREQ);//SMPL_TXOPTION_NONE///
    rc = SMPL_SendOpt(lid, string, tx_len, SMPL_TXOPTION_NONE);/////
    if(SMPL_SUCCESS == rc)
    {
     // PRINT_DEBUG_MSG("Send success!\n");
      
      //PRINT_DEBUG_BYTES(string, tx_len);
      
      break;
    }
    else 
    {
     // NWK_DELAY(5);
     // PRINT_DEBUG_MSG("Send failed!\n");
    }
  }
  return rc;       
}


/**************************************************************************************************
 * @fn          send_bcast_pkg
 *
 * @brief       ���͹㲥����.
 *
 * @param       string  - ������ֽ�����
 *              len     - �ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
smplStatus_t send_bcast_pkg(uint8_t *string, uint8_t len)
{
  smplStatus_t rc;
  
    //PRINT_DEBUG_MSG("Sending Bcast msg...\n");
	
  rc = SMPL_SendOpt(SMPL_LINKID_USER_UUD, string, len, SMPL_TXOPTION_NONE);
  if(SMPL_SUCCESS == rc)
  {
    //PRINT_DEBUG_MSG("Send Bcast success!\n");  
   //PRINT_DEBUG_BYTES(string, len);
  }
  else 
  {
    //PRINT_DEBUG_MSG("Send Bcast failed!\n");
  }
  
  return rc;
}

