/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   bui_pkg_fun.h
// Description: User APP.
//              内部帧的构建与解析处理头文件.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-21
// History:     2014-10-21  Leidi 加入获取控制码,节点IP,命令标识,告警信息的宏.
//              2014-10-27  Leidi 增加函数原型Debui_app_pkg().
//              2015-05-13  Leidi 在枚举类型app_net_status中添加APP_NET_STATUS_FAIL.
*****************************************************************************/

#ifndef bui_pkg_fun_h
#define bui_pkg_fun_h

/************************************************************
************************************************************/

#include "bsp.h"
#include "nwk_layer.h"

//#define MAX_APP_MSG 40
#define MAX_APP_MSG 200
#define MAX_77_MSG 2037

/* 内部使用帧的帧尾 */
#define INNER_FRAME_TAIL 0x16

/* 内部使用帧的控制码的设置与获取 */
#define GET_CONTROL_CODE(msg) ((uint8_t)(msg[1]))
#define SET_CONTROL_CODE(msg, x)  (msg[1] = x)
#define GET_FRAME_TYPE(msg)   ((uint8_t)(msg[1] & 0x03))
#define GET_COMM_TYPE(msg)    ((uint8_t)(msg[1] & 0x0C))
#define GET_REPLY_REQUEST_FLAG(msg) ((uint8_t)(msg[1] & 0x40))
#define CLEAN_REPLY_REQUEST_FLAG(msg) (msg[1] &= 0xbf)

/* 内部使用帧的其他字段设置于获取 */
#define GET_FRAME_HEAD(msg)   ((uint8_t)(msg[0]))
#define GET_ED_IP(msg)      (((uint16_t)(msg[2])<<8) | (msg[3]))
#define GET_DATA_LEN(msg)   ((uint8_t)(msg[7]))
#define SET_DATA_LEN(msg, x)    (msg[7] = x)
#define GET_FRAME_LEN(msg)  ((uint8_t)(msg[7]) + 10)

/* 命令帧的相关字段的获取 */
#define GET_COMMANA_ID_SYSTME(msg)    ((uint8_t)(msg[8]))
#define GET_COMMANA_ID_DOMAIN(msg)    ((uint8_t)(msg[9]))
#define GET_COMMANA_ID_FUN(msg)     ((uint8_t)(msg[10]))
#define GET_COMMANA_ID_RW(msg)      ((uint8_t)(msg[11]))

/* 命令帧的相关字段的获取 */
//#define GET_ALARM_SOURCE(msg)   ((uint8_t)(msg[8]))
//#define GET_ALARM_CONTENT(msg)    ((uint8_t)(msg[9]))

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

typedef struct
{
  uint8_t frm_head;
  uint8_t ctr_code;
  uint16_t ed_addr;
  uint16_t ap_addr;
  uint8_t frm_cnt;
  uint16_t data_len;
  uint8_t msg[MAX_77_MSG];
  uint8_t cs;
  uint8_t frm_end;
} app_77_pkg_t;

enum app_status{
  APP_ERR,
  APP_SUCCESS,
};

typedef enum app_status app_status_t;

enum app_sys{
  APP_SYS_SUCCESS,
  APP_NO_THIS_IP,
};

typedef enum app_sys app_sys_t;


enum app_ap_inner{
  APP_AP_INNER_SUCCESS,
 
};

typedef enum app_ap_inner app_ap_inner_t;


void bui_app_pkg(uint8_t *,app_pkg_t *);
void bui_77_app_pkg(uint8_t *,app_77_pkg_t *);

uint8_t bui_rep_pkg_command(app_pkg_t *,uint8_t *,uint8_t ,uint8_t *);
uint8_t bui_rssi_rep_pkg_command(app_pkg_t *,uint8_t *,uint8_t ,uint8_t *);
uint8_t bui_rep_66_command(app_pkg_t *,uint8_t *,uint8_t ,uint8_t *);
uint16_t bui_rep_77_command(app_77_pkg_t *,uint8_t *,uint16_t ,uint8_t *);
uint8_t bui_rep_88_command(app_pkg_t *,uint8_t *,uint8_t ,uint8_t *);
uint8_t bui_pkg_ack(app_pkg_t *,uint8_t *);
uint8_t bui_pkg_alarm(app_pkg_t *,uint8_t *,uint8_t ,uint8_t *);
uint8_t gen_crc(uint8_t* , uint16_t );
uint8_t bui_check_link_pkg(uint8_t , uint8_t *,uint8_t *);
uint8_t bui_time_sync_pkg(uint8_t *);
uint8_t Debui_app_pkg(const app_pkg_t *pApp_in_pkg, uint8_t new_ctr_code, uint8_t *pOut_msg);
uint8_t bui_77_pkg_ack(app_77_pkg_t *,uint8_t *);
#endif
