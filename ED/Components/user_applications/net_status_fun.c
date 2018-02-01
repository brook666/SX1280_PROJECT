/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_status_fun.c
// Description: User APP.
//              SWSN����״̬�������������ȡ.
// Author:      Leidi
// Version:     2.0
// Date:        2012-3-24
// History:     2014-9-28   Leidi �޸ĸ���������,ʹ������app_frame_t. 
//                                ɾ�������ͷ�ļ���ȫ�ֱ���.
//              2014-10-5   Leidi �澯֡����net_status_proc()����,���������Ӻ���.
//                                ���Ӻ�������no_status_fun().
//              2014-10-27  Leidi ����modify_ip_addr()�еķ���֡��Ϊ͸��(������������).
//              2014-10-29  Leidi ����delete_ed_connect()�еķ���֡��Ϊ͸��(������������).
//              2014-11-10  Leidi �Ƴ�ͷ�ļ�stm8s_uart.h.
//                                ����������õ�ͷ�ļ�user_app_config.h.
//              2015-05-30  Leidi ɾ��ȫ�ֱ���time_sync_cnt,real_time,sync_time.
//                                ������get_time_sync()����Ϊset_datetime().
//              2015-07-29  Leidi ɾ������ͷ�ļ�user_def_fun.h,nwk_types.h.
//                                ɾ������time_sync_space_proc().
//                                ɾ����ȫ�ֱ���sPersistInfo,myIpAddr������,���Ӷ�sLinkID1������.
//              2015-07-30  Leidi �����õ�ȫ�ֱ���time_sync_switch,sLinkID1�������뺯���ڲ�.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "net_status_fun.h"
#include "bsp_mac_addr.h"
#include "nwk_token.h"
#include "nwk.h"
#include "nwk_globals.h"
#include "nwk_api.h"
#include "stm32_rtc.h"
#include <string.h>
#include <stdlib.h>
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
/* �ڵ��Ƿ��Ѿ���APɾ���ı�־ */
uint8_t delete_ed_connect_flag = FALSE;

/**************************************************************************************************
 * @fn          net_status_proc
 *
 * @brief       ��������״̬�������ϲ㺯��,���崦����̽������Ӻ���.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_ERR     - ��������״̬����ʧ��
 *              APP_SUCCESS - ��������״̬�����ɹ�
 **************************************************************************************************
 */
app_status_t net_status_proc(const app_frame_t *in_frame)
{
  uint8_t alarm_info[4];
  app_net_status_t  status_rc = APP_NET_STATUS_SUCCESS;

  switch(in_frame->data[2])
  {
    case 0x03:  status_rc = get_join_link_token(in_frame);  //��ȡjoin Token��link Token
                break; 
    case 0x04:  status_rc = set_join_token(in_frame);     //����join Token
                break;
    case 0x05:  status_rc = set_link_token(in_frame);     //����link Token
                break;        
    case 0x20:  status_rc = get_ed_mac(in_frame);     //��ȡĬ�ϵ�MAC��ַ
                break;
    case 0x30:  status_rc = set_datetime(in_frame);   //��������ʱ��
                break;
    case 0x31:  status_rc = time_sync_proc(in_frame); //��������֡���
                break;
    case 0x80:  status_rc = get_ed_info(in_frame);    //��ѯ�ڵ��������Ϣ����
                break;
    case 0x90:  status_rc = get_ed_mac_and_ip(in_frame);//��ѯ�ڵ��MAC��IP
                break;
    case 0x91:  status_rc = modify_ip_addr(in_frame);   //�޸Ľڵ�IP��ַ
                break;
    case 0x92:  status_rc = delete_ed_connect(in_frame);//ɾ��Ŀ��ڵ�
                break;
    default:    status_rc = no_status_fun(in_frame);
  }

  if(status_rc == APP_NET_STATUS_SUCCESS)
  {
    return APP_SUCCESS;
  }
  else
  {
    alarm_info[0]=0x00;
    alarm_info[1]=0x01;
    alarm_info[2]=0x01;
    alarm_info[3]=0x08;
    send_alarm_msg(in_frame, alarm_info, 4);
    
    return APP_ERR;
  }
}

