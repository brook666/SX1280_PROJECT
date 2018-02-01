/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   69_frm_proc.c
// Description: User APP.
//              网外通信帧(0x69帧)处理源文件.
// Author:      Leidi
// Version:     1.1
// Date:        2014-10-21
// History:     2014-10-21  Leidi 使用新的函数proc_68_frm()处理68帧以传递69帧中的有用信息.
//              2014-10-24  Leidi 删除未使用的外部全局变量resvr_pkg[MAX_RESVR_PKG].
//              2014-10-31  Leidi 修改函数proc_69_data_frm(),在处理内部帧前,清空输出数据存储区域.
//                                调用函数Send_gtway_msg_to_uart(),处理应从不同串口输出的帧.
//              2014-11-1   Leidi 使用新函数check_inner_frame_len()代替函数check_69_data()检测帧头与帧长.
//                                重写函数proc_69_data_frm(),增加对组播帧(0x67帧)的处理.
//                                删除未使用的全局变量set_wlan_flag.
//                                删除未使用函数trans_direc_comp(),proc_ctr_code().
//              2014-11-8   Leidi 删除未使用的函数gtway_mac_comp()和外部全局变量sMyGtwayAddr.
//                                删除未使用的函数proc_encrypt_frm(),proc_ack_req().
//                                删除未使用的头文件,将部分头文件从69_frm_proc.h移到本文件中.
//              2015-01-28  Leidi 新增函数get_inner_frame().
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "69_frm_proc.h"
#include "67_frm_proc.h"
#include "68_frm_proc.h"
#include "77_frm_proc.h"
#include "88_frm_proc.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static uint8_t check_inner_frame_len(const gtway_pkg_t *pIn_pkg);
static uint16_t get_inner_frame(const gtway_pkg_t *pIn_pkg, uint8_t *pOut_msg);
extern uint32_t band_rate;

/**************************************************************************************************
 * @fn          proc_69_data_frm
 *
 * @brief       处理来自串口的完整网外通信帧(0x69帧),并输出处理结果.
 *
 * @param       pIn_pkg - 输入的网外通信帧结构体
 *
 * @return      none
 **************************************************************************************************
 */
void proc_69_data_frm(gtway_pkg_t *in_pkg)
{
  uint8_t data_frm[300];//存储内部帧
  uint8_t gtway_msg[1024];//告警输出
  uint8_t out_pkg[1024];
  uint8_t data_frm_len;
  uint16_t data_77_frm_len;
  uint16_t gtway_msg_len;
  uint8_t alarm_info[10];
  uint8_t inner_frm_head;
      
  /* 内部帧的帧头与帧长检查 */
  if(!check_inner_frame_len(in_pkg))
  {
    /* 告警信息:串口传输正常，但不是有效帧 */
    alarm_info[0] = 0x00;//数据接收正常，但数据域错误
    alarm_info[1] = 0x04;  
    alarm_info[2] = 0x04;
    alarm_info[3] = 0x02;
    
    /* 构建告警帧并输出 */
    gtway_msg_len = bui_gtway_alarm(NULL, alarm_info, 4, gtway_msg); //告警内容直接出现在69帧数据域 
    Send_gtway_msg_to_uart(gtway_msg, gtway_msg_len);
    
    return;
  }
  /*处理68、88和77帧的应答*/
  pro_gtway_data_ack(in_pkg);
  
  /* 从网外帧结构体中获取内部帧 */
  data_77_frm_len = get_inner_frame(in_pkg, data_frm);
  data_frm_len = data_77_frm_len; //77帧长度占用2字节,其他帧长度占用1字节
  
  /* 清空输出数据存储区域 */
  memset(out_pkg, 0x0, sizeof(out_pkg));
  
  /* 根据内部帧的帧头进行分类处理 */
  inner_frm_head = data_frm[0];
  
  switch(inner_frm_head)
  {
    case 0x67: proc_67_frm(in_pkg, out_pkg, &data_frm_len);   //手机(用户端)与节点通信,组播
               break;
    case 0x68: proc_68_frm(in_pkg, out_pkg, &data_frm_len);   //手机(用户端)与节点通信,单播或广播
               break;
    case 0x88: proc_88_frm(data_frm, out_pkg, &data_frm_len); //手机(用户端)与服务器通信
               break;
    case 0x77: proc_77_frm(data_frm, out_pkg, &data_77_frm_len); //服务器与网关通信
               break;
    default: return;
  }

  /* 将内部帧的处理结果包装成外部帧(0x69帧) */
  inner_frm_head = out_pkg[0];
  switch(inner_frm_head)
  {
    case 0x67:  /* Go down */
    case 0x68:  /* Go down */
    case 0x88: gtway_msg_len = bui_outnet_frm(in_pkg, out_pkg, data_frm_len, gtway_msg);
               break;
    case 0x77: gtway_msg_len = bui_outnet_frm(in_pkg, out_pkg, data_77_frm_len, gtway_msg);
               break;
    default: return;
  }
  
  /* 向串口发送网外通信帧 */
  Send_gtway_msg_to_uart(gtway_msg, gtway_msg_len);
	
	if((in_pkg->msg[10]==0x20&in_pkg->msg[9]==0x01))
	{
		delay_ms(5);
	  USART2_NVIC_Configuration(DISABLE);  
	  Usart2_Configuration(band_rate,ENABLE);
    USART2_NVIC_Configuration(ENABLE);
	}
	
}


