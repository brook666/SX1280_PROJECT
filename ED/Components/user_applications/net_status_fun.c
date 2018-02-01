/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_status_fun.c
// Description: User APP.
//              SWSN网络状态参数的设置与获取.
// Author:      Leidi
// Version:     2.0
// Date:        2012-3-24
// History:     2014-9-28   Leidi 修改各函数参数,使用类型app_frame_t. 
//                                删除冗余的头文件和全局变量.
//              2014-10-5   Leidi 告警帧改由net_status_proc()构建,而不是其子函数.
//                                增加函数定义no_status_fun().
//              2014-10-27  Leidi 函数modify_ip_addr()中的返回帧改为透传(集中器不处理).
//              2014-10-29  Leidi 函数delete_ed_connect()中的返回帧改为透传(集中器不处理).
//              2014-11-10  Leidi 移除头文件stm8s_uart.h.
//                                添加用于配置的头文件user_app_config.h.
//              2015-05-30  Leidi 删除全局变量time_sync_cnt,real_time,sync_time.
//                                将函数get_time_sync()改名为set_datetime().
//              2015-07-29  Leidi 删除引用头文件user_def_fun.h,nwk_types.h.
//                                删除函数time_sync_space_proc().
//                                删除对全局变量sPersistInfo,myIpAddr的引用,增加对sLinkID1的引用.
//              2015-07-30  Leidi 将引用的全局变量time_sync_switch,sLinkID1声明移入函数内部.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "net_status_fun.h"
#include "bsp_mac_addr.h"
#include "nwk_token.h"
#include "nwk.h"
#include "nwk_globals.h"
#include "nwk_api.h"
#include "stm32_rtc.h"
#include <string.h>
#include <stdlib.h>
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
/* 节点是否已经被AP删除的标志 */
uint8_t delete_ed_connect_flag = FALSE;

/**************************************************************************************************
 * @fn          net_status_proc
 *
 * @brief       处理网络状态参数的上层函数,具体处理过程交由其子函数.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_ERR     - 处理网络状态参数失败
 *              APP_SUCCESS - 处理网络状态参数成功
 **************************************************************************************************
 */
app_status_t net_status_proc(const app_frame_t *in_frame)
{
  uint8_t alarm_info[4];
  app_net_status_t  status_rc = APP_NET_STATUS_SUCCESS;

  switch(in_frame->data[2])
  {
    case 0x03:  status_rc = get_join_link_token(in_frame);  //读取join Token和link Token
                break; 
    case 0x04:  status_rc = set_join_token(in_frame);     //设置join Token
                break;
    case 0x05:  status_rc = set_link_token(in_frame);     //设置link Token
                break;        
    case 0x20:  status_rc = get_ed_mac(in_frame);     //读取默认的MAC地址
                break;
    case 0x30:  status_rc = set_datetime(in_frame);   //设置日期时间
                break;
    case 0x31:  status_rc = time_sync_proc(in_frame); //设置心跳帧检测
                break;
    case 0x80:  status_rc = get_ed_info(in_frame);    //查询节点的连接信息表项
                break;
    case 0x90:  status_rc = get_ed_mac_and_ip(in_frame);//查询节点的MAC和IP
                break;
    case 0x91:  status_rc = modify_ip_addr(in_frame);   //修改节点IP地址
                break;
    case 0x92:  status_rc = delete_ed_connect(in_frame);//删除目标节点
                break;
    default:    status_rc = no_status_fun(in_frame);
  }

  if(status_rc == APP_NET_STATUS_SUCCESS)
  {
    return APP_SUCCESS;
  }
  else
  {
    alarm_info[0]=0x00;
    alarm_info[1]=0x01;
    alarm_info[2]=0x01;
    alarm_info[3]=0x08;
    send_alarm_msg(in_frame, alarm_info, 4);
    
    return APP_ERR;
  }
}

