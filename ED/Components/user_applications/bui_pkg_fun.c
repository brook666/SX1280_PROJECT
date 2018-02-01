/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   bui_pkg_fun.c
// Description: User APP.
//              SWSN����֡�Ĺ��������.
// Author:      Leidi
// Version:     2.0
// Date:        2012-3-24
// History:     2014-9-25   Leidi ɾ��ȫ�ֱ���ApAddr,ʹ�ùٷ�����nwk_getAPAddress()��ȡAP��ַ. 
//              2014-9-26   Leidi bui_app_pkg()������������const���η�.
//              2014-9-27   Ledi  ��������bui_app_frame(),bui_reply_msg(),bui_alarm_msg(),
//                                bui_order_msg()
//              2014-9-28   Leidi ��������bui_data_msg().
//              2014-9-29   Leidi �޸ĺ���bui_app_frame()�Ĳ����ͷ���ֵ, ʹ���ܷ��ؽ����ɹ���ʧ��.
//              2014-9-30   Leidi ɾ�����º�������:bui_app_pkg(),bui_rep_pkg_command(),bui_pkg_ack(),
//                                bui_pkg_alarm(),bui_68_frm(),bui_ap_inner_command_pkg().
//                                ���ע��.
//                                ���ļ��е�ȫ�ֱ���myIpAddr��Ϊextern,��main_swsn_ed.c�ж���.
//                                �ں���bui_data_msg()�в�����֡���������Ƶ��䱻���ô�����.ͬʱ�Ƴ����ļ�����ص�ȫ�ֱ���.
//              2014-10-8   Leidi �޸�BUG: �ں���bui_alarm_msg()��, Ӧ�ðѸ澯֡��ͨ�ŷ�ʽ��Ϊ<͸��>.
//              2014-10-28  Leidi �ں���bui_data_msg()�У������Ƿ����NEED_REPLY_FRAME,��������֡����ҪӦ��λ.
//              2014-10-29  Leidi �ڸ����������,��������ctr_code�е�comm_dir��ΪED_TO_AP.
//              2014-11-10  Leidi �Ƴ�ͷ�ļ�stm8s_uart.h.
//                                ����������õ�ͷ�ļ�user_app_config.h.
//              2015-01-07  Leidi �޸�BUG:bui_order_msg��Ӧ�ó�ʼ��������Ĵ���״̬ΪONCE_TRANS.
//              2015-07-29  Leidi ɾ������ͷ�ļ�mrfi_spi.h,nwk_types.h.
//                                �Ƴ���ȫ�ֱ���myIpAddr������.
//                                ����ͷ�ļ�addr_control_fun.h,ʹ�ú���Get_IpAddress()����ȡIP��ַ.
//              2015-08-07  Leidi �޸�֡��ʽ:��������ַ��Ϊ2�ֽ�,֡��������Ϊ1�ֽ�.
//              2015-09-02  Zengjia �ı�ڵ�IP�ͼ�����IP�Ļ�ȡ��ʽ��ʹ��
                                    SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
*****************************************************************************/


/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bui_pkg_fun.h"
#include "nwk_globals.h"
#include "nwk_api.h"
#include "bsp.h"
#include "uart_frm_proc.h"
#include <string.h>
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern linkID_t sLinkID1;
/* ------------------------------------------------------------------------------------------------
 *                                            Local Variables
 * ------------------------------------------------------------------------------------------------
 */



/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */


/**************************************************************************************************
 * @fn          gen_crc
 *
 * @brief       ����λ�������ж���޷���8λ�����������ۼӺͳ���256������.
 *
 * @param       frm - ������ֽ�����
 *              len - Ҫ������ֽ����鳤��
 *
 * @return      ������
 **************************************************************************************************
 */
uint8_t gen_crc(const uint8_t *frm, uint8_t len)
{
  uint8_t crc_sum=0;
  uint8_t i;
  for( i =0; i <len; i++)
  {
      crc_sum = *frm + crc_sum;
      frm++;
  }
  return crc_sum;
}


