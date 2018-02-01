/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   uart_frm_proc.h
// Description: 处理来自串口的完整网外通信帧(0x69帧).
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-24
// History:     2014-11-8   Leidi 删除未使用的头文件.
*****************************************************************************/

#ifndef __UART_FRM_PROC_H
#define __UART_FRM_PROC_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"
#include "gtway_frm_proc.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void uart_frm_proc(uint8_t *str,uint16_t len);
void reserve_server_ip(const gtway_pkg_t *pIn_pkg);
#endif

/**************************************************************************************************
 */