/**************************************************************************************************
 * @fn          no_status_fun
 *
 * @brief       未使用的组网状态设置函数.
 *
 * @param       in_frame  - 输入的帧结构体
 *              out_msg   - 输出的字节数组
 *              out_len   - 输出的字节数组长度
 *
 * @return      NO_STATUS_FUN_FAIL     - 处理组网状态设置函数失败
 **************************************************************************************************
 */
app_net_status_t no_status_fun(const app_frame_t *in_frame)
{
  app_net_status_t rc = NO_STATUS_FUN_FAIL;
   
  return rc;
}
/**************************************************************************************************
 * @fn          get_join_link_token
 *
 * @brief       读取join Token和link Token.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_GET_JOIN_LINK_TOKEN_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS           - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t get_join_link_token(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_GET_JOIN_LINK_TOKEN_FAIL;
  uint8_t  msg_id_0 = in_frame->data[3];
  
  if(msg_id_0 == 0x00)
  {
    uint8_t  join_link_token[8];
    ioctlToken_t link ;
    ioctlToken_t join ;
    
    join.tokenType = TT_JOIN;
    link.tokenType = TT_LINK;
    
    SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_GET,(void *)&join);
    SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_GET,(void *)&link);
    
    join.token.joinToken = swsn_htonl(join.token.joinToken);
    link.token.linkToken = swsn_htonl(link.token.linkToken);
    
    memcpy(&join_link_token[0], &join.token.joinToken, sizeof(join.token.joinToken));//加入令牌(如FE 08 01 05)
    memcpy(&join_link_token[4], &link.token.linkToken, sizeof(link.token.linkToken));//连接令牌(如05 04 03 02)
    send_reply_msg(in_frame, join_link_token, 8);
    
    rc = APP_NET_STATUS_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_GET_JOIN_LINK_TOKEN_FAIL;	
  }
  
  return rc;
}
/**************************************************************************************************
 * @fn          set_join_token
 *
 * @brief       设置join Token.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_SET_JOIN_TOKEN_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS  - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t set_join_token(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_SET_JOIN_TOKEN_FAIL;
  uint8_t  msg_id_0 = in_frame->data[3];
  
  if(msg_id_0 == 0x00)
  {
    ioctlToken_t join ;
    
    join.tokenType = TT_JOIN;
    join.token.joinToken = in_frame->data[3] | \
                          ((in_frame->data[4]&0xffffffff)<<8) | \
                          ((uint32_t)(in_frame->data[5]&0xffffffff)<<16) | \
                          ((uint32_t)(in_frame->data[6]&0xffffffff)<<24);  
    
    SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_SET,(void *)&join);
    WriteJoinTokenToFlash(join.token.joinToken);
    
    send_reply_msg(in_frame, NULL, 0);
    
    rc = APP_NET_STATUS_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_SET_JOIN_TOKEN_FAIL;	
  }
  
  return rc;
}

/**************************************************************************************************
 * @fn          set_link_token
 *
 * @brief       设置link Token.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_GET_LINK_TOKEN_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS  - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t set_link_token(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_SET_LINK_TOKEN_FAIL;
  uint8_t  msg_id_0 = in_frame->data[3];
  
  if(msg_id_0 == 0x00)
  {
    ioctlToken_t join ;
    
    join.tokenType = TT_LINK;
    join.token.linkToken = in_frame->data[3] | \
                          ((in_frame->data[4]&0xffffffff)<<8) | \
                          ((uint32_t)(in_frame->data[5]&0xffffffff)<<16) | \
                          ((uint32_t)(in_frame->data[6]&0xffffffff)<<24);  
    
    SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_SET,(void *)&join);
    WriteLinkTokenToFlash(join.token.linkToken);
    
    send_reply_msg(in_frame, NULL, 0);
    
    rc = APP_NET_STATUS_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_SET_LINK_TOKEN_FAIL;	
  }
  
  return rc;
}
/**************************************************************************************************
 * @fn          get_ed_mac
 *
 * @brief       读取默认的MAC地址.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_GET_AP_MAC_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS  - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t get_ed_mac(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_GET_AP_MAC_FAIL;
  uint8_t 		   msg_id_0 = in_frame->data[3];
  uint8_t 		   ed_mac[4]; 
  addr_t         myromaddr = THIS_DEVICE_ADDRESS;	  
  uint8_t i;
  if(msg_id_0 == 0x00)
  {
    for(i = 0; i < 4; i++)
    {
      ed_mac[i] = myromaddr.addr[3 - i];
    }
    
    send_reply_msg(in_frame, ed_mac, 4);
    
    rc = APP_NET_STATUS_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_GET_AP_MAC_FAIL;	
  }
  
  return rc;
}

/**************************************************************************************************
 * @fn          set_datetime
 *
 * @brief       设置日期时间.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_PROC_TIME_SYNC_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS      - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t set_datetime(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_PROC_TIME_SYNC_FAIL;
  uint8_t 	       msg_id_0 = in_frame->data[3];    
  daytime_t        daytime_s, daytime_g;
  
  if(msg_id_0 == 0x01)
  {
    daytime_s.year  = in_frame->data[4];
    daytime_s.month = in_frame->data[5];
    daytime_s.day   = in_frame->data[6];
    daytime_s.week  = in_frame->data[11];
    
    daytime_s.hour   = in_frame->data[7];
    daytime_s.minute = in_frame->data[8];
    daytime_s.second = in_frame->data[9];
    
    /* 获取当前日期时间 */
    Time_Get((ap_time_t *)&daytime_g);
    
    /* 只有当偏差过大时才更新日期时间 */
    if(abs(daytime_s.second - daytime_g.second) > 5 ||
       daytime_s.minute != daytime_g.minute ||
       daytime_s.hour != daytime_g.hour ||
       daytime_s.week != daytime_g.week)
    {
      /* 设置日期时间 */
      Time_Set((uint8_t *)&daytime_s,7);
    }
    
    rc= APP_NET_STATUS_SUCCESS;
  }
  
  return rc;  
}

