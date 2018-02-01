/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_list_proc.h
// Description: User APP.
//              节点信息链表处理.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-25
// History:     2014-10-25  Leidi 添加函数原型Modify_IP_by_MAC().
//              2014-10-28  Leidi 结构体ip_mac_id增加status字段,用以表示节点状态.
//                                删除函数原型pdel_ip(),modify_ip().
//              2014-11-24  Leidi 由于主程序扩大,修改将信道号存放地址CHANNR_PTR由0x0800c800改为0x08010800.
*****************************************************************************/

#ifndef _NET_LIST_PROC_H
#define _NET_LIST_PROC_H

#include "bsp.h"
#include "nwk_layer.h"

//#define CHANNR_PTR          0x0800d000   //zeng add 
//#define LEN_PTR             0x0800d400  
//#define LIST_IND_PTR        0x0800d800
//#define ED_NUM_PTR          0x0800d802
//#define LEN_PTR             0x0800d804
//#define HEAD_LIST_PTR       0x0800d808  //存储节点信息的内存：0xf100--0xf1ff
//#define SYSINFO_PTR         0x0800dc00  //

struct ip_mac_id
{
  uint8_t ip[4];  //节点IP地址
  uint8_t mac[4]; //节点MAC地址
  uint8_t id;     //节点LID
  uint8_t status; //节点的状态字节
  struct ip_mac_id *next;
};

uint8_t init_list(void);
uint8_t add_net_list(uint8_t );
uint8_t ip_comp(struct ip_mac_id *,uint16_t *);
uint8_t mac_comp(struct ip_mac_id *,uint8_t *);
uint8_t add_net_list(uint8_t );

uint8_t pdel_listNode(struct ip_mac_id *);
uint8_t pdel_mac(uint8_t *mac);
uint8_t channr_num_from_flash(void);
void channr_num_to_flash( uint8_t );

uint8_t psearch_ip(uint16_t *, struct ip_mac_id **);
uint8_t psearch_id(uint8_t , struct ip_mac_id **);
uint8_t psearch_mac(uint8_t *, struct ip_mac_id **);

void SEGGER_RTT_print_all_ed_info(struct ip_mac_id *head);
void print_all_ed_info(struct ip_mac_id *);
//void sysinfo_to_flash( void);
//void sysinfo_from_flash( void);

int head_list_to_flash( struct ip_mac_id *);
int flash_to_head_list( struct ip_mac_id *);
//end

uint8_t Modify_IP_by_MAC(const addr_t *, uint16_t );
uint8_t SWSN_deleteInnetED(addr_t mac_addr, linkID_t lid);
void SWSN_checkDuplicateLinkEdIPList(void);
#endif














