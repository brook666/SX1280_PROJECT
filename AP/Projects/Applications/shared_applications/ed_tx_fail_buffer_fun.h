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
#ifndef _ED_TX_FAIL_BUFFER_FUN_H
#define _ED_TX_FAIL_BUFFER_FUN_H

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
 /* 节点发送失败缓冲区状态枚举 */
typedef enum {
  TX_FAIL_BUFFER_AVALIABLE,
  TX_FAIL_BUFFER_IN_USE,
}tx_fail_buffer_state_t;

 /* 操作节点缓冲区后的状态枚举 */
typedef enum {
  TX_FAIL_BUFFER_SUCCESS,
  TX_FAIL_BUFFER_FREE_FAIL,
}tx_fail_buffer_result_t;


/* 单个节点的接收缓冲区 */
typedef struct
{
  volatile tx_fail_buffer_state_t  usage;
           uint8_t                 lid;
           uint8_t                 sendTimes;
           uint8_t                 ed_tx_fail_info[MAX_APP_PAYLOAD];
           uint8_t                 ed_tx_fail_info_len;
} ed_tx_fail_buffer_t;

/* ------------------------------------------------------------------------------------------------
 *                                            Externs
 * ------------------------------------------------------------------------------------------------
 */
extern ed_tx_fail_buffer_t   Ed_tx_fail_buffer[SIZE_INFRAME_Q];
extern volatile uint8_t tx_fail_frm_num ;
extern volatile uint8_t emergency_dispatch_flag ; 
extern volatile uint8_t start_emergency_dispatch_in_rtc ;
/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void Write_tx_fail_ed_buffer(uint8_t lid,uint8_t *pIn_msg, uint8_t* pIn_len);
void Iterates_tx_fail_ed_buffer(void);
void Find_ed_info_in_tx_fail_buffer(void);

#endif

/**************************************************************************************************
 */

