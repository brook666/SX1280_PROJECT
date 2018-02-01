/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_status_fun.c
// Description: User APP.
//              SWSN����״̬����.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-27
// History:     2014-10-27  Leidi �޸ĺ���modify_ip_addr()��ʵ���߼�.
//              2014-10-29  Leidi �޸ĺ���delete_ed_connect()��ʵ���߼�.
//              2015-01-22  Leidi �޸ĺ���get_join_link_token():�����ֽ���.
//                                �޸ĺ���get_ed_info():����MAC���ֽ���.
//                                �޸ĺ���get_ap_mac():��Ϊ�㱨4�ֽ�MAC.
//                                ɾ����ȫ�ֱ���������:sLinkToken,sJoinToken.
//              2015-05-13  Leidi ���Ӻ���ԭ��set_join_ed().
//              2015-05-14  Leidi ɾ���ڵ�ʱ��Ҳɾ�����Join���豸�б��ж�Ӧ���豸.
*****************************************************************************/

#include <string.h>
#include "nwk_layer.h"
#include "bsp.h"
#include "mrfi_spi.h"
#include "net_status_fun.h"
#include "ap_inner_fun.h"
#include "68_frm_proc.h"
#include "net_list_proc.h"

extern volatile uint8_t bsp_reset_ap_config ;
uint8_t reset_system_flag = 0;
extern  uint8_t net_ed_num;
extern int32_t remoter_press_num ;
/***************************************************************************
 * @fn          net_status_proc
 *     
 * @brief       ����״̬������ڣ�����DI2ȷ����DI2 = 0x02��
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SUCCESS - �ɹ�
 *              APP_ERR     - ʧ��
 ***************************************************************************
 */ 
