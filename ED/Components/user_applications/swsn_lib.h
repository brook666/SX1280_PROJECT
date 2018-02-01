/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   app_layer.h
// Description: Ӧ�ò���غ���.
// Author:      Zengjia
// Version:     1.0
// Date:        2015-12-24
// History:     2015-12-24  Zengjia ��ʼ�汾����.
*****************************************************************************/

#ifndef _SWSN_LIB_H
#define _SWSN_LIB_H

//����stm8l����������ͷ�ļ�
#include "swsn_types.h"


/*
  �漶��Դ��ʼ��
*/
  void BSP_Init(void); 

/*
  flash����������һЩ��Ҫ����Ϣ��
  startAddress -- ��ʼ��ַ��programByte -- ָ���д������ݣ�programLen -- ��д�����ݵĳ���
*/
void FLASH_WriteProgram(uint32_t startAddress, const uint8_t *programByte, uint16_t programLen);

/*
  ���ڵĻ�ȡ�����ã�
  BSP_setDateTime -- ���õ�ǰ�����ں�ʱ��.daytime - (����)����ʱ��ṹ��
  BSP_getDateTime -- ��ȡ��ǰ�����ں�ʱ��.daytime - (���)����ʱ��ṹ��
*/
void BSP_setDateTime(daytime_t* daytime);
void BSP_getDateTime(daytime_t* daytime);
void BSP_RCT_Reinit_for_Awu(void);
void swsn_wait_relay_stable(void);
/*
�ϵ縴λһ����������ң����ֱ����Ժͽ����غ�����
abolish_remoter_direct_pairing --�ϵ�һ���Ӻ�ȡ��ң����ֱ����Ժͽ������
isAllow_remoter_direct_pairing --�ж��Ƿ�����ң����ֱ����Ժͽ��
*/
uint8_t abolish_remoter_direct_pairing(void);
uint8_t isAllow_remoter_direct_pairing(void);

//�Ǿ�ȷ��ʱ������������Ź���ι���������ں����ڲ�ִ�У�
//���ÿ��Ǹ���ʱ�����ĳ�ʱʱ�䡣
void SWSN_DELAY(uint16_t  ms);

/*
  Э��ջ��ʼ�������Ӽ�����
*/
void swsn_info_init(void);

/*
  ����swsnЭ��������¼������ɶ�ʧ
*/
void swsn_app_proc(void);

/*
  ע�Ṧ��ģ�麯����
 �����ʶ��Ϊ4���ֽ� DI3 DI2 DI1 DI0,����DI3��ʾ��˾���ţ�DI2��ʾ������
 DI1��ʾ���������治ͬ�Ĺ��ܣ�DI0��1��ʾд��0��ʾ����
 commd_DI2 -- �����ʶDI2
 appFunc   -- ��������
*/
uint8_t registerAppFuncDomain(uint8_t commd_DI2,pAppFunc appFunc);


/*������������֡�����ͻظ�֡��
  in_frame   -- ����ĵ�֡
  reply_para -- �ظ�������
  para_len   -- �ظ����ݵĳ���
*/
uint8_t send_reply_msg(const app_frame_t *in_frame, const uint8_t *reply_para, uint8_t para_len);

/*������������֡�����͸澯֡��
  in_frame   -- ����ĵ�֡
  err_info   -- �澯����
  para_len   -- �澯���ݵĳ���
*/
uint8_t send_alarm_msg(const app_frame_t *in_frame, const uint8_t *err_info, uint8_t err_len);

/*�ڵ�������������������֡��
  commandID   -- �����ʶ
  data        -- ��Ч����
  data_len    -- ��Ч���ݵĳ���
*/
uint8_t send_command_msg(uint8_t *commandID, uint8_t *data, uint8_t data_len);

#endif

