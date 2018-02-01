/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ed_rx_buffer_fun.h
// Description: User APP.
//              节点收发缓冲区处理头文件.
// Author:      Leidi
// Version:     1.1
// Date:        2015-09-28
// History:     2015-09-28  Zengjia 初始版本建立.
*****************************************************************************/
#ifndef _ED_RX_BUFFER_FUN_H
#define _ED_RX_BUFFER_FUN_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <string.h>
#include "bsp.h" 
#include "68_frm_proc.h"
#include "bui_pkg_fun.h"
#include "net_frm_proc.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Typedefs
 * ------------------------------------------------------------------------------------------------
 */
 /* 节点缓冲区状态枚举 */
typedef enum {
  RX_BUFFER_AVALIABLE,
  RX_BUFFER_IN_USE,
}rx_buffer_state_t;

 /* 操作节点缓冲区后的状态枚举 */
typedef enum {
  RX_BUFFER_SUCCESS,
  RX_BUFFER_FREE_FAIL,
}rx_buffer_result_t;


/* 单个节点的接收缓冲区 */
typedef struct
{
  volatile rx_buffer_state_t  usage;  //节点状态
           linkID_t           lid;//LINkID
           uint8_t            ed_rx_info[MAX_APP_PAYLOAD]; //信息缓冲区
           uint8_t            ed_rx_info_len; //信息长度
} ed_rx_buffer_t;

/* ------------------------------------------------------------------------------------------------
 *                                            Externs
 * ------------------------------------------------------------------------------------------------
 */
 extern ed_rx_buffer_t   Ed_rx_buffer[SIZE_INFRAME_Q];
 extern volatile uint8_t app_frm_num ;

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
uint8_t Write_rx_ed_buffer(uint8_t lid);
void Iterates_rx_ed_buffer(void);
void Find_ed_info_in_rx_buffer(void);

#endif

/**************************************************************************************************
 */

