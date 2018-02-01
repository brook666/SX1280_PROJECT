/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   uart_frm_proc.c
// Description: 处理来自串口的完整网外通信帧(0x69帧).
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-24
// History:     2014-10-24  Leidi 删除uart_frm_proc()中有关保留输入的整条0x69帧信息的代码.
//                                删除外部全局变量resvr_pkg[MAX_RESVR_PKG]和resvr_pkg_num.
//                                除全局变量pkg_clr_flag.
//              2014-10-31  Leidi 重写uart_frm_proc()中判断通信方向部分的代码,添加通信方向0x0e(组播).
//              2014-11-8   Leidi 新增函数gtway_info_check(),用来检查网关类型与地址.
//                                修改函数uart_frm_proc()的实现.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <string.h>
#include "uart_frm_proc.h"
#include "firmware_info_proc.h"
#include "gtway_frm_proc.h"
#include "69_frm_proc.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern addr_t sGtwayServerIP  ;
/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
uint8_t gtway_info_check(const gtway_pkg_t *in_pkg);
 
 
/**************************************************************************************************
 * @fn          uart_frm_proc
 *
 * @brief       处理来自串口的完整网外通信帧(0x69帧).
 *
 * @param       str - 串口输入的帧
 *              len - 串口输入的帧长
 *
 * @return      none
 **************************************************************************************************
 */
void uart_frm_proc(uint8_t *str,uint16_t len)
{
  gtway_pkg_t in_pkg;
  uint8_t trans_direction;
  uint8_t ack_msg[30];
  uint8_t ack_msg_len;
  uint8_t frame_type;
  
  /* 将网外通信帧数组装入网外通信帧结构体 */
  bui_gtway_pkg(str,&in_pkg);
  
  /* 检查网外通信帧中的网关信息是否正确 */
  if(gtway_info_check(&in_pkg) == 0)
  {
    return;
  }
  
  /* 检查网外通信帧控制码中的传输方向是否正确 */
  //服务器(S) 手机(M) 网关(G) 电脑(C)
  trans_direction = (in_pkg.ctr_code>>8)&0x0f;
  switch(trans_direction)
  {
    case 0x00: return;  //M->S(X)
    case 0x01: return;  //S->M(X)
    case 0x02: return;  //G->S(X)
    case 0x03: reserve_server_ip(&in_pkg);
		           break;   //S->G
    case 0x04: break;   //M->G
    case 0x05: return;  //G->M(X)
    case 0x06: break;   //M->S->G
    case 0x07: return;  //G->S->M(X)
//    case 0x08: put_multi_hex_uart1(str,len);//处于内网时由网关转发
//               return;  //M->C
    case 0x09: //TCP_Send_Filter(str,len);//处于内网时由网关转发
               return;  //C->M
    case 0x0a: break;   //C->G
    case 0x0b: return;  //G->C(X)
//    case 0x0c: put_multi_hex_uart1(str,len);//处于内网时由网关转发
//               return;  //M->S->C
    case 0x0d: //TCP_Send_Filter(str,len);//处于内网时由网关转发
               return;  //C->S->M
    case 0x0e: break;   //Multicast
    case 0x0f: break;   //Broadcast
		case 0x10: return;
		case 0x11: return;
		case 0x12: return;
		case 0x13: return;
    default: return;
  }

  /* 检查网外通信帧是否启用加密 */
  if(in_pkg.ctr_code & 0x8000)
  {
    /* 解密网外通信帧(未实现) */
  }

  /* 检查网外通信帧是否需要应答 */
  if(in_pkg.ctr_code & 0x4000)
  {
    /* 在处理该帧前先进行应答处理 */
    ack_msg_len = bui_gtway_ack(&in_pkg, ack_msg);
    Send_gtway_msg_to_uart(ack_msg, ack_msg_len);
  }

  /* 检查网外通信帧控制码中的帧类型 */
  frame_type = (in_pkg.ctr_code>>8) & 0x20;
  if(frame_type == 0x00)    //数据帧
  {
    proc_69_data_frm(&in_pkg);
  }
  else if(frame_type == 0x20)//应答帧
  {
    /* 处理应答帧(未实现) */
  }
  
  return;
}

/**************************************************************************************************
 * @fn          gtway_info_check
 *
 * @brief       检查来自串口的完整网外通信帧(0x69帧)的【网关类型】和【网关地址】是否正确.
 *
 * @param       pIn_pkg - 输入的网外通信帧结构体
 *
 * @return      1 - 网关信息正确
 *              0 - 网关信息错误
 **************************************************************************************************
 */
uint8_t gtway_info_check(const gtway_pkg_t *in_pkg)
{
  addr_t universal_gtway_addr = {0x00, 0x00, 0x00, 0x00};
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  addr_t my_gtway_addr;
  uint16_t my_gtway_type;
  uint8_t i;
  
  
  /* 允许使用通用网关地址 */
  if(!memcmp(&universal_gtway_addr, in_pkg->gtway_addr, MAC_ADDR_SIZE))
  {
    return 1;
  }
  
  /* 调整网关地址的字节序 */
  //网外通信帧使用大端(MSB)字节序
  //网关内部使用小端(LSB)字节序
  for(i = 0; i < MAC_ADDR_SIZE; i++)
  {
    my_gtway_addr.addr[i] = pMyGtwayAddr->addr[3-i]; //pMyGtwayAddr.addr={0x01,0x00,0x00,0x00}
  }
  
  /* 检查网关地址是否为本网关地址 */
  if(memcmp(&my_gtway_addr, in_pkg->gtway_addr, MAC_ADDR_SIZE))
  {
    return 0;
  }
  
  my_gtway_type = pMyGtwayAddr->type[0] | (pMyGtwayAddr->type[1] << 8); //pMyGtwayAddr.type={0xfc,0x00}
  
  /* 检查网关类型是否为本网关类型 */
  if(in_pkg->gtway_type != my_gtway_type)
  {
    return 0;
  }
  
  return 1;
}


/**************************************************************************************************
 * @fn          reserve_server_ip
 *
 * @brief       保存服务器IP.
 *
 * @param       pIn_pkg   - 输入的网外通信帧结构体
 *
 * @return      void
 **************************************************************************************************
 */
void reserve_server_ip(const gtway_pkg_t *pIn_pkg)
{
  
  memcpy(sGtwayServerIP.addr, pIn_pkg->server_addr, 4);
}
/**************************************************************************************************
 */