/**************************************************************************************************
 * @fn          no_status_fun
 *
 * @brief       δʹ�õ�����״̬���ú���.
 *
 * @param       in_frame  - �����֡�ṹ��
 *              out_msg   - ������ֽ�����
 *              out_len   - ������ֽ����鳤��
 *
 * @return      NO_STATUS_FUN_FAIL     - ��������״̬���ú���ʧ��
 **************************************************************************************************
 */
app_net_status_t no_status_fun(const app_frame_t *in_frame)
{
  app_net_status_t rc = NO_STATUS_FUN_FAIL;
   
  return rc;
}
/**************************************************************************************************
 * @fn          get_join_link_token
 *
 * @brief       ��ȡjoin Token��link Token.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_GET_JOIN_LINK_TOKEN_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS           - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t get_join_link_token(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_GET_JOIN_LINK_TOKEN_FAIL;
  uint8_t  msg_id_0 = in_frame->data[3];
  
  if(msg_id_0 == 0x00)
  {
    uint8_t  join_link_token[8];
    ioctlToken_t link ;
    ioctlToken_t join ;
    
    join.tokenType = TT_JOIN;
    link.tokenType = TT_LINK;
    
    SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_GET,(void *)&join);
    SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_GET,(void *)&link);
    
    join.token.joinToken = swsn_htonl(join.token.joinToken);
    link.token.linkToken = swsn_htonl(link.token.linkToken);
    
    memcpy(&join_link_token[0], &join.token.joinToken, sizeof(join.token.joinToken));//��������(��FE 08 01 05)
    memcpy(&join_link_token[4], &link.token.linkToken, sizeof(link.token.linkToken));//��������(��05 04 03 02)
    send_reply_msg(in_frame, join_link_token, 8);
    
    rc = APP_NET_STATUS_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_GET_JOIN_LINK_TOKEN_FAIL;	
  }
  
  return rc;
}
/**************************************************************************************************
 * @fn          set_join_token
 *
 * @brief       ����join Token.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_SET_JOIN_TOKEN_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS  - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t set_join_token(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_SET_JOIN_TOKEN_FAIL;
  uint8_t  msg_id_0 = in_frame->data[3];
  
  if(msg_id_0 == 0x00)
  {
    ioctlToken_t join ;
    
    join.tokenType = TT_JOIN;
    join.token.joinToken = in_frame->data[3] | \
                          ((in_frame->data[4]&0xffffffff)<<8) | \
                          ((uint32_t)(in_frame->data[5]&0xffffffff)<<16) | \
                          ((uint32_t)(in_frame->data[6]&0xffffffff)<<24);  
    
    SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_SET,(void *)&join);
    WriteJoinTokenToFlash(join.token.joinToken);
    
    send_reply_msg(in_frame, NULL, 0);
    
    rc = APP_NET_STATUS_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_SET_JOIN_TOKEN_FAIL;	
  }
  
  return rc;
}

/**************************************************************************************************
 * @fn          set_link_token
 *
 * @brief       ����link Token.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_GET_LINK_TOKEN_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS  - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t set_link_token(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_SET_LINK_TOKEN_FAIL;
  uint8_t  msg_id_0 = in_frame->data[3];
  
  if(msg_id_0 == 0x00)
  {
    ioctlToken_t join ;
    
    join.tokenType = TT_LINK;
    join.token.linkToken = in_frame->data[3] | \
                          ((in_frame->data[4]&0xffffffff)<<8) | \
                          ((uint32_t)(in_frame->data[5]&0xffffffff)<<16) | \
                          ((uint32_t)(in_frame->data[6]&0xffffffff)<<24);  
    
    SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_SET,(void *)&join);
    WriteLinkTokenToFlash(join.token.linkToken);
    
    send_reply_msg(in_frame, NULL, 0);
    
    rc = APP_NET_STATUS_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_SET_LINK_TOKEN_FAIL;	
  }
  
  return rc;
}
/**************************************************************************************************
 * @fn          get_ed_mac
 *
 * @brief       ��ȡĬ�ϵ�MAC��ַ.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_GET_AP_MAC_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS  - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t get_ed_mac(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_GET_AP_MAC_FAIL;
  uint8_t 		   msg_id_0 = in_frame->data[3];
  uint8_t 		   ed_mac[4]; 
  addr_t         myromaddr = THIS_DEVICE_ADDRESS;	  
  uint8_t i;
  if(msg_id_0 == 0x00)
  {
    for(i = 0; i < 4; i++)
    {
      ed_mac[i] = myromaddr.addr[3 - i];
    }
    
    send_reply_msg(in_frame, ed_mac, 4);
    
    rc = APP_NET_STATUS_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_GET_AP_MAC_FAIL;	
  }
  
  return rc;
}

/**************************************************************************************************
 * @fn          set_datetime
 *
 * @brief       ��������ʱ��.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_PROC_TIME_SYNC_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS      - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t set_datetime(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_PROC_TIME_SYNC_FAIL;
  uint8_t 	       msg_id_0 = in_frame->data[3];    
  daytime_t        daytime_s, daytime_g;
  
  if(msg_id_0 == 0x01)
  {
    daytime_s.year  = in_frame->data[4];
    daytime_s.month = in_frame->data[5];
    daytime_s.day   = in_frame->data[6];
    daytime_s.week  = in_frame->data[11];
    
    daytime_s.hour   = in_frame->data[7];
    daytime_s.minute = in_frame->data[8];
    daytime_s.second = in_frame->data[9];
    
    /* ��ȡ��ǰ����ʱ�� */
    Time_Get((ap_time_t *)&daytime_g);
    
    /* ֻ�е�ƫ�����ʱ�Ÿ�������ʱ�� */
    if(abs(daytime_s.second - daytime_g.second) > 5 ||
       daytime_s.minute != daytime_g.minute ||
       daytime_s.hour != daytime_g.hour ||
       daytime_s.week != daytime_g.week)
    {
      /* ��������ʱ�� */
      Time_Set((uint8_t *)&daytime_s,7);
    }
    
    rc= APP_NET_STATUS_SUCCESS;
  }
  
  return rc;  
}

