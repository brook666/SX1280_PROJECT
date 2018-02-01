/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ap_inner_fun.c
// Description: 处理节点发个网关内部处理的帧.
// Author:      Leidi
// Version:     1.0
// Date:        2014-3-24
// History:     2014-10-24  Leidi 在函数ap_inner_proc()中,由节点上发的帧的处理输出
//                                不在本函数中进行,而在其调用者处进行.
*****************************************************************************/ 
#include "bsp.h"
#include "bui_pkg_fun.h"
#include "68_frm_proc.h"
#include "ap_inner_fun.h"
#include "net_status_fun.h"
#include "net_para_fun.h"
#include "user_def_fun.h"

/***************************************************************************
 * @fn          ap_inner_proc
 *     
 * @brief       集中器处理网络帧。网络帧根据控制码的不同，comm_type=0x00或者0x01
 *              则透传给串口；comm_type == 0x04,则是集中器内部需要处理的帧。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度      
 *     
 * @return      APP_SUCCESS - 成功
 ***************************************************************************
 */ 
app_status_t ap_inner_proc(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  uint8_t  msg_id_1= in_pkg->msg[1]; 

	
  if(msg_id_1 == 0x01)
  {
    /* 处理ED上发的网络状态命令帧 */
    net_para_proc(in_pkg, out_pkg, len);     
  }	
  if(msg_id_1 == 0x02)
  {
    /* 处理ED上发的网络状态命令帧 */
    net_status_proc(in_pkg, out_pkg, len);     
  }
  else if(msg_id_1 == 0x80)
  {
    user_def_proc(in_pkg, out_pkg, len); 
  }  
  return APP_SUCCESS;
}


/*****************************************************************************/