/**************************************************************************************************
 * @fn          bui_app_frame
 *
 * @brief       ��������ֽ�������н���������Ϊ֡�ṹ���Է����ȡ֡�ṹ�ĸ�������.
 *
 * @param       in_msg      - ������ֽ�����
 *              in_msg_len  - ������ֽ����鳤��
 *              out_frame   - �����õ�֡�ṹ��
 *
 * @return      APP_ERR     - ����֡�ṹ��ʧ��,������ֽ������ʽ����ȷ
 *              APP_SUCCESS - ����֡�ṹ��ɹ�
 **************************************************************************************************
 */
app_status_t bui_app_frame(const uint8_t *in_msg, uint8_t in_msg_len, app_frame_t *out_frame)
{
  uint8_t data_len, i;
  app_status_t rc = APP_ERR;

  if(in_msg[0] == APP_FRAME_HEAD && in_msg[in_msg_len-1] == APP_FRAME_TAIL)
  {
    /* ֡��ʼ�� */
    out_frame->frm_head = in_msg[0];
    
    /* ������ */
    out_frame->ctr_code.code = in_msg[1];
    
    /* �ڵ�IP��ַ */
    out_frame->ed_addr_1 = in_msg[2];
    out_frame->ed_addr_0 = in_msg[3];
    
    /* ��������ַ */
    out_frame->ap_addr_1 = in_msg[4];
    out_frame->ap_addr_0 = in_msg[5];
    
    /* ֡������ */
    out_frame->frm_cnt = in_msg[6];
  
    /*�����򳤶�*/
    out_frame->data_len = in_msg[7];
    data_len = in_msg[7];
    
    /*������*/
    for(i = 0; i < data_len; i++)
    {
      out_frame->data[i] = in_msg[8+i];
    }
    
    /*У����*/
    out_frame->cs = in_msg[8 + data_len];
    
    /*֡������*/
    out_frame->frm_tail = in_msg[9 + data_len];
    
    rc = APP_SUCCESS;
  }
  
  return rc;
}


/**************************************************************************************************
 * @fn          bui_reply_msg
 *
 * @brief       �������֡�ṹ����<Ӧ��֡>����ʽ����Ϊ�ֽ����飬�Ա���ͨ�����緢��.
 *
 * @param       in_frame    - �����֡�ṹ��
 *              out_msg     - ������ֽ�����
 *
 * @return      ������ֽ����鳤��
 **************************************************************************************************
 */
uint8_t bui_reply_msg(const app_frame_t *in_frame, uint8_t *out_msg)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};
    
  /* ��ȡ����IP��AP��IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr); 
  
  /* ��ȡԭʼ������ */
  ctr_code.code = in_frame->ctr_code.code;
  
  /* �޸Ŀ����룺����ҪӦ���Ӧ��֡ */
  ctr_code.frame_type = REPLY_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_dir = ED_TO_AP;

  /* ���֡���� */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = in_frame->frm_cnt;  
  out_msg[7] = 0;//������Ϊ����Ϊ0
  out_msg[8] = gen_crc(out_msg, 8);//CSУ����
  out_msg[9] = APP_FRAME_TAIL;
  
  /* ����֡�� */
  return 10;
}


/**************************************************************************************************
 * @fn          bui_alarm_msg
 *
 * @brief       �������֡�ṹ����<�澯֡>����ʽ����Ϊ�ֽ����飬�Ա���ͨ�����緢��.
 *
 * @param       in_frame    - �����֡�ṹ��
 *              err_info    - ����ĸ澯��Ϣ�ֽ�����
 *              err_len     - �澯��Ϣ�ֽ����鳤��
 *              out_msg     - ������ֽ�����
 *
 * @return      ������ֽ����鳤��
 **************************************************************************************************
 */
uint8_t bui_alarm_msg(const app_frame_t *in_frame, const uint8_t *err_info, uint8_t err_len, uint8_t *out_msg)
{
  uint8_t i;
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //���4�ֽ�IP��ַ
  static uint8_t alarm_frm_cnt=0;//����Ӧ��֡��֡������
  
  /* ��ȡ����IP��AP��IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* ��ȡԭʼ������ */
  if(in_frame != NULL)
  {
    ctr_code.code = in_frame->ctr_code.code;
  }
  else
  {
    ctr_code.code = 0x00;
  }
  
  /* �޸Ŀ����룺����ҪӦ��ĸ澯֡,͸�� */
  ctr_code.frame_type = ALARM_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type = TRANSPARENT_COMM;
  ctr_code.comm_dir = ED_TO_AP;

  /* ���֡���� */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  if(in_frame != NULL)
  {
    out_msg[6] = in_frame->frm_cnt;  
  }
  else
  {
    out_msg[6] = alarm_frm_cnt;  
    alarm_frm_cnt++;
  }
  out_msg[7] = err_len;
  for(i=0; i<err_len; i++)
  {
    out_msg[8+i] = err_info[i];
  }
  out_msg[8+err_len] = gen_crc(out_msg, 8+err_len);//CSУ����
  out_msg[9+err_len] = APP_FRAME_TAIL;
  
  /* ����֡�� */
  return 10+err_len;
}


