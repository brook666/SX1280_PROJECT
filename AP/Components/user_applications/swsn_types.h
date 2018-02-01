/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   swsn_types.h
// Description: Ӧ�ò����ݵĻ�������.
// Author:      Zengjia
// Version:     1.0
// Date:        2015-12-24
// History:     2015-12-24  Zengjia ��ʼ�汾����.
*****************************************************************************/

#ifndef _SWSN_TYPES_H
#define _SWSN_TYPES_H

//����stm8l����������ͷ�ļ�
#include "stm8l15x_conf.h"

/* ------------------------------------------------------------------------------------------------
 *                                             Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAX_APP_MSG 40        //Ӧ�ò�֡����������󳤶�

#define APP_FRAME_HEAD 0x68   //Ӧ�ò�֡��֡ͷ
#define APP_FRAME_TAIL 0x16   //Ӧ�ò�֡��֡β


/* ------------------------------------------------------------------------------------------------
 *                                             Types
 * ------------------------------------------------------------------------------------------------
 */
/* ֡���Ͷ��� */
typedef enum {
  DATA_FRAME  = 0x00, //����֡
  ORDER_FRAME = 0x01, //����֡
  REPLY_FRAME = 0x02, //Ӧ��֡
  ALARM_FRAME = 0x03, //�澯֡
} frame_type_t;

/* ͨ�ŷ�ʽ���� */
typedef enum {
  TRANSPARENT_COMM    = 0x00, //Զ�̷���,����͸��
  NO_TRANSPARENT_COMM = 0x01, //Զ�̷���,���ݲ�͸��
  LOCAL_COMM          = 0x02, //���ط���
  RESERVE_COMM        = 0x03, //δ���巽ʽ
} comm_type_t;

/* ����״̬���� */
typedef enum {
  ONCE_TRANS  = 0x00, //���δ���
  FIRST_TRANS = 0x01, //��ʼ֡
  LAST_TRANS  = 0x02, //����֡
  MIDDLE_TRANS= 0x03, //�м�֡
} trans_type_t;

/* Ӧ�������� */
typedef enum {
  NO_NEED_REPLY = 0x00, //����ҪӦ��
  NEED_REPLY    = 0x01, //��ҪӦ��
} reply_type_t;

/* ͨ�ŷ����� */
typedef enum {
  AP_TO_ED = 0x00, //������->�ڵ�
  ED_TO_AP = 0x01, //�ڵ�->������
} comm_dir_t;

/* �����붨�� */
typedef union {
  struct
  {
    uint8_t frame_type  : 2;  //֡����
    uint8_t comm_type   : 2;  //ͨ�ŷ�ʽ
    uint8_t trans_type  : 2;  //����״̬
    uint8_t reply_type  : 1;  //Ӧ������
    uint8_t comm_dir    : 1;  //ͨ�ŷ���
  };
  uint8_t code;
} ctr_code_t;

/* Ӧ�ò�֡�ṹ���� */
typedef struct
{
  uint8_t frm_head;           //֡ͷ
  ctr_code_t ctr_code;        //������
  uint8_t ed_addr_1;          //�ڵ��ַ���ֽ�
  uint8_t ed_addr_0;          //�ڵ��ַ���ֽ�
  uint8_t ap_addr_1;          //��������ַ���ֽ�
  uint8_t ap_addr_0;          //��������ַ���ֽ�
  uint8_t frm_cnt;            //֡������
  uint8_t data_len;           //�����򳤶�
  uint8_t data[MAX_APP_MSG];  //������
  uint8_t cs;                 //У����
  uint8_t frm_tail;           //֡β
} app_frame_t;

/* ����ʱ��ṹ�� */
typedef struct
{  
  uint8_t   year;     //0~99
  uint8_t   month;    //1~12
  uint8_t   day;      //1~31
  uint8_t   week;     //1~7
  uint8_t   hours;    //0~23
  uint8_t   minutes;  //0~59  
  uint8_t   seconds;  //0~59
}daytime_t;

/* Ӧ�ò����ִ�н������ */
typedef enum app_status{
  APP_ERR,            //ִ��ʧ��
  APP_SUCCESS,        //ִ�гɹ�
} app_status_t;

typedef app_status_t (*pAppFunc)(const app_frame_t *in_frame);
typedef struct
{
  uint8_t commandId_DI2;
  pAppFunc pFunx;
}app_interface_t;

#define MAX_APP_FUNC 10
extern app_interface_t appFuncDomain[MAX_APP_FUNC];






#endif


