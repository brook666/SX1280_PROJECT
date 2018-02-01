/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_frm_proc.c
// Description: User APP.
//              SWSN����֡��������Դ�ļ�.
// Author:      Leidi
// Version:     1.0
// Date:        2012-3-24
// History:     2014-9-25   Leidi ɾ��ȫ�ֱ���ApAddr,ʹ�ùٷ�����nwk_getAPAddress()��ȡAP��ַ.
//              2014-9-26   Leidi ɾ������ȫ�ֱ���head_list.
//                                ɾ������ȫ�ֱ���continue_msg[200].
//                                ɾ�����õ�ȫ�ֱ���net_ed_num, net_ed_add_sem.
//                                ɾ������ͷ�ļ�.
//                                �޸�sCB()�ķ���ֵ,���Ǳ���LIDʱ����1.
//                                net_frm_proc()�ĺ�������ֵ��Ϊvoid.
//                                �޸�proc_net_msg()�ĺ���ԭ���붨��,ȥ�����õĲ���lid.����������const���η�.
//              2014-9-27   Leidi �޸�proc_net_msg()�ĺ���ԭ���붨��,�ָ�����lid.
//                                �޸�proc_net_msg()������.
//              2014-9-28   Leidi �Ƴ�ap_inner_fun.h,������ap_inner_proc(),�˺����Ĺ����ѹ�ʱ.
//              2014-9-29   Leidi proc_bcast_frm()������������������:bcast_frm_proc(),proc_bcast_msg().
//                                �Ƴ����ļ������õĺ궨��MISSES_IN_A_ROW.
//              2014-9-30   Leidi ȫ�ֱ���last_tx_frm_num��last_rx_frm_num�ڱ��ļ��ж���.
//              2014-10-8   Leidi ɾ��ȫ�ֱ���recv_cont_frm_flag��cont_frm_minu_count����ش���.�˱��������ڽ�����������֡ʱ��ʹ����Ƶ.
//              2014-10-29  Leidi �ں���proc_net_msg()��,��ԭ��ֻ����͸������ҪӦ����ظ�֡,�ֹ���͸�����ظ�֡.
//              2014-11-10  Leidi �Ƴ�ͷ�ļ�stm8s_uart.h.
//                                ����������õ�ͷ�ļ�user_app_config.h.
//              2015-01-20  Leidi �޸�Bug:����net_frm_proc()��SMPL_Receiveʧ��ʱ������net_frm_cur_num.
//              2015-07-29  Leidi ɾ������ͷ�ļ�nwk_types.h.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "net_frm_proc.h"
#include "bui_pkg_fun.h"
#include "net_para_fun.h"
#include "net_status_fun.h"
#include "uart_frm_proc.h"
#include "nwk_api.h"
#include "nwk.h"
#include "bsp.h"
#include "stm32_usart2_gtway.h"
#include <string.h>
#include "ed_tx_fail_buffer_fun.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern linkID_t sLinkID1;        //���ӱ�ʶ��(LID)
volatile uint8_t net_frm_cur_num=0;  //�ײ���ն����зǹ㲥֡������
volatile uint8_t bro_cast_sem=0;     //�ײ���ն����й㲥֡������,��ȡ��֮������
volatile uint8_t bro_cast_total_num =0; //�ײ���ն����й㲥֡��������һֱ����
extern volatile net_frm_back_t net_ack_back;

uint8_t send_usart_frm_count=0;

uint8_t last_tx_frm_num;//��һ�η��͵�֡��
uint8_t last_rx_frm_num;//��һ�γɹ����յ�֡��

uint8_t reply_frame_received;   //��ЧӦ��֡���ձ�־:�յ���ЧӦ��֡ʱ��0


/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void proc_net_msg(linkID_t lid, const uint8_t *, uint8_t);
static void proc_bcast_msg(const uint8_t *msg, uint8_t len);
static void bui_usart68_frm(uint8_t * ,uint8_t *,uint8_t data_len,uint8_t);

