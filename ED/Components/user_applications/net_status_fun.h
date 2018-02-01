/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_status_fun.h
// Description: User APP.
//              SWSN����״̬�������������ȡ.
// Author:      Leidi
// Version:     1.0
// Date:        2012-3-24
// History:     2014-9-28   Leidi �޸ĸ���������,ʹ������app_frame_t. 
//                                ɾ�������ͷ�ļ�.
//              2014-9-29   Leidi ����app_net_status_t�Ķ�����bui_pkg_fun.h���뱾�ļ�.
//              2014-10-5   Leidi ����ö������app_net_status_t�ĳ�Ա.
//                                ���Ӻ���ԭ��no_status_fun().
//              2015-07-29  Leidi ����ͷ�ļ�bui_pkg_fun.h.
*****************************************************************************/

#ifndef _NET_STATUS_FUN_H
#define _NET_STATUS_FUN_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bui_pkg_fun.h"

/* ------------------------------------------------------------------------------------------------
 *                                             Types
 * ------------------------------------------------------------------------------------------------
 */
typedef enum app_net_status{
  APP_NET_STATUS_SUCCESS,
  NO_STATUS_FUN_FAIL,
  APP_RESET_SYSTEM_FAIL,
  APP_GET_NET_INFO_FAIL,
  APP_GET_ED_INFO_FAIL,
  APP_GET_JOIN_LINK_TOKEN_FAIL,
  APP_SET_JOIN_TOKEN_FAIL,
  APP_SET_LINK_TOKEN_FAIL,
  APP_GET_AP_MAC_FAIL,
  APP_PROC_TIME_SYNC_FAIL,
  APP_GET_ED_MAC_AND_IP_FAIL,
  APP_MODIFY_IP_ADDR_FAIL,
  APP_DELETE_ED_CONNECT_FAIL,
} app_net_status_t;


/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
app_status_t net_status_proc(const app_frame_t *in_frame);

app_net_status_t no_status_fun(const app_frame_t *in_frame);
app_net_status_t get_join_link_token(const app_frame_t *in_frame);
app_net_status_t set_join_token(const app_frame_t *in_frame);
app_net_status_t set_link_token(const app_frame_t *in_frame);
app_net_status_t get_ed_info(const app_frame_t *in_frame);
app_net_status_t get_ed_mac(const app_frame_t *in_frame);
app_net_status_t set_datetime(const app_frame_t *in_frame);
app_net_status_t time_sync_proc(const app_frame_t *in_frame);
app_net_status_t time_sync_space_proc(const app_frame_t *in_frame);
app_net_status_t get_ed_mac_and_ip(const app_frame_t *in_frame);
app_net_status_t modify_ip_addr(const app_frame_t *in_frame);
app_net_status_t delete_ed_connect(const app_frame_t *in_frame);


#endif

/**************************************************************************************************
 */