/**************************************************************************************************
 * @fn          bui_order_msg
 *
 * @brief       �������֡�ṹ����<����֡>����ʽ����Ϊ�ֽ����飬�Ա���ͨ�����緢��.
 *
 * @param       comm_type   - ͨ�ŷ�ʽ:
 *                            TRANSPARENT_COMM    - ͸��,���ͽo������������������,ԭ�����
 *                            NO_TRANSPARENT_COMM - ��͸��,���ͽo�������������ڲ�����
 *              in_frame    - �����֡�ṹ��
 *              reply_para  - ���������ؽ���ֽ�����
 *              para_len    - ����ؽ���ֽ����鳤��
 *              out_msg     - ������ֽ�����
 *
 * @return      ������ֽ����鳤��
 **************************************************************************************************
 */
uint8_t bui_order_msg(comm_type_t comm_type, const app_frame_t *in_frame, const uint8_t *reply_para, uint8_t para_len, uint8_t *out_msg)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //���4�ֽ�IP��ַ
  uint8_t i;

  /* ��ȡ����IP��AP��IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* ��ȡԭʼ������ */
  ctr_code.code = in_frame->ctr_code.code;
  
  /* �޸Ŀ����룺����ҪӦ�������֡,�Ƿ�͸���ɲ������� */
  ctr_code.frame_type = ORDER_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type  = comm_type;
  ctr_code.comm_dir = ED_TO_AP;
  ctr_code.trans_type = ONCE_TRANS;

  /* ���֡���� */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = in_frame->frm_cnt;  
  out_msg[7] = 4 + para_len;//������Ϊ����Ϊ �����ʶ+��������
  for(i=0; i<4; i++)
  {
    out_msg[8+i] = in_frame->data[i];
  }
  for(i=0; i<para_len; i++)
  {
     out_msg[12+i] = reply_para[i];
  }
  out_msg[12+para_len] = gen_crc(out_msg, 12+para_len);//CSУ����
  out_msg[13+para_len] = APP_FRAME_TAIL;
  
  /* ����֡�� */
  return para_len+14;
}

/**************************************************************************************************
 * @fn          bui_data_msg
 *
 * @brief       �������֡�ṹ����<����֡>����ʽ����Ϊ�ֽ����飬�Ա���ͨ�����緢��.
 *
 * @param       in_msg      - ������ֽ�����,��Ϊ����֡��������
 *              in_msg_len  - ������ֽ����鳤��
 *              out_msg     - ������ֽ�����
 *
 * @return      ������ֽ����鳤��
 **************************************************************************************************
 */
uint8_t bui_data_msg(const uint8_t *in_msg, uint8_t in_msg_len, uint8_t *out_msg)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};
  uint8_t i;
  extern uint8_t last_tx_frm_num;//���͵�֡��

  /* ��ȡ����IP��AP��IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* ���ÿ����룺��ҪӦ�������֡,͸��,���δ��� */
  ctr_code.frame_type = DATA_FRAME;
#ifdef NEED_REPLY_FRAME
  ctr_code.reply_type = NEED_REPLY;
#else
  ctr_code.reply_type = NO_NEED_REPLY;
#endif
  ctr_code.comm_type  = TRANSPARENT_COMM;
  ctr_code.trans_type = ONCE_TRANS;
  ctr_code.comm_dir = ED_TO_AP;

  /* ���֡���� */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  
  /* ���֡������(�Ѹ���) */
  out_msg[6] = last_tx_frm_num;
    
  out_msg[7] = in_msg_len;
  
  /* ��������� */
  for(i=0; i<in_msg_len; i++)
  {
    out_msg[8+i] = in_msg[i];
  }
 
  out_msg[8+in_msg_len] = gen_crc(out_msg, 8+in_msg_len);//CSУ����
  out_msg[9+in_msg_len] = APP_FRAME_TAIL;
  
  /* ����֡�� */
  return 10+in_msg_len;
}