app_status_t net_status_proc(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  
  app_net_status_t  status_rc= APP_NET_STATUS_SUCCESS;
  uint8_t  msg_id_1 = in_pkg->msg[2];    
  uint8_t  alarm_info[4];
  
  switch(msg_id_1)
  {
    case 0x01:  status_rc = reset_system(in_pkg, out_pkg, len);//ϵͳ��λ
                break; 
    case 0x03:  status_rc = get_join_link_token(in_pkg, out_pkg, len);//��ȡ�������ƺ���������,
                break;
    case 0x10:  status_rc = get_net_info(in_pkg, out_pkg, len);//��ѯ���統ǰ�ڵ���
                break;
    case 0x11:  status_rc = get_ed_info(in_pkg, out_pkg, len);// ��ѯ�������м�¼�Ľڵ���Ϣ
                break;
    case 0x13:  status_rc = set_join_ed(in_pkg, out_pkg, len);
                break;
    case 0x20:  status_rc = get_ap_mac(in_pkg, out_pkg, len);//��ȡ������MAC��
                break;
    case 0x40:  status_rc = reset_ap(in_pkg, out_pkg, len);//�ָ�����������������
                break;
    case 0x90:  status_rc = get_ed_mac_and_ip(in_pkg, out_pkg, len);//��ѯĿ��ڵ��MAC��IP
                break;
    case 0x91:  status_rc = modify_ip_addr(in_pkg, out_pkg, len);//�޸�Ŀ��ڵ�IP��ַ
                break;
    case 0x92:  status_rc = delete_ed_connect(in_pkg, out_pkg, len);// ɾ��Ŀ��ڵ�
                break;
    case 0xa0:  status_rc = read_temoter_press_num(in_pkg, out_pkg, len);
                break;
    default:    status_rc = no_status_fun(in_pkg, out_pkg, len);
  }
       
  if(status_rc ==APP_NET_STATUS_SUCCESS)
  {  
    return APP_SUCCESS;
  }
  else if(status_rc == APP_THIS_ED_IS_NOT_FOUND)
  {
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x02;
    alarm_info[2] = 0x02;
    alarm_info[3] = 0x05;
    *len =bui_pkg_alarm(in_pkg, alarm_info, 4, out_pkg); 
    
    return APP_ERR;      
  }
  else if((status_rc == APP_MODIFY_IP_ADDR_FAIL)||(status_rc == APP_IP_CONFLICT))
  {
    return APP_ERR;     
  }
  else
  {
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x02;
    alarm_info[2] = 0x02;
    alarm_info[3] = 0x08;
    *len =bui_pkg_alarm(in_pkg, alarm_info, 4, out_pkg);  
    
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
app_net_status_t no_status_fun(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = NO_STATUS_FUN_FAIL;
   
  return rc;
}

/***************************************************************************
 * @fn          reset_system
 *     
 * @brief       ��ȡ�������ƺ���������,DI1 = 0x03��DI0 = 0��ǰ4�ֽ��Ǽ�������
 *              (��FE 08 01 05) ����4�ֽ�����������(��05 04 03 02) ��
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS        - �ɹ�
 *              APP_RESET_SYSTEM_FAIL         - ʧ��
 ***************************************************************************
 */ 
app_net_status_t reset_system(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_RESET_SYSTEM_FAIL;
  uint8_t  msg_id_0 = in_pkg->msg[3];
  
#ifdef DEBUG
  put_string_uart1("reset system!!!");
#endif
  
  if(msg_id_0 == 1)
  {

   *len = bui_rep_pkg_command(in_pkg, NULL, 0, out_pkg); 
    reset_system_flag = 1;
    rc= APP_NET_STATUS_SUCCESS;
  } 
  else
  {
  }
  
  return rc;
}
/***************************************************************************
 * @fn          get_join_link_token
 *     
 * @brief       ��ȡ�������ƺ���������,DI1 = 0x03��DI0 = 0��ǰ4�ֽ��Ǽ�������
 *              (��FE 08 01 05) ����4�ֽ�����������(��05 04 03 02) ��
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS        - �ɹ�
 *              APP_GET_JOIN_LINK_TOKEN_FAIL  - ʧ��
 ***************************************************************************
 */ 
app_net_status_t get_join_link_token(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_GET_JOIN_LINK_TOKEN_FAIL;
  uint8_t  msg_id_0 = in_pkg->msg[3];
  uint8_t  join_link_token[8];
  uint32_t join_token = swsn_htonl(DEFAULT_JOIN_TOKEN);
  uint32_t link_token = swsn_htonl(DEFAULT_LINK_TOKEN);
  
#ifdef DEBUG
  put_string_uart1("enter get_join_link_token!!!");
#endif
  
  if(msg_id_0 == 0)
  {
    memcpy(&join_link_token[0], &join_token, sizeof(join_token));//��������(��FE 08 01 05)
    memcpy(&join_link_token[4], &link_token, sizeof(link_token));//��������(��05 04 03 02)

   *len = bui_rep_pkg_command(in_pkg, join_link_token, 8, out_pkg); 
    
    rc= APP_NET_STATUS_SUCCESS;
  } 
  else
  {
  }
  
  return rc;
}


/***************************************************************************
 * @fn          get_net_info (SWSN���ù���_1_0_0_40�������ʹ��,��ɾ)
 *     
 * @brief       ��ѯ���統ǰ�ڵ���,DI1 = 0x10��DI0 = 0���ֽ�0����������֧��
 *              �����ڵ������ֽ�1������ӽڵ��������ֽ�2������ӣ������ڵ�
 *              �����ֽ�3���������ڵ��������Լ�������Ч��       
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS - �ɹ�
 *              APP_GET_NET_INFO_FAIL  - ʧ��
 ***************************************************************************
 */ 
app_net_status_t get_net_info(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_status_t  rc= APP_GET_NET_INFO_FAIL;
  uint8_t  msg_id_0 = in_pkg->msg[3];
  uint8_t  net_info[4];
  
  if(msg_id_0==0)
  {
    net_info[0] =  NUM_CONNECTIONS;
    net_info[1] =  nwk_getEDState(&net_info[2],&net_info[3]);

    *len = bui_rep_pkg_command(in_pkg,net_info,4,out_pkg);
    rc= APP_NET_STATUS_SUCCESS; 
  }
  else
  {
  }  
  return rc; 
}

/***************************************************************************
 * @fn          get_ed_info(���ڲ���)
 *     
 * @brief       ��ѯ�������м�¼�Ľڵ���Ϣ,DI1 = 0x11��DI0 = 0����������֡��
 *              �û�����1���ֽ�ΪĿ��ڵ���š����ڻظ�֡������9�ֽڣ��ֽ�0:
 *              �ڵ�ţ��ֽ�1:�ڵ�״̬(0-�ڵ�δ����,1-�ڵ��������,2-�ڵ���
 *              ����)���ֽ�2:�ڵ�HOP�����ֽ�3��6:MAC (��FE 08 03 01)���ֽ�
 *              7��8:IP(��03 01)���ֽ�9:RSSI���ֽ�10:LQI�����Լ�������Ч��
 *              MAC��ַΪ��λ��ǰ����λ�ں�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS   - �ɹ�
 *              APP_GET_ED_INFO_FAIL     - ʧ��
 *              APP_THIS_ED_IS_NOT_FOUND - ʧ�ܣ��ڵ㲻����
 ***************************************************************************
 */ 
app_net_status_t get_ed_info(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_GET_ED_INFO_FAIL;
  uint8_t   msg_id_0= in_pkg->msg[3];
  linkID_t  ed_lid = in_pkg->msg[4];
  uint8_t   ed_info[11]; 
  struct ip_mac_id *p = NULL;
  
  if(msg_id_0 == 0)
  {
    ed_info[0] = ed_lid;
    
    /* ͨ��LID���ҽڵ���Ϣ */
    if(nwk_getEDbasicInfo(ed_lid,&ed_info[1]) && psearch_id(ed_lid, &p))
    {
      ed_info[9] = ed_info[7];//RSSI
      ed_info[10] = ed_info[8];//LQI
      
      /* IP,�㱨����λ��ʱ����˳��(��03 01)����Ϊ֡�ṹ��˳����� */
      ed_info[7] = p->ip[1];
      ed_info[8] = p->ip[0];
      
      rc = APP_NET_STATUS_SUCCESS;
      *len = bui_rep_pkg_command(in_pkg, ed_info, 11, out_pkg);
    }
    else
    {
      rc = APP_THIS_ED_IS_NOT_FOUND;
    }
  }
  else
  {
  }  
  return rc;
}
/***************************************************************************
 * @fn          get_ap_mac(SWSN���ù���_1_0_0_40�������ʹ��,��ɾ)
 *     
 * @brief       ��ȡ������MAC��ַ��DI1 = 0x20��DI0 = 0������4�ֽڣ���ʾ������
 *              MAC��ַ������FE 08 09 19��MAC��ַΪ��λ��ǰ����λ�ں�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS - �ɹ�
 *              APP_GET_AP_MAC_FAIL    - ʧ��
 ***************************************************************************
 */ 
app_net_status_t get_ap_mac(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_GET_AP_MAC_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];
  uint8_t ap_mac[4];
  addr_t const *myromaddr;
  uint8_t i;
  
  
  if(msg_id_0==0)
  {
    /* ��ȡ������MAC(��19 09 08 FE) */
    myromaddr = nwk_getMyAddress();
    
    /* AP��MAC��ַ������㱨(��FE 08 09 19) */
    for(i = 0; i < 4; i++)
    {
      ap_mac[i] = myromaddr->addr[3-i];
    }
    
    rc= APP_NET_STATUS_SUCCESS;
    *len = bui_rep_pkg_command(in_pkg, ap_mac, 4, out_pkg);
  }
  else
  {
  }
  return rc;
}
/***************************************************************************
 * @fn          set_join_ed(ÿ���һ���ڵ㣬��Ҫʹ�øú���)
 *     
 * @brief       ɨ���ά�룬�ֶ������Ȩ�ڵ㣬DI1 = 0x13��DI0 = 0������ʱ
 *              ABCD����ڵ�MAC��ַ������ʱA��������ӵĽڵ�����������ֵΪ
 *              255�����ʾ���ʧ�ܡ�ֻ�Լ�������Ч��MAC��ַΪ��λ��ǰ��
 *              ��λ�ں�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS - �ɹ�
 *              APP_SET_JOIN_ED_FAIL   - ʧ��
 ***************************************************************************
 */ 
app_net_status_t set_join_ed(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_SET_JOIN_ED_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];
  mac_addr_t *joinAddr = NULL;
  uint8_t tmp[6];
  uint8_t i = 0;
  int8_t result = -1;
  
  if(msg_id_0 == 1)
  {
    if(in_pkg->data_len == 0x0a)
    {
      joinAddr = (mac_addr_t *)&in_pkg->msg[4];      
    }
    else if(in_pkg->data_len == 0x08)
    {
      for(i=0;i<4;i++)
      {
        tmp[i] = in_pkg->msg[4+i] ;
      }
      tmp[4] = in_pkg->msg[6];
      tmp[5] = in_pkg->msg[7];
      joinAddr = (mac_addr_t *)tmp;  
    }
    else
    {
    }
    
    if(-1 != (result = nwk_joinList_add(joinAddr)))
    {
      nwk_joinList_save();
    }
    memset(tmp,0x0,sizeof(tmp));
		tmp[0]=result;
		for(i=0;i<4;i++)
		{
			tmp[i+1] = in_pkg->msg[4+i];
		}
    rc = APP_NET_STATUS_SUCCESS;
    *len = bui_rep_pkg_command(in_pkg, tmp, 5, out_pkg);
  }
  else
  {
  }
  return rc;
}