extern uint8_t registerAppFuncDomain(uint8_t ,pAppFunc );
/**************************************************************************************************
 * @fn          sCB
 *
 * @brief       ED�˵���Ƶ���ӽ��ջص�����,���жϷ�����������.�������ӱ�ʶ��(LID)ʶ���յ����û�֡.
 *              ֡�Ķ�ȡӦ�����û�Ӧ�ó���������ڴ˴�����.����μ�SimpliciTI API(SWRA221 Version 1.2)
 *              ��7����Callback Interface.
 *
 * @param       lid - �յ��û�֡�����ӱ�ʶ��(LID)
 *
 * @return      0         - ���õ�֡,�յ���֡��������������й��û��Ժ�ȡ��
 *              non-zero  - ���õ�֡,�յ���֡����������������ͷ�
 **************************************************************************************************
 */
uint8_t sCB(linkID_t lid)
{
  if(lid == sLinkID1)//����Ǳ�����LID
  {
    net_frm_cur_num++;
  }
  else if(lid == SMPL_LINKID_USER_UUD)//�����������(�㲥)��LID
  {
    bro_cast_sem++;
    bro_cast_total_num++;
  }
  else//����ǷǱ�����LID 
  {
    return 1;
  }
 
  return 0;
}


/**************************************************************************************************
 * @fn          net_frm_proc
 *
 * @brief       ���ݱ���LIDȡ�طǹ㲥�û�֡�����ɴ���������.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void net_frm_proc(void)
{
  uint8_t msg[MAX_APP_PAYLOAD], len;
  uint8_t alarm_info[4];
  bspIState_t intState;
  
  //PRINT_DEBUG_MSG("Proc net frm...\n");

  if(net_frm_cur_num)
  {
    /* process all frames waiting */
    if (SMPL_SUCCESS == SMPL_Receive(sLinkID1, msg, &len))
    {
      /* �����յ���֡ */
      proc_net_msg(sLinkID1, msg, len);
    }  
    else
    {
     /* �����澯֡ */
     alarm_info[0]=0x00;
     alarm_info[1]=0x01;
     alarm_info[2]=0x01;
     alarm_info[3]=0xfe;
     len = bui_alarm_msg(NULL, alarm_info, 4, msg);
     
     /* ���͸澯֡ */
     send_net_pkg(sLinkID1,msg,len);
    
     //PRINT_DEBUG_MSG("Process net frm ERROR:");
     //PRINT_DEBUG_BYTES(msg,len);
    }
        
    BSP_ENTER_CRITICAL_SECTION(intState);
    net_frm_cur_num--;
    BSP_EXIT_CRITICAL_SECTION(intState);
  }
}


/**************************************************************************************************
 * @fn          proc_net_msg
 *
 * @brief       �Էǹ㲥�û�֡��ʽ���н��������ദ��.
 *
 * @param       lid - ���ӱ�ʾ��,���ڷ��ͷ���֡
 *              msg - ���յ����û�֡
 *              len - ���յ����û�֡����
 *
 * @return      none
 **************************************************************************************************
 */
