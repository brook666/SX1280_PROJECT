/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_frm_proc.h
// Description: User APP.
//              SWSN����֡��������ͷ�ļ�.
// Author:      Leidi
// Version:     1.0
// Date:        2012-3-24
// History:     2014-9-26   Leidi ɾ������ȫ�ֺ���ԭ��uint8_t net_ed_link(void).
//                                net_frm_proc()�ĺ�������ֵ��Ϊvoid.
//              2014-9-29   Leidi ����proc_bcast_frm()����Ϊbcast_frm_proc().
//              2015-07-29  Leidi ����ͷ�ļ�nwk_types.h.
*****************************************************************************/

#ifndef NET_FRM_PROC_H
#define NET_FRM_PROC_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "nwk_types.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
 
uint8_t sCB(linkID_t );
void net_frm_proc(void);
void bcast_frm_proc(void);
smplStatus_t send_usartdata_to_ap(uint8_t * ,uint8_t);
uint8_t send_usart_data_pkg(linkID_t lid, uint8_t *string, uint8_t tx_len);
#endif