/***************************************************************************
 * @fn          reset_ap
 *     
 * @brief       �ָ����������������ã�DI1 = 0x40��DI0 = 1���������б����
 *              �ڵ���Ϣȫ����������������������Խڵ���Ч��
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS - �ɹ�
 *              APP_RESET_AP_FAIL      - ʧ��
 ***************************************************************************
 */ 
app_net_status_t reset_ap(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_status_t  rc= APP_RESET_AP_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];    
  
  if(msg_id_0 == 0x01)
  {    
    bsp_reset_ap_config =0x01;
    rc= APP_NET_STATUS_SUCCESS; 
    *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);    
  }
  else
  {
  } 
  return rc;  
}

/***************************************************************************
 * @fn          get_ed_mac_and_ip
 *     
 * @brief       ��ѯĿ��ڵ��MAC��IP��DI1 = 0x90��DI0 = 0����������֡���û�
 *              ����1���ֽڣ�X��ΪĿ��ڵ���š����ڻظ�֡��ǰ4���ֽ�(ABCD)Ϊ
 *              �ڵ�MAC��ַ����2���ֽڣ�XY��Ϊ�ڵ�IP��ַ�����磺 
 *              5C 01 01 FE + 5C 01���˲�ѯ�Խڵ�ͼ���������Ч������Ŀ��Ϊ
 *              �ڵ�ʱ��Ŀ��ڵ���ſ�Ϊ����ֵ���ڵ����кŵĺϷ���Χ��
 *              1~NUM_CONNECTIONS(��������)��������Ϣ���λ��ǰ����λ�ں�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS      - �ɹ�
 *              APP_GET_ED_MAC_AND_IP_FAIL  - ʧ��
 ***************************************************************************
 */ 