static void proc_net_msg(linkID_t lid, const uint8_t *msg, uint8_t len)
{
  app_frame_t   in_frame;
  frame_type_t  frame_type;
  comm_type_t   comm_type;
  //trans_type_t  trans_type;
  reply_type_t  reply_type;
  uint8_t out_msg[MAX_APP_PAYLOAD];
  uint8_t out_msg_len;
  uint8_t alarm_info[4];
 
//  PRINT_DEBUG_MSG("Proc net msg...\n");
  /* ������AP������֡ */
  if(APP_SUCCESS == bui_app_frame(msg, len, &in_frame))
  {
    frame_type  = (frame_type_t)in_frame.ctr_code.frame_type;  //֡����
    comm_type   = (comm_type_t)in_frame.ctr_code.comm_type;    //ͨ�ŷ�ʽ
    //trans_type  = (trans_type_t)in_frame.ctr_code.trans_type;  //����״̬
    reply_type  = (reply_type_t)in_frame.ctr_code.reply_type;  //Ӧ������
       
    if(comm_type == TRANSPARENT_COMM)//���������ʽڵ㣬����͸��
    {
      /*if(last_rx_frm_num == in_frame.frm_cnt)//���յ���֡���ϴ��յ���֡����ͬ��˵�����ط�֡����ҪӦ���֡
      {
        if(reply_type == NEED_REPLY)//�����ҪӦ��˵�����ظ�֡
        {
          send_ack_msg(&in_frame);//����Ӧ��֡
          //return;//��������ҪӦ����ظ�֡
        }
        return;//�������ظ�֡
      }*/
      /* ���е��˴�˵�������ط�֡���ǲ���ҪӦ���֡ */
      
      if(frame_type == DATA_FRAME)//����֡
      {
        //PRINT_DEBUG_MSG("Recv data msg:");
        //PRINT_DEBUG_BYTES(msg,len);
        
      //  SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//ֻ���֡����  
        put_multi_hex_uart1(in_frame.data, in_frame.data_len);				
//        out_msg_len = bui_ed_to_ap_msg(msg,len,out_msg);
//        send_net_pkg(lid, out_msg, out_msg_len);
      }
      else //������֡
      {                    
       // PRINT_DEBUG_MSG("Recv msg:");
       // PRINT_DEBUG_BYTES(msg,len);
       // SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//ֻ���֡����  
				put_multi_hex_uart1(in_frame.data, in_frame.data_len);	
      }
      
      /* ���е��˴�˵����Ч֡�Ѿ�ͨ��������� */
      last_rx_frm_num = in_frame.frm_cnt;//���½���֡��
      if(reply_type == NEED_REPLY)
      {
        out_msg_len = bui_reply_msg(&in_frame, out_msg);//����Ӧ��֡
        send_net_pkg(lid, out_msg, out_msg_len);//����Ӧ��֡
      }
      
    }
    else if(comm_type == NO_TRANSPARENT_COMM)//���������ʽڵ㣬���ݲ�͸��
    {
      if(frame_type == REPLY_FRAME)//Ӧ��֡
      {
        if(last_tx_frm_num == in_frame.frm_cnt)//���յ���֡���ϴη��͵�֡����ͬ��˵�����ϴη��͵�Ӧ��֡
        {
          reply_frame_received = 0x01;
        }
        else//��Ч��Ӧ��֡
        {
          reply_frame_received = 0x00;
        }
      }
      else if(frame_type == ORDER_FRAME)//����֡ 
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
          alarm_info[0]=0x00;
          alarm_info[1]=0x01;
          alarm_info[2]=0x01;
          alarm_info[3]=0x08;
          send_alarm_msg(&in_frame, alarm_info, 4);
        }
        
       /* ���ͻظ�֡ */
      //  send_net_pkg(lid, out_msg, out_msg_len);
      }
      else//�澯֡������֡
      {
      //  PRINT_DEBUG_MSG("net frame type ERROR!\n");
      //  PRINT_DEBUG_BYTES(msg,len);
      }
    }
		
		if(comm_type == RESERVE_COMM)//   RESERVE
    {
      /*if(last_rx_frm_num == in_frame.frm_cnt)//���յ���֡���ϴ��յ���֡����ͬ��˵�����ط�֡����ҪӦ���֡
      {
        if(reply_type == NEED_REPLY)//�����ҪӦ��˵�����ظ�֡
        {
          send_ack_msg(&in_frame);//����Ӧ��֡
          //return;//��������ҪӦ����ظ�֡
        }
        return;//�������ظ�֡
      }*/
      /* ���е��˴�˵�������ط�֡���ǲ���ҪӦ���֡ */
      
      if(frame_type == DATA_FRAME)//����֡
      {
        //PRINT_DEBUG_MSG("Recv data msg:");
        //PRINT_DEBUG_BYTES(msg,len);
        
        //SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//ֻ���֡����  
        put_multi_hex_uart1(in_frame.data, in_frame.data_len);				
        out_msg_len = bui_ed_to_ap_msg(msg,len,out_msg);
        send_net_pkg(lid, out_msg, out_msg_len);
      }
      else //������֡
      {                    
       // PRINT_DEBUG_MSG("Recv msg:");
       // PRINT_DEBUG_BYTES(msg,len);
       // SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//ֻ���֡����  
				put_multi_hex_uart1(in_frame.data, in_frame.data_len);	
      }
      
      /* ���е��˴�˵����Ч֡�Ѿ�ͨ��������� */
      last_rx_frm_num = in_frame.frm_cnt;//���½���֡��
      if(reply_type == NEED_REPLY)
      {
        out_msg_len = bui_reply_msg(&in_frame, out_msg);//����Ӧ��֡
        send_net_pkg(lid, out_msg, out_msg_len);//����Ӧ��֡
      }
      
    }
		
	  return;
  }
  else //�Ƿ�֡:֡��ʽ����ȷ
  {
   // PRINT_DEBUG_MSG("Recv illegal msg:");
    //PRINT_DEBUG_BYTES(msg,len);
		  return;
  }
	
}


