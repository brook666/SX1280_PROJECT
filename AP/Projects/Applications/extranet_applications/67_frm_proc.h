/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   67_frm_proc.h
// Description: User APP.
//              �ڲ��鲥֡(0x67֡)����ͷ�ļ�.
// Author:      Leidi
// Version:     1.0
// Date:        2014-11-1
// History:     2014-11-1   Leidi ��ʼ�汾����.
*****************************************************************************/
#ifndef _67_FRM_PROC_H
#define _67_FRM_PROC_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "gtway_frm_proc.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define IP_GROUP_MAX_NUM 10 //ÿ���鲥֡���������IP��ַ����
#define IP_GROUP_FRAME_BUFFER  10   //����10��67֡
#define IP_GROUP_FRAME_LENGTH  40   //�鲥֡�ĳ���

/* ------------------------------------------------------------------------------------------------
 *                                            Typedefs
 * ------------------------------------------------------------------------------------------------
 */
  /* �ڵ㻺����״̬ö�� */
typedef enum {
  GROUP_BUFFER_AVALIABLE,
  GROUP_BUFFER_IN_USE,
}group_buffer_state_t;

typedef struct
{
  volatile group_buffer_state_t usage;
           uint8_t              orderStamp;
           uint8_t              msg[IP_GROUP_FRAME_LENGTH];
           uint16_t             len;
}group_buffer_t;


/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void proc_67_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len);
uint8_t Change_67frm_to_68frm(gtway_pkg_t *pIn_gtway_pkg, uint16_t *pOut_ip_group);
uint16_t Change_67frm_to_69frm(uint8_t * pIn_msg,gtway_pkg_t *pOut_gtway_pkg, uint16_t *len);
void write_group_buffer(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t* pOut_len);
void write_67frm_to_ed_tx_buffer(void);
#endif

 /**************************************************************************************************
 */