app_net_status_t get_ed_mac_and_ip (app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_status_t  rc= APP_GET_ED_MAC_AND_IP_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3]; 
  uint8_t mac_and_ip[6];
  uint8_t i=0;
  
  if(msg_id_0 == 0x00)
  {
    struct ip_mac_id *p=NULL;
    
    if(psearch_id(in_pkg->msg[4], &p))//����������Ƿ�Ϊ�����е�LID
    {
      for(i=0;i<4;i++)
      {
        mac_and_ip[i] = p->mac[i];
      }
      for(i=0;i<2;i++)
      {
        mac_and_ip[i+4] = p->ip[i];
      }
      rc= APP_NET_STATUS_SUCCESS;
      *len = bui_rep_pkg_command(in_pkg,mac_and_ip,6,out_pkg); 
    }
    else
    {
    }    
  }
  else
  {
  }
  return rc;  
}

/***************************************************************************
 * @fn          modify_ip_addr
 *     
 * @brief       �޸�Ŀ��ڵ�IP��ַ��DI1 = 0x91��DI0 = 1��ǰ4���ֽ�(ABCD)ΪĿ��
 *              �ڵ�MAC��ַ����2���ֽڣ�XY��ΪҪ�޸ĵ�IP��ַ�����磺 
 *              5C 01 01 FE + 22 11��ֻ�跢�͸���������������ȷ����Ϣ��ȷ��ᷢ
 *              �͸��ڵ㡣���޸ĳɹ���Ӧ���յ�2������֡���ֱ����Լ�������ڵ㡣
 *              IP��ַ�ֱ𱣴��ڽڵ��뼯������FLASH�С�MAC��ַ��IP��ַ���ǵ�λ
 *              ��ǰ����λ�ں�ע�⣬0x68֡�ڵ��ַ���е�ֵʵ�����ǽڵ�IP(��ת)
 *              ��ַ��ʹ�ô�֡�޸�Ŀ��ڵ�IP��ַ֮�󣬷��͵��ڵ��0x68֡�Ľڵ��
 *              ַ�򶼱���ʹ���µ�IP��ַ�������޷���ڵ�ͨ�š�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS   - �ɹ�
 *              APP_MODIFY_IP_ADDR_FAIL  - ʧ��
 *              APP_IP_CONFLICT          - ��IP������IP��ͻ
 ***************************************************************************
 */ 
