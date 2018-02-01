/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_status_fun.h
// Description: User APP.
//              SWSN网络状态处理.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-21
// History:     2015-05-13  Leidi 增加函数原型set_join_ed().
*****************************************************************************/
#ifndef _NET_STATUS_FUN_H
#define _NET_STATUS_FUN_H

#include "bsp.h"
#include "nwk_layer.h"
#include "bui_pkg_fun.h"

typedef enum app_net_status{
  APP_NET_STATUS_SUCCESS,
  NO_STATUS_FUN_FAIL,
  APP_RESET_SYSTEM_FAIL,
  APP_GET_NET_INFO_FAIL,
  APP_GET_ED_INFO_FAIL,
  APP_THIS_ED_IS_NOT_FOUND,
  APP_GET_JOIN_LINK_TOKEN_FAIL,
  APP_GET_AP_MAC_FAIL,
  APP_GET_NET_MAC_FAIL,
  APP_SET_JOIN_ED_FAIL,
  APP_RESET_AP_FAIL,
  APP_TIME_SYNC_PROC_FAIL,
  APP_TIME_SYNC_SPACE_PROC,
  APP_GET_ED_MAC_AND_IP_FAIL,
  APP_MODIFY_IP_ADDR_FAIL,
  APP_DELETE_ED_CONNECT_FAIL,
  APP_IP_CONFLICT,
}app_net_status_t;

 
app_status_t net_status_proc(app_pkg_t *,uint8_t *,uint8_t *);
app_net_status_t reset_system(app_pkg_t *,uint8_t *,uint8_t *);
app_net_status_t get_net_info(app_pkg_t *,uint8_t *,uint8_t *);
app_net_status_t get_ed_info(app_pkg_t *,uint8_t *,uint8_t *);
app_net_status_t get_join_link_token(app_pkg_t *,uint8_t *,uint8_t *);
app_net_status_t get_ap_mac(app_pkg_t *,uint8_t *,uint8_t *);
app_net_status_t set_join_ed(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len);
app_net_status_t reset_ap(app_pkg_t *,uint8_t *, uint8_t *);
app_net_status_t get_ed_mac_and_ip (app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len);
app_net_status_t modify_ip_addr(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len);
app_net_status_t delete_ed_connect(app_pkg_t *,uint8_t *, uint8_t *);
app_net_status_t no_status_fun(app_pkg_t *,uint8_t *, uint8_t *);
app_net_status_t read_temoter_press_num(app_pkg_t *,uint8_t *, uint8_t *);

extern uint8_t reset_system_flag ;
#endif

