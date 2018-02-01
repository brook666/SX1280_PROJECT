/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ed_buffer_fun.h
// Description: User APP.
//              �ڵ��շ�����������ͷ�ļ�.
// Author:      Leidi
// Version:     1.1
// Date:        2014-10-21
// History:     2014-10-21  Leidi ��ʼ�汾����.
//              2014-04-24  Leidi ��ed_tx_buffer_t�����Ӷ�дID�����γɻ��ζ���.
*****************************************************************************/
#ifndef _ED_TX_BUFFER_FUN_H
#define _ED_TX_BUFFER_FUN_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"
#include "ed_tx_rx_buffer.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
 /* ÿ���ڵ�ķ��ͻ��������� */
#define TX_BUFFER_NUM_FOR_ONE_ED  4

 /* ���нڵ�ͬʱʹ�õĻ��������� */
#define TX_BUFFER_NUM_FOR_ALL_ED  12

/* ------------------------------------------------------------------------------------------------
 *                                            Typedefs
 * ------------------------------------------------------------------------------------------------
 */
 /* �ڵ㻺����״̬ö�� */
typedef enum {
  BUFFER_AVALIABLE,
  BUFFER_IN_USE,
}buffer_state_t;

// /* �����ڵ㻺�������״̬ö�� */
//typedef enum {
//  BUFFER_SUCCESS,
//  BUFFER_IP_MODIFY_FAIL,
//  BUFFER_FREE_FAIL,
//}buffer_result_t;

 /* ����֡����������������ͨ��֡ͷ��Ҫ��Ϣ */
typedef struct
{
  buffer_state_t  in_use;
  uint16_t  ctr_code;
  uint8_t   mobile_addr[6];
  uint16_t  pid_num;
  uint8_t   inner_msg_len;
  uint8_t   inner_msg[MAX_APP_PAYLOAD];
} gtway_frm_info_t;

/* �����ڵ�ķ��ͻ����� */
typedef struct
{
  buffer_state_t  in_use;
  uint16_t  ed_ip;
  uint8_t   ed_tx_info_write_id;
  uint8_t   ed_tx_info_read_id;
  gtway_frm_info_t ed_tx_info[TX_BUFFER_NUM_FOR_ONE_ED];
} ed_tx_buffer_t;

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void Write_tx_ed_buffer(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t* pOut_len);
void Iterates_tx_ed_buffer(void);
buffer_result_t Modify_ip_in_tx_ed_buffer(uint16_t current_ed_ip, uint16_t modified_ed_ip);
buffer_result_t Free_tx_ed_buffer(uint16_t ed_ip);
uint8_t isReadyToWriteGroupFrm(void);
void Write_usart1_data_txbuffer(uint8_t *, uint8_t *, uint8_t* );
#endif

/**************************************************************************************************
 */

