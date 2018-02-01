/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   swsn_types.h
// Description: 应用层数据的基本类型.
// Author:      Zengjia
// Version:     1.0
// Date:        2015-12-24
// History:     2015-12-24  Zengjia 初始版本建立.
*****************************************************************************/
#pragma anon_unions

#ifndef _SWSN_TYPES_H
#define _SWSN_TYPES_H

//包含stm8l外设驱动的头文件
//#include "stm8l15x_conf.h"
#include "stdint.h"
#include "smpl_nwk_config.h"
/* ------------------------------------------------------------------------------------------------
 *                                             Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAX_APP_MSG (MAX_APP_PAYLOAD-10)        //应用层帧的数据域最大长度

#define APP_FRAME_HEAD 0x68   //应用层帧的帧头
#define APP_FRAME_TAIL 0x16   //应用层帧的帧尾


/* ------------------------------------------------------------------------------------------------
 *                                             Types
 * ------------------------------------------------------------------------------------------------
 */
/* 帧类型定义 */
typedef enum {
  DATA_FRAME  = 0x00, //数据帧
  ORDER_FRAME = 0x01, //命令帧
  REPLY_FRAME = 0x02, //应答帧
  ALARM_FRAME = 0x03, //告警帧
} frame_type_t;

/* 通信方式定义 */
typedef enum {
  TRANSPARENT_COMM    = 0x00, //远程访问,数据透传
  NO_TRANSPARENT_COMM = 0x01, //远程访问,数据不透传
  LOCAL_COMM          = 0x02, //本地访问
  RESERVE_COMM        = 0x03, //未定义方式
} comm_type_t;

/* 传输状态定义 */
typedef enum {
  ONCE_TRANS  = 0x00, //单次传输
  FIRST_TRANS = 0x01, //起始帧
  LAST_TRANS  = 0x02, //结束帧
  MIDDLE_TRANS= 0x03, //中间帧
} trans_type_t;

/* 应答请求定义 */
typedef enum {
  NO_NEED_REPLY = 0x00, //不需要应答
  NEED_REPLY    = 0x01, //需要应答
} reply_type_t;

/* 通信方向定义 */
typedef enum {
  AP_TO_ED = 0x00, //集中器->节点
  ED_TO_AP = 0x01, //节点->集中器
} comm_dir_t;

/* 控制码定义 */
typedef union {
  struct
  {
    uint8_t frame_type  : 2;  //帧类型
    uint8_t comm_type   : 2;  //通信方式
    uint8_t trans_type  : 2;  //传输状态
    uint8_t reply_type  : 1;  //应答请求
    uint8_t comm_dir    : 1;  //通信方向
  };
  uint8_t code;
} ctr_code_t;

/* 应用层帧结构定义 */
typedef struct
{
  uint8_t frm_head;           //帧头
  ctr_code_t ctr_code;        //控制码
  uint8_t ed_addr_1;          //节点地址高字节
  uint8_t ed_addr_0;          //节点地址低字节
  uint8_t ap_addr_1;          //集中器地址高字节
  uint8_t ap_addr_0;          //集中器地址低字节
  uint8_t frm_cnt;            //帧计数器
  uint8_t data_len;           //数据域长度
  uint8_t data[MAX_APP_MSG];  //数据域
  uint8_t cs;                 //校验码
  uint8_t frm_tail;           //帧尾
} app_frame_t;

/* 日期时间结构体 */
typedef struct
{  
  uint16_t year;
  uint8_t month; 
  uint8_t day; 
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint16_t msec;
  uint8_t week;
}daytime_t;

/* 应用层操作执行结果定义 */
typedef enum app_status{
  APP_ERR,            //执行失败
  APP_SUCCESS,        //执行成功
} app_status_t;

typedef app_status_t (*pAppFunc)(const app_frame_t *in_frame);
typedef struct
{
  uint8_t commandId_DI2;
  pAppFunc pFunx;
}app_interface_t;

#define MAX_APP_FUNC 10
extern app_interface_t appFuncDomain[MAX_APP_FUNC];






#endif


