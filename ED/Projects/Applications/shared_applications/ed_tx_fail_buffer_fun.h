/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ed_rx_buffer_fun.h
// Description: User APP.
//              �ڵ��շ�����������ͷ�ļ�.
// Author:      Leidi
// Version:     1.1
// Date:        2015-09-28
// History:     2015-09-28  Zengjia ��ʼ�汾����.
*****************************************************************************/
#ifndef _ED_TX_FAIL_BUFFER_FUN_H
#define _ED_TX_FAIL_BUFFER_FUN_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <string.h>
#include "bsp.h"
#include "68_frm_proc.h"
#include "bui_pkg_fun.h"
#include "net_frm_proc.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Typedefs
 * ------------------------------------------------------------------------------------------------
 */
 /* �ڵ㷢��ʧ�ܻ�����״̬ö�� */
typedef enum {
  TX_FAIL_BUFFER_AVALIABLE,
  TX_FAIL_BUFFER_IN_USE,
}tx_fail_buffer_state_t;

 /* �����ڵ㻺�������״̬ö�� */
typedef enum {
  TX_FAIL_BUFFER_SUCCESS,
  TX_FAIL_BUFFER_FREE_FAIL,
}tx_fail_buffer_result_t;


/* �����ڵ�Ľ��ջ����� */
typedef struct
{
  volatile tx_fail_buffer_state_t  usage;
           uint8_t                 lid;
           uint8_t                 sendTimes;
           uint8_t                 ed_tx_fail_info[MAX_APP_PAYLOAD];
           uint8_t                 ed_tx_fail_info_len;
} ed_tx_fail_buffer_t;

/* ------------------------------------------------------------------------------------------------
 *                                            Externs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void Write_tx_fail_ed_buffer(uint8_t lid,uint8_t *pIn_msg, uint8_t* pIn_len);
void Find_ed_info_in_tx_fail_buffer(void);
void Iterates_tx_fail_ed_buffer(void);



extern volatile uint8_t app_frm_num ;
extern volatile uint8_t tx_fail_frm_num ;
extern volatile uint8_t start_emergency_dispatch_in_rtc ;
extern volatile uint8_t emergency_dispatch_flag ; 

#endif

/**************************************************************************************************
 */