/**************************************************************************************************
 * @fn          time_sync_proc
 *
 * @brief       ��������֡���.
 *
 * @param       in_frame  - �����֡�ṹ��
 *              out_msg   - ������ֽ�����
 *              out_len   - ������ֽ����鳤��
 *
 * @return      APP_PROC_TIME_SYNC_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS      - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t time_sync_proc(const app_frame_t *in_frame)
{
  app_net_status_t  rc = APP_PROC_TIME_SYNC_FAIL;
  uint8_t 	        msg_id_0 = in_frame->data[3];
  uint8_t           time_sync[1];
  extern uint8_t    time_sync_switch; //����֡��⿪��
  
  if(msg_id_0 == 0x00)
  {
    time_sync[0] = time_sync_switch;
    
    send_reply_msg(in_frame, time_sync, 1);
    
    rc= APP_NET_STATUS_SUCCESS; 
  }
  else if(msg_id_0 == 0x01)
  {
    time_sync[0] = in_frame->data[4];
    time_sync_switch = time_sync[0];
    
    send_reply_msg(in_frame, time_sync, 1);
    
    rc= APP_NET_STATUS_SUCCESS;
  }
  
  return rc;  
}

/**************************************************************************************************
 * @fn          get_ed_info
 *
 * @brief       ��ѯ�ڵ��������Ϣ����.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_GET_NET_INFO_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS      - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t get_ed_info(const app_frame_t *in_frame)
{
  app_net_status_t rc = APP_GET_NET_INFO_FAIL;
  uint8_t 		   msg_id_0 = in_frame->data[3];
  uint8_t 		   ed_info[9]; 
  extern linkID_t  sLinkID1;   //���ӱ�ʾ��,���ڻ�ȡ������Ϣ

  if(msg_id_0 == 0x00)
  {
    /* ��ȡ������Ϣ */
    connInfo_t *pCInfo = nwk_getConnInfo(sLinkID1);
    if(pCInfo != NULL)
    {
      ed_info[0] =  pCInfo->thisLinkID;
      ed_info[1] =  pCInfo->connState;
      ed_info[2] =  pCInfo->hops2target;
      ed_info[3] =  pCInfo->peerAddr[0];
      ed_info[4] =  pCInfo->peerAddr[1];
      ed_info[5] =  pCInfo->sigInfo.rssi;
      ed_info[6] =  pCInfo->sigInfo.lqi;
      
      send_reply_msg(in_frame, ed_info, 7);
    
      rc= APP_NET_STATUS_SUCCESS;
    }
  }
  else if(msg_id_0 == 0x01)
  {
    rc = APP_GET_NET_INFO_FAIL;
  }

  return rc;
}

