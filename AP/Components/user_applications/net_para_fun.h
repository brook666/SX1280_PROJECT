/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_para_fun.h
// Description: User APP.
//              SWSN������Ƶ�������������ȡ.
// Author:      Leidi
// Version:     1.0
// Date:        2012-3-24
// History:     2014-9-26   Leidi ɾ�����ܴ����0x10~0x18�Ĳ�������/��ȡ����ԭ��.
//                                �޸ĸ���������,ʹ������app_frame_t.
//              2014-9-29   Leidi ����app_net_para_t�Ķ�����bui_pkg_fun.h���뱾�ļ�.
//              2014-10-5   Leidi ����ö������app_net_para_t�ĳ�Ա.
//              2015-07-28  Leidi ɾ������ԭ��read_freqest().
//                                ����ͷ�ļ�bui_pkg_fun.h.
*****************************************************************************/

#ifndef _NET_PARA_FUN_H
#define _NET_PARA_FUN_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bui_pkg_fun.h"

/* ------------------------------------------------------------------------------------------------
 *                                             Types
 * ------------------------------------------------------------------------------------------------
 */
typedef enum app_net_para{
  APP_NET_PARA_SUCCESS,
  APP_FREQ_READ_FAIL,
  APP_FREQ_WRITE_FAIL,
  APP_CHAN_READ_FALL,
  APP_CHAN_WRITE_FALL,
  APP_CHANSPC_READ_FALL,
  APP_CHANSPC_WRITE_FALL,
  APP_RATE_READ_FALL,
  APP_RATE_WRITE_FALL,
  APP_RXBW_READ_FALL,
  APP_RXBW_WRITE_FALL,
  APP_MANCHTER_READ_FALL,
  APP_MANCHTER_WRITE_FALL,
  APP_DEVIATN_READ_FALL,
  APP_DEVIATN_WRITE_FALL,
  APP_MODFORMAT_READ_FALL,
  APP_MODFORMAT_WRITE_FALL,
  APP_WHITING_READ_FALL,
  APP_WHITING_WRITE_FALL,
  APP_TXPW_READ_FALL,
  APP_TXPW_WRITE_FALL,
  APP_PARTNUM_READ_FALL,
  APP_VERSION_READ_FALL,
  APP_FREQEST_READ_FALL,
  APP_LQI_READ_FALL,
  APP_RSSI_READ_FALL,
  APP_MARCSTATE_READ_FALL,
  APP_WORH_READ_FALL,
  APP_WORL_READ_FALL,
  APP_PKTSTATUS_READ_FALL,
  APP_VCO_READ_FALL,
  APP_TXBYTES_READ_FALL,
  APP_RXBYTES_READ_FALL,
  APP_RCCTRL1_READ_FALL,
  APP_RCCTRL0_READ_FALL,
  APP_FEC_PROC_FALL,
  APP_SET_ADDRESS_FALL,
  NO_PAPA_FUN_FAIL,
} app_net_para_t;


/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
app_status_t   net_para_proc(const app_frame_t *in_frame);

app_net_para_t freq_proc(const app_frame_t *in_frame);     //Ƶ�ʵĶ�д
app_net_para_t chan_proc(const app_frame_t *in_frame);     //�ŵ��Ķ�д
app_net_para_t chanspc_proc(const app_frame_t *in_frame);  //�ŵ�����Ķ�д
app_net_para_t rate_proc(const app_frame_t *in_frame);   //���ʵĶ�д,
app_net_para_t rxbw_proc(const app_frame_t *in_frame);    //�����˲�����Ķ�д
app_net_para_t manchter_proc(const app_frame_t *in_frame);  //����˹�ر���ʹ�ܵĶ�д
app_net_para_t deviatn_proc(const app_frame_t *in_frame);  //Ƶ��ƫ��Ķ�д
app_net_para_t modformat_proc(const app_frame_t *in_frame);//���Ʒ�ʽ�Ķ�д
app_net_para_t whiting_proc(const app_frame_t *in_frame);  //�׻�ʹ��
app_net_para_t txpw_proc(const app_frame_t *in_frame);     //���书�ʵĶ�д
app_net_para_t read_partnum(const app_frame_t *in_frame);   //��ȡоƬ������
app_net_para_t read_version(const app_frame_t *in_frame);   //��ȡоƬ�汾��
app_net_para_t read_lqi(const app_frame_t *in_frame);   //��ȡLQI
app_net_para_t read_rssi(const app_frame_t *in_frame);   //��ȡRSSI
app_net_para_t no_para_fun(const app_frame_t *in_frame);   
app_net_para_t fec_proc(const app_frame_t *in_frame); //FEC״̬��ȡ������

app_net_para_t set_my_device_address(const app_frame_t *in_frame);//���ñ�����ַ

#endif

/**************************************************************************************************
 */