/**************************************************************************************************
 * @fn          bui_switch_state_msg
 *
 * @brief       ֻ�Ը��쿪�صĳ������Ч�����ڻ�ȡ���ص�״̬.
 *
 * @param       comm_type   - ͨ�ŷ�ʽ:
 *                            TRANSPARENT_COMM    - ͸��,���ͽo������������������,ԭ�����
 *                            NO_TRANSPARENT_COMM - ��͸��,���ͽo�������������ڲ�����
 *              in_frame    - �����֡�ṹ��
 *              reply_para  - ���������ؽ���ֽ�����
 *              para_len    - ����ؽ���ֽ����鳤��
 *              out_msg     - ������ֽ�����
 *
 * @return      ������ֽ����鳤��
 **************************************************************************************************
 */
uint8_t bui_switch_state_msg(comm_type_t comm_type, const uint8_t *reply_para, uint8_t para_len, uint8_t *out_msg)
{
  uint8_t i;
  static uint8_t frm_cnt = 1;
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};
  
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  /* ��ȡԭʼ������ */
  ctr_code.code = 0x81;
  
  /* �޸Ŀ����룺����ҪӦ�������֡,�Ƿ�͸���ɲ������� */
  ctr_code.frame_type = ORDER_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type  = comm_type;
  ctr_code.comm_dir = ED_TO_AP;
  ctr_code.trans_type = ONCE_TRANS;

  /* ���֡���� */
  out_msg[0]  = APP_FRAME_HEAD;
  out_msg[1]  = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6]  = frm_cnt++;  
  out_msg[7]  = 4 + para_len;//������Ϊ����Ϊ �����ʶ+��������
  out_msg[8]  = 0xfa;
  out_msg[9]  = 0x80;
  out_msg[10] = 0x01;
  out_msg[11] = 0x00;

  for(i=0; i<para_len; i++)
  {
     out_msg[12+i] = reply_para[i];
  }
  out_msg[12+para_len] = gen_crc(out_msg, 12+para_len);//CSУ����
  out_msg[13+para_len] = APP_FRAME_TAIL;
  
  /* ����֡�� */
  return para_len+14;
}


/**************************************************************************************************
 * @fn          send_reply_msg
 *
 * @brief       ���������ͻظ�֡.
 *
 * @param       in_frame    - �����֡�ṹ��
 *              reply_para  - ���������ؽ���ֽ�����
 *              para_len    - ����ؽ���ֽ����鳤��
 *
 * @return      2���㲥֡����ظ���1,���ͳɹ���0������ʧ�ܡ�
 **************************************************************************************************
 */
uint8_t send_reply_msg(const app_frame_t *in_frame, const uint8_t *reply_para, uint8_t para_len)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //���4�ֽ�IP��ַ
  uint8_t i=0;
  uint8_t out_msg[MAX_APP_PAYLOAD] = {0};
  smplStatus_t rc;
  /* ��ȡ����IP��AP��IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* ��ȡԭʼ������ */
  ctr_code.code = in_frame->ctr_code.code;
  
  /* �޸Ŀ����룺����ҪӦ�������֡,�Ƿ�͸���ɲ������� */
  ctr_code.frame_type = ORDER_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
	if(in_frame->data[2]==0x2C)
  {
    ctr_code.comm_type  = NO_TRANSPARENT_COMM;	
	}
	else
	{
    ctr_code.comm_type  = TRANSPARENT_COMM;
	}
  ctr_code.comm_dir = ED_TO_AP;
  ctr_code.trans_type = ONCE_TRANS;

  /* ���֡���� */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = in_frame->frm_cnt;  
  out_msg[7] = 4 + para_len;//������Ϊ����Ϊ �����ʶ+��������
  for(i=0; i<4; i++)
  {
    out_msg[8+i] = in_frame->data[i];
  }
  for(i=0; i<para_len; i++)
  {
     out_msg[12+i] = reply_para[i];
  }
  out_msg[12+para_len] = gen_crc(out_msg, 12+para_len);//CSУ����
  out_msg[13+para_len] = APP_FRAME_TAIL;
  
  if((((in_frame->ed_addr_1<<8)|in_frame->ed_addr_0) != 0xffff) &&\
     (((in_frame->ed_addr_1<<8)|in_frame->ed_addr_0) != 0x0))
  {
    if(in_frame->ctr_code.comm_type == LOCAL_COMM)
    {
      // SEND_BYTES_TO_UART(out_msg, para_len+14);
			 put_multi_hex_uart1(out_msg, para_len+14);
       return 1;
    }
    else if(in_frame->ctr_code.comm_type == NO_TRANSPARENT_COMM)
    {
      rc = send_net_pkg(sLinkID1, out_msg, para_len+14);
      if(rc == SMPL_SUCCESS)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }
  return 2;
}

