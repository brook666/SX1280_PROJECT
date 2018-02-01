/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ap_inner_fun.c
// Description: ����ڵ㷢�������ڲ������֡.
// Author:      Leidi
// Version:     1.0
// Date:        2014-3-24
// History:     2014-10-24  Leidi �ں���ap_inner_proc()��,�ɽڵ��Ϸ���֡�Ĵ������
//                                ���ڱ������н���,����������ߴ�����.
*****************************************************************************/ 
#include "bsp.h"
#include "bui_pkg_fun.h"
#include "68_frm_proc.h"
#include "ap_inner_fun.h"
#include "net_status_fun.h"
#include "net_para_fun.h"
#include "user_def_fun.h"

/***************************************************************************
 * @fn          ap_inner_proc
 *     
 * @brief       ��������������֡������֡���ݿ�����Ĳ�ͬ��comm_type=0x00����0x01
 *              ��͸�������ڣ�comm_type == 0x04,���Ǽ������ڲ���Ҫ�����֡��
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���      
 *     
 * @return      APP_SUCCESS - �ɹ�
 ***************************************************************************
 */ 
app_status_t ap_inner_proc(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  uint8_t  msg_id_1= in_pkg->msg[1]; 

	
  if(msg_id_1 == 0x01)
  {
    /* ����ED�Ϸ�������״̬����֡ */
    net_para_proc(in_pkg, out_pkg, len);     
  }	
  if(msg_id_1 == 0x02)
  {
    /* ����ED�Ϸ�������״̬����֡ */
    net_status_proc(in_pkg, out_pkg, len);     
  }
  else if(msg_id_1 == 0x80)
  {
    user_def_proc(in_pkg, out_pkg, len); 
  }  
  return APP_SUCCESS;
}


/*****************************************************************************/