/**************************************************************************************************
 * @fn          get_ed_mac_and_ip
 *
 * @brief       ��ѯ�ڵ��MAC��IP.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_GET_ED_MAC_AND_IP_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS      - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t get_ed_mac_and_ip(const app_frame_t *in_frame)
{
  app_net_status_t  rc = APP_GET_ED_MAC_AND_IP_FAIL;
  uint8_t           msg_id_0 = in_frame->data[3]; 
  uint8_t           mac_and_ip[6];
  uint8_t i;     
  if(msg_id_0 == 0x00)
  {
    addr_t *macAddr = (addr_t *)nwk_getMyAddress();//���4�ֽ�MAC��ַ
    ip_addr_t ipAddr  = {0};   //���4�ֽ�IP��ַ
    
    SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
    
    for(i=0;i<4;i++)
    {
      mac_and_ip[i] = macAddr->addr[i];//4�ֽڱ����豸��ַ
    }
    for(i=0;i<2;i++)
    {
      mac_and_ip[i+4] = ipAddr.edAddr[i];//2�ֽڱ���IP��ַ
    }
    
    send_reply_msg(in_frame, mac_and_ip, 6);
    
    rc= APP_NET_STATUS_SUCCESS;
  }
  
  return rc;  
}

/**************************************************************************************************
 * @fn          modify_ip_addr
 *
 * @brief       �޸Ľڵ�IP��ַ.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_MODIFY_IP_ADDR_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS      - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t modify_ip_addr(const app_frame_t *in_frame)
{
  app_net_status_t  rc = APP_MODIFY_IP_ADDR_FAIL;
  uint8_t           msg_id_0 = in_frame->data[3];    

  if(msg_id_0 == 0x01)
  {
    ip_addr_t ipAddr  = {0};   //���4�ֽ�IP��ַ
    
    SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
    
    if(!memcmp(&in_frame->data[4], nwk_getMyAddress(), NET_ADDR_SIZE))//���������ǰ4�ֽ��Ƿ�Ϊ�ҵĵ�ַ
    {
      //�������2�ֽ�Ϊ�µ�IP��ַ,IP��ַ�м�����IP��ַ���ܸı�
      ipAddr.edAddr[0] = in_frame->data[8];
      ipAddr.edAddr[1] = in_frame->data[9];
      
//      WriteIPAddressToFlash(ipAddr);//����FLASH�е�IP��ַ
      SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_SET,(void *)&ipAddr);
      
      send_reply_msg(in_frame, NULL, 0);
      
      rc= APP_NET_STATUS_SUCCESS;
    }
  }
  
  return rc;  
}

/**************************************************************************************************
 * @fn          delete_ed_connect
 *
 * @brief       ɾ��Ŀ��ڵ�.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_DELETE_ED_CONNECT_FAIL     - ����������Ϣʧ��
 *              APP_NET_STATUS_SUCCESS      - ����������Ϣ�ɹ�
 **************************************************************************************************
 */
app_net_status_t delete_ed_connect(const app_frame_t *in_frame)
{
  app_net_status_t  rc = APP_DELETE_ED_CONNECT_FAIL;
  uint8_t           msg_id_0 = in_frame->data[3];
  uint8_t           data_len = in_frame->data_len;

  if(msg_id_0 == 0x01)
  {   
    if((data_len==8)&&(!memcmp(&in_frame->data[4], nwk_getMyAddress(), NET_ADDR_SIZE)))//����������Ƿ�Ϊ�ҵĵ�ַ
    {
      delete_ed_connect_flag = TRUE;
      send_reply_msg(in_frame, NULL, 0);
      
      rc= APP_NET_STATUS_SUCCESS;
    }
  }
  
  return rc;  
}