/**************************************************************************************************
 * @fn          send_alarm_msg
 *
 * @brief       �Լ�����������֡���и澯��ָʾ֡�����г��ֵĴ���.
 *
 * @param       in_frame    - �����֡�ṹ��
 *              err_info    - ����ĸ澯��Ϣ�ֽ�����
 *              err_len     - �澯��Ϣ�ֽ����鳤��
 *
 * @return      2���㲥֡����澯��1,���ͳɹ���0������ʧ�ܡ�
 **************************************************************************************************
 */
uint8_t send_alarm_msg(const app_frame_t *in_frame, const uint8_t *err_info, uint8_t err_len)
{
  uint8_t i;
  uint8_t out_msg[MAX_APP_PAYLOAD]={0};
  ctr_code_t ctr_code;
  smplStatus_t rc;
  ip_addr_t ipAddr = {0};   //���4�ֽ�IP��ַ
  static uint8_t alarm_frm_cnt=0;//����Ӧ��֡��֡������
  
  /* ��ȡ����IP��AP��IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* ��ȡԭʼ������ */
  if(in_frame != NULL)
  {
    ctr_code.code = in_frame->ctr_code.code;
  }
  else
  {
    ctr_code.code = 0x00;
  }
  
  /* �޸Ŀ����룺����ҪӦ��ĸ澯֡,͸�� */
  ctr_code.frame_type = ALARM_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type = TRANSPARENT_COMM;
  ctr_code.comm_dir = ED_TO_AP;

  /* ���֡���� */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  if(in_frame != NULL)
  {
    out_msg[6] = in_frame->frm_cnt;  
  }
  else
  {
    out_msg[6] = alarm_frm_cnt;  
    alarm_frm_cnt++;
  }
  out_msg[7] = err_len;
  for(i=0; i<err_len; i++)
  {
    out_msg[8+i] = err_info[i];
  }
  out_msg[8+err_len] = gen_crc(out_msg, 8+err_len);//CSУ����
  out_msg[9+err_len] = APP_FRAME_TAIL;

  if(((in_frame->ed_addr_1<<8)|in_frame->ed_addr_0) != 0xffff)
  {
    if(in_frame->ctr_code.comm_type == LOCAL_COMM)
    {
       //SEND_BYTES_TO_UART(out_msg, 10+err_len);
			 put_multi_hex_uart1(out_msg, 10+err_len);
       return 1;
    }
    else if(in_frame->ctr_code.comm_type == NO_TRANSPARENT_COMM)
    {
      rc = send_net_pkg(sLinkID1, out_msg, 10+err_len);
      if(rc == SMPL_SUCCESS)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }

  return 2;
}

/**************************************************************************************************
 * @fn          send_command_msg 
 *
 * @brief       �ڵ��������Ϸ�������֡�����������ʶ������.
 *
 * @param       commandID    - �����ʶ��4���ֽ�
 *              data         - ��������
 *              data_len     - ���ݳ���
 *
 * @return      1�����ͳɹ���2������ʧ�ܡ�
 **************************************************************************************************
 */
uint8_t send_command_msg(uint8_t *commandID, uint8_t *data, uint8_t data_len)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //���4�ֽ�IP��ַ
  uint8_t i;
  uint8_t out_msg[MAX_APP_PAYLOAD] = {0};
  smplStatus_t rc;
  static uint8_t order_frm_cnt = 0;
  /* ��ȡ����IP��AP��IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* �޸Ŀ����룺����ҪӦ�������֡,�Ƿ�͸���ɲ������� */
  ctr_code.frame_type = ORDER_FRAME;
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
  out_msg[6] = order_frm_cnt++;  
  out_msg[7] = 4 + data_len;//������Ϊ����Ϊ �����ʶ+��������
  for(i=0; i<4; i++)
  {
    out_msg[8+i] = commandID[i];
  }
  for(i=0; i<data_len; i++)
  {
     out_msg[12+i] = data[i];
  }
  out_msg[12+data_len] = gen_crc(out_msg, 12+data_len);//CSУ����
  out_msg[13+data_len] = APP_FRAME_TAIL;
  
  rc = send_net_pkg(sLinkID1, out_msg, data_len+14);
  if(rc == SMPL_SUCCESS)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/**************************************************************************************************
 * @fn          send_ack_msg
 *
 * @brief       ����Ӧ��֡.
 *
 * @param       in_frame    - �����֡�ṹ��
 *
 * @return      2���㲥֡����Ӧ��1������Ӧ��ɹ���0������Ӧ��ʧ�ܡ�
 **************************************************************************************************
 */
uint8_t send_ack_msg(const app_frame_t *in_frame)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};
  uint8_t out_msg[MAX_APP_PAYLOAD] = {0};
  smplStatus_t rc;  
  /* ��ȡ����IP��AP��IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr); 
  
  /* ��ȡԭʼ������ */
  ctr_code.code = in_frame->ctr_code.code;
  
  /* �޸Ŀ����룺����ҪӦ���Ӧ��֡ */
  ctr_code.frame_type = REPLY_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_dir = ED_TO_AP;

  /* ���֡���� */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = in_frame->frm_cnt;  
  out_msg[7] = 0;//������Ϊ����Ϊ0
  out_msg[8] = gen_crc(out_msg, 8);//CSУ����
  out_msg[9] = APP_FRAME_TAIL;
  
  if(((in_frame->ed_addr_1<<8)|in_frame->ed_addr_0) != 0xffff)
  {
    if(in_frame->ctr_code.comm_type == LOCAL_COMM)
    {
      // SEND_BYTES_TO_UART(out_msg, 10);
			 put_multi_hex_uart1(out_msg, 10);
       return 1;
    }
    else
    {
      rc = send_net_pkg(sLinkID1, out_msg, 10);
      if(rc == SMPL_SUCCESS)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }
  else
  {
    return 2;
  }
}

