/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   nwk_joinList.h
// Description: NWK APP.
//              本文件用于支持swsn的网络应用层Join过程中的许可列表.
// Author:      Leidi
// Version:     1.0
// Date:        2015-05-13
// History:     2015-05-13  Leidi 初始版本建立.
*****************************************************************************/
#ifndef __NWK_JOIN_LIST_H
#define __NWK_JOIN_LIST_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "nwk_types.h"


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
 /* 许可Join的设备列表的条目数量 */
#define JOIN_LIST_ITEMS_NUM NUM_CONNECTIONS

///* Join设备列表在FLASH中存放的起始地址 */
//#define JOIN_LIST_START_ADDR 0x08020000UL

/* ------------------------------------------------------------------------------------------------
 *                                            Typdefs
 * ------------------------------------------------------------------------------------------------
 */
/* Join设备状态枚举 */
typedef enum {
  JOIN_ADDR_NULL,    //未使用的列表项
  JOIN_ADDR_IN_USE,  //正在使用的列表项
} join_state_t;

/* Join设备信息项 */
typedef struct
{
  mac_addr_t   mac;
  ip_addr_t    ip;
  join_state_t state;
} join_list_t;


/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void nwk_joinList_init(void);
void nwk_joinList_deInit(void);
int8_t nwk_joinList_add(const mac_addr_t *);
int8_t nwk_joinList_del(const addr_t *);
void nwk_joinList_save(void);
uint8_t nwk_joinList_isFound(const addr_t *);
const join_list_t *nwk_joinList_get(void);

uint8_t nwk_joinList_assignIP(const addr_t *,ip_addr_t *);
uint8_t nwk_joinList_IPConflict(const ip_addr_t*);
uint8_t nwk_joinList_searchIP(const addr_t *,ip_addr_t *);
uint16_t nwk_getAPandTotalEDInfor(uint8_t *pOutMsg);


#endif

/**************************************************************************************************
 */