/**************************************************************************************************
 * @fn          time_sync_proc
 *
 * @brief       设置心跳帧检测.
 *
 * @param       in_frame  - 输入的帧结构体
 *              out_msg   - 输出的字节数组
 *              out_len   - 输出的字节数组长度
 *
 * @return      APP_PROC_TIME_SYNC_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS      - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t time_sync_proc(const app_frame_t *in_frame)
{
  app_net_status_t  rc = APP_PROC_TIME_SYNC_FAIL;
  uint8_t 	        msg_id_0 = in_frame->data[3];
  uint8_t           time_sync[1];
  extern uint8_t    time_sync_switch; //心跳帧检测开关
  
  if(msg_id_0 == 0x00)
  {
    time_sync[0] = time_sync_switch;
    
    send_reply_msg(in_frame, time_sync, 1);
    
    rc= APP_NET_STATUS_SUCCESS; 
  }
  else if(msg_id_0 == 0x01)
  {
    time_sync[0] = in_frame->data[4];
    time_sync_switch = time_sync[0];
    
    send_reply_msg(in_frame, time_sync, 1);
    
    rc= APP_NET_STATUS_SUCCESS;
  }
  
  return rc;  
}

/**************************************************************************************************
 * @fn          get_ed_info
 *
 * @brief       查询节点的连接信息表项.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_GET_NET_INFO_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS      - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t get_ed_info(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_GET_NET_INFO_FAIL;
  uint8_t 		   msg_id_0 = in_frame->data[3];
  uint8_t 		   ed_info[9]; 
  extern linkID_t  sLinkID1;   //连接标示符,用于获取连接信息

  if(msg_id_0 == 0x00)
  {
    /* 获取连接信息 */
    connInfo_t *pCInfo = nwk_getConnInfo(sLinkID1);
    if(pCInfo != NULL)
    {
      ed_info[0] =  pCInfo->thisLinkID;
      ed_info[1] =  pCInfo->connState;
      ed_info[2] =  pCInfo->hops2target;
      ed_info[3] =  pCInfo->peerAddr[0];
      ed_info[4] =  pCInfo->peerAddr[1];
      ed_info[5] =  pCInfo->sigInfo.rssi;
      ed_info[6] =  pCInfo->sigInfo.lqi;
      
      send_reply_msg(in_frame, ed_info, 7);
    
      rc= APP_NET_STATUS_SUCCESS;
    }
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_GET_NET_INFO_FAIL;
  }

  return rc;
}