/***************************************************************************
 * @fn          bui_pkg_ack
 *     
 * @brief       ����68��88֡��Ӧ��֡
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg     - �����68��88֡
 *              out_pkg    - ���ص��ֽ�����
 *     
 * @return      �ظ�֡����
 ***************************************************************************
 */ 
uint8_t bui_pkg_ack(app_pkg_t *in_pkg,uint8_t *out_pkg)
{
  uint8_t ctr_code;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  ctr_code &= ~BIT0;
  ctr_code |= BIT1;
  *out_pkg = in_pkg->frm_head;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  *(out_pkg+7) = 0;
    
   
  *(out_pkg+8)=gen_crc(out_pkg,8);//CSУ����
  *(out_pkg+9)=0x16; 
  return 10;
}

/***************************************************************************
 * @fn          bui_pkg_alarm
 *     
 * @brief       ����68֡�ĸ澯֡
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg     - �����88֡
 *              err_info   - �澯����
 *              err_len    - �澯��Դ
 *              out_pkg    - ���ص��ֽ�����
 *     
 * @return      �澯֡����
 ***************************************************************************
 */ 
uint8_t bui_pkg_alarm(app_pkg_t *in_pkg,uint8_t *err_info,uint8_t err_len,uint8_t *out_pkg)
{
  uint8_t ctr_code;
  uint8_t i;
  uint8_t k;
  static uint8_t alarm_frm_cnt=0;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  ctr_code |= BIT0;
  ctr_code |= BIT1;
  *out_pkg = in_pkg->frm_head;
  if(in_pkg != NULL)
  {
    *(out_pkg+1) = ctr_code;
    *(out_pkg+2) = in_pkg->ed_addr >>8;
    *(out_pkg+3) = in_pkg->ed_addr;
    *(out_pkg+4) = in_pkg->ap_addr >>8;
    *(out_pkg+5) = in_pkg->ap_addr; 
    *(out_pkg+6) = in_pkg->frm_cnt;  
    *(out_pkg+7) = err_len;
    for(k=0;k<err_len;k++)
      *(out_pkg+8+k) = *(err_info+k);             
    *(out_pkg+8+err_len)=gen_crc(out_pkg,8+err_len);//CSУ����
    *(out_pkg+9+err_len)=0x16;
    return 10+err_len;  
  }
  else
  { 
    alarm_frm_cnt ++;
        
   *(out_pkg+1) = 0x03;
    for(i=0;i<2;i++)
      *(out_pkg+i+2) = 0xee;
    *(out_pkg+4) = 0xff;
    *(out_pkg+5) = 0xff; 
    *(out_pkg+6) = alarm_frm_cnt;   
    //*(out_pkg+7) = 1;
    *(out_pkg+7) = err_len;
    for(k=0;k<err_len;k++)
      *(out_pkg+8+k) = *(err_info+k);             
    *(out_pkg+8+err_len)=gen_crc(out_pkg,8+err_len);//CSУ����
    *(out_pkg+9+err_len)=0x16;
    return 10+err_len;  
  } 
}