app_net_status_t modify_ip_addr(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t  rc= APP_MODIFY_IP_ADDR_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];    
  uint8_t alarm_info[10];
  
  if(msg_id_0 == 0x01)
  {
    addr_t mac_addr;
    uint16_t new_ip_addr;
    linkID_t lid;
    struct ip_mac_id *p = NULL;
    uint8_t send_pkg[MAX_APP_PAYLOAD];
    uint8_t send_pkg_len;
    uint8_t send_result;
    
    /* ���������л�ȡ�ڵ��MAC��ַ���µ�IP */
    memcpy(mac_addr.addr, &in_pkg->msg[4], MAC_ADDR_SIZE);
    new_ip_addr = in_pkg->msg[8] | (in_pkg->msg[9]<<8);
    
    /* ��ȡ�ڵ�MAC��ַ��Ӧ��LID */
    lid = nwk_getLinkIdByAddress(&mac_addr);
    
    /* �ڵ�MAC��ַ��������ӵĽڵ��б��� */
    if(lid != 0)
    {
      /* �����µ�IP�Ƿ������е�IP��ͻ */
      if(psearch_ip(&new_ip_addr, &p))
      {
        /* �µ�IP�����е�IP��ͻ */
        rc = APP_IP_CONFLICT;
      }
      else
      {
        /* ������ڵ㷢�͵�����֡,ͬʱ�޸��������� */
        send_pkg_len = Debui_app_pkg(in_pkg, 0x05, send_pkg);
        
        /* ��ڵ㷢���޸�IP������֡ */
        send_result = send_net_pkg(lid, send_pkg, send_pkg_len);
        
        /* ��ڵ㷢�ͳɹ� */
        if(send_result == 1)
        {
          /* �޸Ľڵ���Ϣ�����е�IP��ַ */
          Modify_IP_by_MAC(&mac_addr, new_ip_addr);
          
          rc= APP_NET_STATUS_SUCCESS;
        }
        
      }
    }
    
    if(rc!= APP_NET_STATUS_SUCCESS)
    { 
      alarm_info[0] = 0x00;
      alarm_info[1] = 0x02;      
      alarm_info[2] = 0x02;
      alarm_info[3] = 0x14;
      *len =bui_pkg_alarm(in_pkg, alarm_info, 4, out_pkg);  
    }
    else
    {   
      *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);    
    }   
  }
  return rc;  
}

/***************************************************************************
 * @fn          delete_ed_connect
 *     
 * @brief       ɾ��Ŀ��ڵ㣬DI1 = 0x92��DI0 = 1��4���ֽ�(ABCD)ΪĿ��ڵ�MAC
 *              ��ַ������5C 01 01 FE��ֻ�跢�͸���������������ȷ����Ϣ��ȷ��
 *              �ᷢ�͸��ڵ㲢ɾ���ڵ���Ϣ���ڵ㱻ɾ�����Ͽ����ӣ���������
 *              ��������ַ���ָ�����״̬�����������������������������в���ָ��
 *              �Ľڵ㲢ɾ�����Ͽ����ӡ�MAC��ַ��IP��ַ���ǵ�λ��ǰ����λ�ں�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS - �ɹ�
 *              ����                   - ʧ��
 ***************************************************************************
 */ 