/**************************************************************************************************
 * @fn          bcast_frm_proc
 *
 * @brief       ȡ�ع㲥�û�֡�����ɴ���������.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void bcast_frm_proc(void)
{
  uint8_t msg[MAX_APP_PAYLOAD], len;
  bspIState_t intState;
  
//  PRINT_DEBUG_MSG("Proc BCast frm...\n");

  if(bro_cast_sem)
  {
    /* process all frames waiting */
    if (SMPL_SUCCESS == SMPL_Receive(SMPL_LINKID_USER_UUD, msg, &len))
    {
      /* �����յ���֡ */
      proc_bcast_msg(msg, len);
      
      BSP_ENTER_CRITICAL_SECTION(intState);
      bro_cast_sem--;
      BSP_EXIT_CRITICAL_SECTION(intState);
     }  
     else
     {
//       PRINT_DEBUG_MSG("Recv BCast frm ERROR!\n");
     } 
  }
}


/**************************************************************************************************
 * @fn          proc_bcast_msg
 *
 * @brief       �Թ㲥�û�֡��ʽ���н��������ദ��.
 *
 * @param       msg - ���յ����û�֡
 *              len - ���յ����û�֡����
 *
 * @return      none
 **************************************************************************************************
 */
void proc_bcast_msg(const uint8_t *msg, uint8_t len)
{
  app_frame_t   in_frame;
  frame_type_t  frame_type;
  comm_type_t   comm_type;
  uint8_t alarm_info[4];
//  PRINT_DEBUG_MSG("Proc BCast msg...\n");

  /* ������AP������֡ */
  if(APP_SUCCESS == bui_app_frame(msg, len, &in_frame))
  {
    frame_type  = (frame_type_t)in_frame.ctr_code.frame_type;  //֡����
    comm_type   = (comm_type_t)in_frame.ctr_code.comm_type;    //ͨ�ŷ�ʽ
   
    last_rx_frm_num = in_frame.frm_cnt;//��ȡ����֡��
    
    if(comm_type == TRANSPARENT_COMM)//���������ʽڵ㣬����͸��
    {
//      PRINT_DEBUG_MSG("Recv BCast msg:");
//      PRINT_DEBUG_BYTES(msg,len);
      
      //SEND_BYTES_TO_UART(msg, len);//�������֡
		//	put_multi_hex_uart1(msg, len);	
//      SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//ֻ���֡����
			put_multi_hex_uart1(in_frame.data, in_frame.data_len);	
    }
    else if(comm_type == NO_TRANSPARENT_COMM)//���������ʽڵ㣬���ݲ�͸��
    {
      if(frame_type == ORDER_FRAME)//����֡
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
          alarm_info[0]=0x00;
          alarm_info[1]=0x01;
          alarm_info[2]=0x01;
          alarm_info[3]=0x08;
          send_alarm_msg(&in_frame, alarm_info, 4);;
        }
//        PRINT_DEBUG_MSG("BCast order frame execute result:");
        //PRINT_DEBUG_BYTES(out_msg, out_msg_len);
      }
      else//�㲥֡��֧�ִ�����:�澯֡��Ӧ��֡
      {
//        PRINT_DEBUG_MSG("BCast frame type ERROR!\n");
        //PRINT_DEBUG_BYTES(msg,len);
      }
    }
    else//ͨ�ŷ�ʽ���󣺹㲥֡��Ӧ�д�ͨ�ŷ�ʽ
    {
//      PRINT_DEBUG_MSG("BCast Comm type ERROR!\n");
//      PRINT_DEBUG_BYTES(msg,len);
    }
  }
  else //�Ƿ�֡:֡��ʽ����ȷ
  {
//    PRINT_DEBUG_MSG("Recv illegal BCast msg:");
//    PRINT_DEBUG_BYTES(msg,len);
  }
}