uint8_t bui_rep_pkg_command(app_pkg_t *in_pkg,uint8_t *reply_para,uint8_t para_len,uint8_t *out_pkg)
{

  uint8_t ctr_code;
  uint8_t j,k;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  *out_pkg = 0x68;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  *(out_pkg+7) = para_len+4;
   for(j=0;j<4;j++)
    *(out_pkg+8+j) = in_pkg->msg[j];
   for(k=0;k<para_len;k++)
     *(out_pkg+12+k) = *(reply_para+k);
   
  *(out_pkg+12+para_len)=gen_crc(out_pkg,12+para_len);//CSУ����
  *(out_pkg+13+para_len)=0x16;
  
  return para_len+14;
}

/***************************************************************************
 * @fn          bui_app_pkg
 *     
 * @brief       ��֡�ֽ�����ת���ɽṹ�����ݣ����ں�������
 *     
 * @data        2015��08��07��
 *     
 * @param       msg        - ֡�ֽ�����
 *              app_in_pkg - ת����Ľṹ������
 *     
 * @return      void
 ***************************************************************************
 */ 
void bui_app_pkg(uint8_t *msg,app_pkg_t *app_in_pkg)
{
  uint8_t j = 0;

  /*����֡��ʼ��1*/
  app_in_pkg->frm_head  =*msg;//
  /*����֡��ַa0-A1*/
  app_in_pkg->ctr_code  =*(msg+1);
  app_in_pkg->ed_addr   =*(msg+2);
  app_in_pkg->ed_addr   = (app_in_pkg->ed_addr<<8) |  *(msg+3);
  app_in_pkg->ap_addr   =*(msg+4);
  app_in_pkg->ap_addr   =(app_in_pkg->ap_addr<<8) | *(msg+5);
  app_in_pkg->frm_cnt   = *(msg+6);
 
  /*����֡�����򳤶�*/
  app_in_pkg->data_len  =*(msg+7);
 
  /*����֡������*/
  for( j=0;j<app_in_pkg->data_len;j++)
    app_in_pkg->msg[j]  =*(msg+8+j);

  /*����֡У����CS*/
  app_in_pkg->cs=*(msg+8+app_in_pkg->data_len);
 
  /*����֡������*/
  app_in_pkg->frm_end =*(msg+9+app_in_pkg->data_len);

}


/**************************************************************************************************
 * @fn          bui_ed_to_ap_msg
 *
 * @brief       �������֡�ṹ����<����֡>����ʽ����Ϊ�ֽ����飬�Ա���ͨ�����緢��.
 *
 * @param       in_msg      - ������ֽ�����,��Ϊ����֡��������
 *              in_msg_len  - ������ֽ����鳤��
 *              out_msg     - ������ֽ�����
 *
 * @return      ������ֽ����鳤��
 **************************************************************************************************
 */
uint8_t bui_ed_to_ap_msg(const uint8_t *in_msg, uint8_t in_msg_len, uint8_t *out_msg)
{
  memcpy(out_msg,in_msg,in_msg_len);
  out_msg[1] |= 0x80;
  out_msg[in_msg_len-2] += 0x80;
  return in_msg_len;
}
/**************************************************************************************************
 */