app_net_status_t delete_ed_connect(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t  rc= APP_DELETE_ED_CONNECT_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];    
#ifdef DEBUG
  put_string_uart1("enter delete_ed_connect!!!");
#endif  
  
  if(msg_id_0 == 0x01)
  {
    addr_t mac_addr;
    linkID_t lid;
    uint8_t send_pkg[MAX_APP_PAYLOAD];
    uint8_t send_pkg_len;
    
    /* ���������л�ȡ�ڵ��MAC��ַ */
    memcpy(mac_addr.addr, &in_pkg->msg[4], MAC_ADDR_SIZE);
    
    /* ��ȡ�ڵ�MAC��ַ��Ӧ��LID */
    lid = nwk_getLinkIdByAddress(&mac_addr);

    /* �ڵ�MAC��ַ��������ӵĽڵ��б��� */
    if(lid != 0)
    {
      /* ������ڵ㷢�͵�����֡,ͬʱ�޸��������� */
      send_pkg_len = Debui_app_pkg(in_pkg, 0x05, send_pkg);
        
      /* ��ڵ㷢��ɾ���ڵ������֡ */
      send_net_pkg(lid, send_pkg, send_pkg_len);

      if(SWSN_deleteInnetED(mac_addr,lid))//ɾ�������еĽڵ�ɹ�
      {
        rc= APP_NET_STATUS_SUCCESS;
        *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);    
      }
      else
      {
      }   
    }
    else
    {
			//��������£��˴����øú���������0;
			//�˴����ã�ֻ��Ϊ�˷�ֹ�����д���û��ɾ�������ݡ�
			pdel_mac(mac_addr.addr);
			
			SWSN_DeleteEDByMAC(mac_addr);

      /*�ýڵ�û�м������磬�������Join���豸�б��д��ڸýڵ���Ϣ,
        �����Join���豸�б���ɾ����Ӧ���豸�������� */
      if(-1 != nwk_joinList_del(&mac_addr))
      {
        nwk_joinList_save();
      }
      rc= APP_NET_STATUS_SUCCESS;
      *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg); 
    }
  }
  else
  {
  }
  return rc;  
}


/***************************************************************************
 * @fn          delete_ed_connect
 *     
 * @brief       ɾ��Ŀ��ڵ㣬DI1 = 0x92��DI0 = 1��4���ֽ�(ABCD)ΪĿ��ڵ�MAC
 *              ��ַ������5C 01 01 FE��ֻ�跢�͸���������������ȷ����Ϣ��ȷ��
 *              �ᷢ�͸��ڵ㲢ɾ���ڵ���Ϣ���ڵ㱻ɾ�����Ͽ����ӣ���������
 *              ��������ַ���ָ�����״̬�����������������������������в���ָ��
 *              �Ľڵ㲢ɾ�����Ͽ����ӡ�MAC��ַ��IP��ַ���ǵ�λ��ǰ����λ�ں�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_STATUS_SUCCESS - �ɹ�
 *              ����                   - ʧ��
 ***************************************************************************
 */ 
app_net_status_t read_temoter_press_num(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t  rc= APP_DELETE_ED_CONNECT_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];    

#ifdef DEBUG
  put_string_uart1("enter delete_ed_connect!!!");
#endif  
  
  if(msg_id_0 == 0x00)
  {
    uint8_t tmp[4] ={0};
    tmp[0] = remoter_press_num;
    tmp[1] = (remoter_press_num>>8);
    tmp[2] = (remoter_press_num>>16);
    tmp[3] = (remoter_press_num>>24);
    
    rc= APP_NET_STATUS_SUCCESS;
    *len = bui_rep_pkg_command(in_pkg,tmp,4,out_pkg);    
  }
  else
  {
  }
  return rc;  
}





