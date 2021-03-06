/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   uart_frm_proc.h
// Description: User APP.
//              处理从串口输入的帧.
// Author:      Leidi
// Version:     1.0
// Date:        2012-3-24
// History:     2014-9-30   Leidi 修无用的头文件bui_pkg_fun.h和函数原型uart_pkg_proc(app_pkg_t *).
//                                将私有的函数原型放回.c文件中并用static修饰.
//              2015-07-29  Leidi 引入头文件bui_pkg_fun.h.
*****************************************************************************/

#ifndef __UART_FRM_PROC_H
#define __UART_FRM_PROC_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bui_pkg_fun.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */


 
void uart_frm_proc(uint8_t *,uint8_t );
void uart2_frm_proc(uint8_t *,uint16_t );
smplStatus_t send_net_pkg(linkID_t lid, uint8_t *,uint8_t );
smplStatus_t send_bcast_pkg(uint8_t *, uint8_t );

#endif