/**************************************************************************************************
 * @fn          get_ed_mac_and_ip
 *
 * @brief       查询节点的MAC和IP.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_GET_ED_MAC_AND_IP_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS      - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t get_ed_mac_and_ip(const app_frame_t *in_frame)
{
  app_net_status_t  rc = APP_GET_ED_MAC_AND_IP_FAIL;
  uint8_t           msg_id_0 = in_frame->data[3]; 
  uint8_t           mac_and_ip[6];
  uint8_t i;     
  if(msg_id_0 == 0x00)
  {
    addr_t *macAddr = (addr_t *)nwk_getMyAddress();//获得4字节MAC地址
    ip_addr_t ipAddr  = {0};   //获得4字节IP地址
    
    SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
    
    for(i=0;i<4;i++)
    {
      mac_and_ip[i] = macAddr->addr[i];//4字节本机设备地址
    }
    for(i=0;i<2;i++)
    {
      mac_and_ip[i+4] = ipAddr.edAddr[i];//2字节本机IP地址
    }
    
    send_reply_msg(in_frame, mac_and_ip, 6);
    
    rc= APP_NET_STATUS_SUCCESS;
  }
  
  return rc;  
}

/**************************************************************************************************
 * @fn          modify_ip_addr
 *
 * @brief       修改节点IP地址.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_MODIFY_IP_ADDR_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS      - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t modify_ip_addr(const app_frame_t *in_frame)
{
  app_net_status_t  rc = APP_MODIFY_IP_ADDR_FAIL;
  uint8_t           msg_id_0 = in_frame->data[3];    

  if(msg_id_0 == 0x01)
  {
    ip_addr_t ipAddr  = {0};   //获得4字节IP地址
    
    SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
    
    if(!memcmp(&in_frame->data[4], nwk_getMyAddress(), NET_ADDR_SIZE))//检测数据域：前4字节是否为我的地址
    {
      //数据域后2字节为新的IP地址,IP地址中集中器IP地址不能改变
      ipAddr.edAddr[0] = in_frame->data[8];
      ipAddr.edAddr[1] = in_frame->data[9];
      
//      WriteIPAddressToFlash(ipAddr);//更新FLASH中的IP地址
      SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_SET,(void *)&ipAddr);
      
      send_reply_msg(in_frame, NULL, 0);
      
      rc= APP_NET_STATUS_SUCCESS;
    }
  }
  
  return rc;  
}

/**************************************************************************************************
 * @fn          delete_ed_connect
 *
 * @brief       删除目标节点.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_DELETE_ED_CONNECT_FAIL     - 处理组网信息失败
 *              APP_NET_STATUS_SUCCESS      - 处理组网信息成功
 **************************************************************************************************
 */
app_net_status_t delete_ed_connect(const app_frame_t *in_frame)
{
  app_net_status_t  rc = APP_DELETE_ED_CONNECT_FAIL;
  uint8_t           msg_id_0 = in_frame->data[3];
  uint8_t           data_len = in_frame->data_len;

  if(msg_id_0 == 0x01)
  {   
    if((data_len==8)&&(!memcmp(&in_frame->data[4], nwk_getMyAddress(), NET_ADDR_SIZE)))//检测数据域：是否为我的地址
    {
      delete_ed_connect_flag = TRUE;
      send_reply_msg(in_frame, NULL, 0);
      
      rc= APP_NET_STATUS_SUCCESS;
    }
  }
  
  return rc;  
}