/**************************************************************************************************
 * @fn          check_inner_frame_len
 *
 * @brief       检查来自串口的完整网外通信帧(0x69帧)的数据域长度是否正确.
 *
 * @param       pIn_pkg - 输入的网外通信帧结构体
 *
 * @return      1 - 数据域长度正确
 *              0 - 数据域长度错误
 **************************************************************************************************
 */
static uint8_t check_inner_frame_len(const gtway_pkg_t *pIn_pkg)
{
  uint8_t frame_head = pIn_pkg->msg[0];
  uint8_t inner_msg_len;
  uint8_t mult_ip_count;
  uint8_t mult_field_len;
  
  switch(frame_head)
  {
//    case 0x66: /* Go down */
    case 0x68: /* Go down */
    case 0x88: inner_msg_len = pIn_pkg->msg[7] + 10;  //数据域长度为1字节
               break;
    case 0x77: inner_msg_len = (((pIn_pkg->msg[7]&0xffff)<<8) | pIn_pkg->msg[8]) + 11;  //数据域长度为2字节
               break;
    case 0x67: //组播帧的内部帧由IP地址域和常规内部帧组成
    {
      mult_ip_count = pIn_pkg->msg[1];
      if(mult_ip_count > IP_GROUP_MAX_NUM)
      {
        return 0;
      }
      mult_field_len = 2 * mult_ip_count +2;
      inner_msg_len = mult_field_len + pIn_pkg->msg[mult_field_len + 7] + 10;
      break;
    }
    default: return 0;
  }
  
  /* 检测内部帧的长度是否等于外部帧数据域的长度 */
  if(pIn_pkg->data_len != inner_msg_len)
  {
    return 0;
  }
  
  return 1;
}

/**************************************************************************************************
 * @fn          get_inner_frame
 *
 * @brief       从网外帧结构体中获取内部帧.
 *
 * @param       pIn_pkg   - 输入的网外通信帧结构体
 *              pOut_msg  - 输出的内部帧数组
 *
 * @return      输出的内部帧数组长度
 **************************************************************************************************
 */
static uint16_t get_inner_frame(const gtway_pkg_t *pIn_pkg, uint8_t *pOut_msg)
{
  uint16_t out_msg_len = pIn_pkg->data_len;
  
  memcpy(pOut_msg, pIn_pkg->msg, out_msg_len);

  return out_msg_len;
}


/**************************************************************************************************
 */