smplStatus_t	send_usartdata_to_ap(uint8_t * usart_data,uint8_t data_len)
{
	if(data_len!=0)
  {
	  uint8_t out_msg[MAX_APP_PAYLOAD] = {0};		
		send_usart_frm_count++;		
		
    bui_usart68_frm(usart_data,out_msg,data_len,send_usart_frm_count);			
		
	  send_usart_data_pkg(sLinkID1, out_msg, 10+data_len);
  }
}

/**************************************************************************************************
 * @fn          send_usart_data_pkg
 *
 * @brief       ��ָ�������ӱ�ʶ��(LID)��ED��������.����Դ�����MISSES_IN_A_ROW����.
 *
 * @param       lid     - ���ӱ�ʶ��(LID)
 *              string  - ������ֽ�����
 *              tx_len  - �ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
uint8_t send_usart_data_pkg(linkID_t lid, uint8_t *string, uint8_t tx_len)
{
  uint8_t       rc;
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"Sending msg to AP..."RTT_CTRL_RESET"\n");
#endif
    
    rc = send_user_msg(lid, string, tx_len);
    if (1 == rc)
    {
#ifdef DEBUG
      put_string_uart1("Send success!\n");
      put_multi_char_uart1(string, tx_len);
#endif
//      for(i = 0; i < 30; i++)
//      {
//        SWSN_DELAY(1);   
//        if((net_ack_back.lid == lid) && (net_ack_back.isBack == 1))
//        {
//          memset((void*)&net_ack_back,0x0,sizeof(net_ack_back));
#ifdef IWDG_START 
          IWDG_ReloadCounter();  
#endif
          return rc;
//        }
//      } 
//      Write_tx_fail_ed_buffer(lid, string, &tx_len); 
    }
    else 
    {
    }
  
  if(rc !=1)  
  {			
    Write_tx_fail_ed_buffer(lid, string, &tx_len);  
  }
#ifdef IWDG_START 
   IWDG_ReloadCounter();  
#endif  
  return rc;           
}	
	
void bui_usart68_frm(uint8_t * usart_data,uint8_t * out_msg,uint8_t data_len,uint8_t frm_cnt)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //���4�ֽ�IP��ַ
  uint8_t i=0;

  /* ��ȡ����IP��AP��IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* �޸Ŀ����룺����ҪӦ�������֡,�Ƿ�͸���ɲ������� */
  ctr_code.frame_type = DATA_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type  = TRANSPARENT_COMM;
  ctr_code.comm_dir = ED_TO_AP;
  ctr_code.trans_type = ONCE_TRANS;

  /* ���֡���� */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = frm_cnt;  
  out_msg[7] = data_len;//������Ϊ����Ϊ �����ʶ+��������
  for(i=0; i<data_len; i++)
  {
    out_msg[8+i] = *(usart_data+i);
  }
  out_msg[8+data_len] = gen_crc(out_msg, 8+data_len);//CSУ����
  out_msg[8+1+data_len] = APP_FRAME_TAIL;
	
}

/**************************************************************************************************
 */
