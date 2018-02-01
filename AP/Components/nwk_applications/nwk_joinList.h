/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   nwk_joinList.h
// Description: NWK APP.
//              ���ļ�����֧��swsn������Ӧ�ò�Join�����е�����б�.
// Author:      Leidi
// Version:     1.0
// Date:        2015-05-13
// History:     2015-05-13  Leidi ��ʼ�汾����.
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
 /* ���Join���豸�б����Ŀ���� */
#define JOIN_LIST_ITEMS_NUM NUM_CONNECTIONS

///* Join�豸�б���FLASH�д�ŵ���ʼ��ַ */
//#define JOIN_LIST_START_ADDR 0x08020000UL

/* ------------------------------------------------------------------------------------------------
 *                                            Typdefs
 * ------------------------------------------------------------------------------------------------
 */
/* Join�豸״̬ö�� */
typedef enum {
  JOIN_ADDR_NULL,    //δʹ�õ��б���
  JOIN_ADDR_IN_USE,  //����ʹ�õ��б���
} join_state_t;

/* Join�豸��Ϣ�� */
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


