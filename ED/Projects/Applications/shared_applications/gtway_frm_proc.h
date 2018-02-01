/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   gtway_frm_proc.h
// Description: ��������ͨ��֡.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-23
// History:     2014-10-23  Leidi �������ͨ��֡��֡ͷ��֡β��.
//                                ��ӻ�ȡ����ͨ��֡�Ĵ��䷽���.
//                                ��������ԭ��Send_gtway_msg_to_uart().
//                                ��������ԭ��Reverse_trans_direction().
//                                ��������ԭ��Bui_bcast_gtway_msg_from_inner_msg().
//              2014-10-24  Leidi ɾ������ԭ��select_resvr_pkg().
//                                ɾ������ԭ��rebuild_69_frm().
//                                ɾ������ԭ��rebuild_69_alarm().
//                                ɾ������ԭ��decrement_resvr_pkg_num().
//              2015-01-21  Leidi ɾ������ԭ��bui_gtway_bcast().
//              2015-01-28  Leidi ɾ������ԭ��get_innet_frm().
//                                ɾ�����Ͷ���net_pkg_t.
*****************************************************************************/

#ifndef _GTWAY_FRM_PROC_H
#define _GTWAY_FRM_PROC_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
 #include"bsp.h"


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAX_APP_MSG1 2048

/* ����ͨ��֡��0x69��֡ͷ��֡β */
#define GTWAY_FRAME_HEAD 0x69
#define GTWAY_FRAME_TAIL 0x17

/* ��ȡ����ͨ��֡�Ĵ��䷽�� */
#define GET_GTWAT_FRM_TRANS_DIRECTION(msg)  ((uint8_t)msg[2]&0x0f)


/* ------------------------------------------------------------------------------------------------
 *                                            Typedefs
 * ------------------------------------------------------------------------------------------------
 */
typedef struct
{
  uint8_t  frm_head;
  uint8_t  reserve1;
  uint16_t ctr_code;
  uint16_t gtway_type;
  uint8_t  gtway_addr[4];
  uint8_t  server_addr[4];
  uint8_t  mobile_addr[6];
  uint16_t pid_num;
  uint16_t data_len;     
  uint8_t  msg[MAX_APP_MSG1];   
  uint8_t  cs;
  uint8_t  frm_end;

} gtway_pkg_t;

//typedef struct
//{
//  gtway_pkg_t gtway_pkg;
//  uint8_t    cur_num;
//}net_pkg_t;

enum app_device_disg{
  APP_DEVICE_DISG_SUCCESS,
  APP_DEVICE_DISG_FAIL,
};

typedef enum app_device_disg app_device_disg_t;

enum app_server_para{
  APP_SERVER_PARA_SUCCESS,
  APP_HEART_BEAT_FAIL,
  APP_REQ_MAC_FAIL,
};

typedef enum app_server_para app_server_para_t;

enum app_client_para{
  APP_CLIENT_PARA_SUCCESS,
  APP_SET_WLAN_SSID_FAIL,
  APP_CHECK_AP_FAIL,
};

typedef enum app_client_para app_client_para_t;

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void bui_gtway_pkg(uint8_t *,gtway_pkg_t *);
//uint16_t get_innet_frm(gtway_pkg_t *,uint8_t *);
uint16_t bui_outnet_frm(gtway_pkg_t *,uint8_t *,uint16_t ,uint8_t *);
uint8_t bui_gtway_ack(gtway_pkg_t *,uint8_t * );
uint8_t bui_gtway_alarm(gtway_pkg_t *,uint8_t *,uint8_t ,uint8_t *);

//uint8_t bui_gtway_bcast(uint8_t *,uint8_t ,uint8_t *);


//void Send_gtway_msg_to_uart(uint8_t *pMsg, uint16_t msg_len);
uint16_t Reverse_trans_direction(uint16_t control_code);
uint8_t Bui_bcast_gtway_msg_from_inner_msg(const uint8_t *pInner_msg, uint8_t inner_msg_len, uint8_t *pOut_msg);
void pro_gtway_data_ack(gtway_pkg_t *pIn_pkg);
uint8_t Bui_exception_msg( uint8_t *pInner_msg, uint8_t inner_msg_len, uint8_t *pOut_msg);
uint8_t Bui_send_to_cloudserver_msg(uint8_t *pInner_msg, uint8_t inner_msg_len, uint8_t *pOut_msg);
//void test_exception_msg(void);
#endif
/**************************************************************************************************
 */

