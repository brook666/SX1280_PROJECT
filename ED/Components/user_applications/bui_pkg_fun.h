/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   bui_pkg_fun.h
// Description: User APP.
//              SWSN����֡�Ĺ��������.
// Author:      Leidi
// Version:     2.0
// Date:        2012-3-24
// History:     2014-9-27   Leidi ��������frame_type_t,frame_type_t,comm_type_t,trans_type_t,reply_type_t,app_frame_t. 
//                                �����궨��APP_FRAME_HEAD,APP_FRAME_TAIL.
//                                ��������ԭ��bui_app_frame(),bui_reply_msg(),bui_alarm_msg(),bui_order_msg().
//              2014-9-28   Leidi ��������ԭ��bui_data_msg().
//              2014-9-29   Leidi ����app_user_def_t�Ķ����ɱ��ļ�����user_def_fun.h. 
//                                ����app_net_status_t�Ķ����ɱ��ļ�����net_status_fun.h.
//                                ����app_net_para_t�Ķ����ɱ��ļ�����net_para_fun.h.
//                                ɾ����������app_sys_t.
//              2014-9-30   Leidi ɾ�����º���ԭ��:bui_app_pkg(),bui_rep_pkg_command(),bui_pkg_ack(),
//                                bui_pkg_alarm(),bui_68_frm(),bui_ap_inner_command_pkg().
//                                ɾ������app_pkg_t.
//                                ������õ�ͷ�ļ�bsp.h.
//                                ����gen_crc()��������.c�ļ��в���static����.
//                                ���ע��.
//              2014-10-29  Leidi ���ͨ�ŷ����ö������comm_dir_t.
//                                ����ctr_code_t��,��reverse��Ϊcomm_dir.
//              2015-08-07  Leidi �޸�֡��ʽ:��������ַ��Ϊ2�ֽ�,֡��������Ϊ1�ֽ�.
*****************************************************************************/

#ifndef bui_pkg_fun_h
#define bui_pkg_fun_h

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "nwk_types.h"
#include "swsn_lib.h"

#define GET_REPLY_REQUEST_FLAG(msg) ((uint8_t)(msg[1] & 0x40));
#define GET_COMM_TYPE(msg)    ((uint8_t)(msg[1] & 0x0C));
#define GET_FRAME_TYPE(msg)   ((uint8_t)(msg[1] & 0x03));
#define GET_COMMANA_ID_DOMAIN(msg)    ((uint8_t)(msg[9]));
/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
 
 typedef struct
{
  uint8_t  frm_head;
  uint8_t  ctr_code;
  uint16_t ed_addr;
  uint16_t ap_addr;
  uint8_t  frm_cnt;
  uint8_t  data_len;
  uint8_t  msg[MAX_APP_MSG];
  uint8_t  cs;
  uint8_t  frm_end;
} app_pkg_t;
 
 
app_status_t bui_app_frame(const uint8_t *in_msg, uint8_t in_msg_len, app_frame_t *out_frame);
uint8_t bui_reply_msg(const app_frame_t *in_frame, uint8_t *out_msg);
uint8_t bui_alarm_msg(const app_frame_t *in_frame,const uint8_t *err_info, uint8_t err_len, uint8_t *out_msg);
uint8_t bui_order_msg(comm_type_t comm_type, const app_frame_t *in_frame, const uint8_t *reply_para, uint8_t para_len, uint8_t *out_msg);
uint8_t bui_data_msg(const uint8_t *in_msg, uint8_t in_msg_len, uint8_t *out_msg);
uint8_t bui_switch_state_msg(comm_type_t comm_type, const uint8_t *reply_para, uint8_t para_len, uint8_t *out_msg);
uint8_t gen_crc(const uint8_t* , uint8_t );
uint8_t send_ack_msg(const app_frame_t *in_frame);
uint8_t bui_ed_to_ap_msg(const uint8_t *in_msg, uint8_t in_msg_len, uint8_t *out_msg);

uint8_t bui_pkg_alarm(app_pkg_t *,uint8_t *,uint8_t ,uint8_t *);
uint8_t bui_rep_pkg_command(app_pkg_t *in_pkg,uint8_t *reply_para,uint8_t para_len,uint8_t *out_pkg);

void bui_app_pkg(uint8_t *,app_pkg_t *);
uint8_t bui_pkg_ack(app_pkg_t *,uint8_t *);
#endif

