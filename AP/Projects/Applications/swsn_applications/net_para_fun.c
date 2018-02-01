/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_para_fun.c
// Description: User APP.
//              SWSN������Ƶ�������������ȡ.
// Author:      Leidi
// Version:     2.0
// Date:        2012-3-24
// History:     2014-9-26   Leidi ɾ��ȫ�ֱ���change_flag,check_flag. 
//                                ɾ������set_channel().
//              2014-9-26   Leidi ɾ�����ܴ����0x10~0x18�Ĳ�������/��ȡ����.
//                                �޸ĸ���������,ʹ������app_frame_t.
//                                ɾ��ԭ�е�����Ϣ.
//              2014-10-5   Leidi �澯֡����net_para_proc()����,���������Ӻ���.
//              2014-10-8   Leidi ����ⲿȫ�ֱ���sLinkID1,ͨ��������ȡRSSI��LQI,������ʹ�ö�ֵ0x01.
//              2014-11-10  Leidi �Ƴ�ͷ�ļ�stm8s_uart.h.
//                                ����������õ�ͷ�ļ�user_app_config.h.
//              2015-07-28  Laidandan   �޸ı��ļ��еĺ�������ӦSi4438��ƵоƬ.
//                                      ɾ������read_freqest().
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "net_para_fun.h"
#include "68_frm_proc.h"
#include "nwk_layer.h"
#include "firmware_info_proc.h"
#include "flash_start_addr.h"
#include "mrfi.h"
#include "net_list_proc.h"
#include "sx1276-LoRa.h"
extern uint8_t reset_system_flag ;
extern uint32_t band_rate;
extern volatile uint8_t bsp_reset_ap_config ;
uint8_t RefreshFirewareFlag = 0 ;
/***************************************************************************
 * @fn          net_para_proc
 *     
 * @brief       ��Ƶ����������ڣ�����DI2ȷ����DI2 = 0x01��
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
app_status_t net_para_proc(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  
  app_net_para_t para_rc= APP_NET_PARA_SUCCESS;
  uint8_t msg_id_1 = in_pkg->msg[2];
  uint8_t alarm_info[4];
  switch(msg_id_1)
  {
    case 0x01: para_rc = freq_proc(in_pkg, out_pkg, len); //Ƶ�ʴ���
               break;
    case 0x02: para_rc = chan_proc(in_pkg, out_pkg, len); //�ŵ�����
               break;
    case 0x03: para_rc = chanspc_proc(in_pkg, out_pkg, len); //�ŵ��������
               break;
    case 0x04: para_rc = rate_proc(in_pkg, out_pkg, len);  //�������ʴ���  
               break;
    case 0x06: //para_rc = manchter_proc(in_pkg, out_pkg, len);
               break;
    case 0x07:// para_rc = deviatn_proc(in_pkg, out_pkg, len);
               break;
    case 0x08: para_rc = modformat_proc(in_pkg, out_pkg, len); //���Ʒ�ʽ����
               break;
    case 0x09: //para_rc = whiting_proc(in_pkg, out_pkg, len);
               break;
    case 0x0a: para_rc = txpw_proc(in_pkg, out_pkg, len); //���书��
               break;
    case 0x0b: //para_rc = read_partnum(in_pkg, out_pkg, len);
               break;
    case 0x0c: //para_rc = read_version(in_pkg, out_pkg, len);
               break;
    case 0x0f: para_rc = read_rssi(in_pkg, out_pkg, len); //��ȡrssi
               break;
		case 0x20: para_rc = bandrate_proc(in_pkg, out_pkg, len); //��������
               break;
    case 0x2C: para_rc = add_ap_rssi(in_pkg, out_pkg, len);// ����������RSSIֵ����֡��
               break;
		case 0x30: para_rc = device_type_proc(in_pkg, out_pkg, len);//�豸���ͻ�ȡ
               break;
    case 0x80: para_rc = begin_firmware_update (in_pkg, out_pkg, len);//��ʼ�̼���Ϣ
               break;
    case 0x88: para_rc = end_firmware_update (in_pkg, out_pkg, len);//�����̼���Ϣ,
               break;
    case 0xff: para_rc = set_my_device_address(in_pkg, out_pkg, len);//���������͡���ַ�ͼ�������ַд��flash
               break;
    default:   para_rc = no_para_fun(in_pkg, out_pkg, len);// δʹ�õ���Ƶ�������ú���.
  }
  
  
  if(para_rc ==APP_NET_PARA_SUCCESS)
  {  
    return APP_SUCCESS;
  }
  else if(para_rc == APP_RSSI_NO_ED_FIND)
  {
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x02;     
    alarm_info[2] = 0x02;
    alarm_info[3] = 0x05;
    *len =bui_pkg_alarm(in_pkg, alarm_info, 4, out_pkg); 
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
 * @fn          no_para_fun
 *
 * @brief       δʹ�õ���Ƶ�������ú���.
 *
 * @param       in_frame  - �����֡�ṹ��
 *              out_msg   - ������ֽ�����
 *              out_len   - ������ֽ����鳤��
 *
 * @return      NO_PAPA_FUN_FAIL     - ������Ƶ�������ú���ʧ��
 **************************************************************************************************
 */
app_net_para_t no_para_fun(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  app_net_para_t  rc = NO_PAPA_FUN_FAIL;
  
  return rc;
}

/***************************************************************************
 * @fn          freq_proc
 *     
 * @brief       Ƶ�ʴ���DI1 = 0x01��DI0 = 0 ʱΪ��Ƶ�ʣ�DI0 = 1ʱΪ����Ƶ�ʡ�
 *              ע��:���ü�����Ƶ��ǰ���������ýڵ�Ƶ�ʣ�����ᵼ�¼������ͽڵ�
 *              �޷�ͨ�š�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS - �ɹ�
 *              APP_FREQ_READ_FAIL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t freq_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t rc = APP_FREQ_PROC_FAIL;
  uint8_t msg_id_0= in_pkg->msg[3];
  uint8_t freq_temp[4];
    
  if(msg_id_0==0)
  {
    //Mrfi_GetProperty(PROP_FREQ_CONTROL_GROUP, 4, PROP_FREQ_CONTROL_INTE, freq_temp);
		Get_Radio_fre(freq_temp);
    *len = bui_rep_pkg_command(in_pkg,freq_temp,4,out_pkg);
    
    rc= APP_NET_PARA_SUCCESS;    
  }
  else if(msg_id_0 == 0x01)
  {
    
    freq_temp[0] = in_pkg->msg[4];
    freq_temp[1] = in_pkg->msg[5];
    freq_temp[2] = in_pkg->msg[6];
    freq_temp[3] = in_pkg->msg[7];
      
//    Mrfi_SetProperty(PROP_FREQ_CONTROL_GROUP, 4, PROP_FREQ_CONTROL_INTE, freq_temp);
    *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg); 
    
    rc= APP_NET_PARA_SUCCESS;        
  }  
  return rc;
  
}

/***************************************************************************
 * @fn          chan_proc
 *     
 * @brief       �ŵ�����DI1 = 0x02��DI0 = 0 ʱΪ���ŵ���DI0 = 1ʱΪ�����ŵ���
 *              ע��:���ü�����Ƶ��ǰ���������ýڵ��ŵ�������ᵼ�¼������ͽڵ�
 *              �޷�ͨ�š�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS - �ɹ�
 *              APP_CHAN_READ_FALL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t chan_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc=APP_CHAN_PROC_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   chan_temp[1];

  if(msg_id_0==0)
  {  
    chan_temp[0] = MRFI_GetChannelNum();   
    *len = bui_rep_pkg_command(in_pkg,chan_temp,1,out_pkg);   
    
    rc= APP_NET_PARA_SUCCESS;         
  }
  else if(msg_id_0 == 0x01)
  {  
    MRFI_SetChannelNum(in_pkg->msg[4]);     
    *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);
    
    rc= APP_NET_PARA_SUCCESS;    
  }
  return rc; 
}

/***************************************************************************
 * @fn          chanspc_proc
 *     
 * @brief       �ŵ��������DI1 = 0x03��DI0 = 0 ʱΪ���ŵ������DI0 = 1ʱ
 *              Ϊ�����ŵ������ע��:���ü�����Ƶ��ǰ���������ýڵ��ŵ������
 *              ����ᵼ�¼������ͽڵ��޷�ͨ�š�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS    - �ɹ�
 *              APP_CHANSPC_READ_FALL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t chanspc_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_CHANSPC_PROC_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   chanspc_temp[2];
  
  if(msg_id_0==0)
  {
    //Mrfi_GetProperty(PROP_FREQ_CONTROL_GROUP, 2, PROP_FREQ_CONTROL_CHANNEL_STEP_SIZE_1, chanspc_temp); 
		 chanspc_temp[0]=0x88;
		 chanspc_temp[1]=0x88;
    *len = bui_rep_pkg_command(in_pkg,chanspc_temp,2,out_pkg);

    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
 
  } 
  return rc;
  
}

/***************************************************************************
 * @fn          rate_proc
 *     
 * @brief       �������ʴ���DI1 = 0x04��DI0 = 0 ʱΪ���������ʣ�DI0 = 1ʱ
 *              Ϊ�����������ʡ�ע��:���ü�����Ƶ��ǰ���������ýڵ��������ʣ�
 *              ����ᵼ�¼������ͽڵ��޷�ͨ�š�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS - �ɹ�
 *              APP_RATE_READ_FALL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t rate_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_RATE_PROC_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   rate_temp[3];
  
  if(msg_id_0==0)
  {
    //Mrfi_GetProperty(PROP_MODEM_GROUP, 7, PROP_MODEM_DATA_RATE_2, rate_temp);
		Get_Radio_Rate(rate_temp);
    *len = bui_rep_pkg_command(in_pkg,rate_temp,3,out_pkg); 
    
    rc= APP_NET_PARA_SUCCESS;    
  }
  else if(msg_id_0 == 0x01)
  {
//    rate_temp[0] = in_pkg->msg[4];
//    rate_temp[1] = in_pkg->msg[5];
//    rate_temp[2] = in_pkg->msg[6];
//    rate_temp[3] = in_pkg->msg[7];
//    rate_temp[4] = in_pkg->msg[8];
//    rate_temp[5] = in_pkg->msg[9];
//    rate_temp[6] = in_pkg->msg[10];
//    
//    Mrfi_SetProperty(PROP_MODEM_GROUP, 7, PROP_MODEM_DATA_RATE_2, rate_temp);   
//    *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);     

//    rc= APP_NET_PARA_SUCCESS;        
  }
  return rc;  
}


/***************************************************************************
 * @fn          deviatn_proc
 *     
 * @brief       �ŵ�����DI1 = 0x07��DI0 = 0 ʱΪ��Ƶƫ��DI0 = 1ʱΪ����Ƶƫ��
 *              ע��:���ü�����Ƶ��ǰ���������ýڵ�Ƶƫ������ᵼ�¼������ͽڵ�
 *              �޷�ͨ�š�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS   - �ɹ�
 *              APP_DEVIATN_READ_FALL  - ʧ��
 ***************************************************************************
 */ 
app_net_para_t deviatn_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_DEVIATN_PROC_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   deviatn_temp[3];
  
  if(msg_id_0==0)
  {
//    Mrfi_GetProperty(PROP_MODEM_GROUP, 3, PROP_MODEM_FREQ_DEV_2, deviatn_temp);
    *len = bui_rep_pkg_command(in_pkg,deviatn_temp,3,out_pkg);     

    rc= APP_NET_PARA_SUCCESS;        
  }
  else if(msg_id_0 == 0x01)
  {
    deviatn_temp[0] = in_pkg->msg[4];
    deviatn_temp[1] = in_pkg->msg[5];
    deviatn_temp[2] = in_pkg->msg[6];
    
//    Mrfi_SetProperty(PROP_MODEM_GROUP, 3, PROP_MODEM_FREQ_DEV_2, deviatn_temp); 
    *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);  
    
    rc= APP_NET_PARA_SUCCESS;        
  } 
  return rc;
}

/***************************************************************************
 * @fn          modformat_proc
 *     
 * @brief       ���Ʒ�ʽ����DI1 = 0x08��DI0 = 0 ʱΪ�����Ʒ�ʽ��DI0 = 1ʱ
 *              Ϊ���õ��Ʒ�ʽ��ע��:���ü�����Ƶ��ǰ���������ýڵ���Ʒ�ʽ��
 *              ����ᵼ�¼������ͽڵ��޷�ͨ�š���ѡ���Ʒ�ʽΪ2FSK��2GFSK��
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS     - �ɹ�
 *              APP_MODFORMAT_READ_FALL  - ʧ��
 ***************************************************************************
 */ 
app_net_para_t modformat_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_MODFORMAT_PROC_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   modformat_temp[7];
  
  if(msg_id_0==0)
  {    
		Get_Radio_mode( modformat_temp);
    *len = bui_rep_pkg_command(in_pkg,modformat_temp,6,out_pkg);  

    rc= APP_NET_PARA_SUCCESS;        
  }
  else if(msg_id_0 == 0x01)
  {      
  }
  return rc;  
}



/***************************************************************************
 * @fn          txpw_proc
 *     
 * @brief       ���书�ʴ���DI1 = 0x0a��DI0 = 0 ʱΪ�����书�ʣ�DI0 = 1ʱ
 *              Ϊ���÷��书�ʡ�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS - �ɹ�
 *              APP_TXPW_READ_FALL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t txpw_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_TXPW_PROC_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   txpw_temp[1];
  
  if(msg_id_0==0)
  {
    //Mrfi_GetProperty(PROP_PA_GROUP, 1, PROP_PA_PWR_LVL, txpw_temp);
		Get_Radio_Txpower(txpw_temp);
    *len = bui_rep_pkg_command(in_pkg,txpw_temp,1,out_pkg);
    
    rc= APP_NET_PARA_SUCCESS;        
  }
  else if(msg_id_0 == 0x01)
  {
    txpw_temp[0] = in_pkg->msg[4];
    
		Set_Radio_Txpower(txpw_temp);
		
		Get_Radio_Txpower(txpw_temp);
//    Mrfi_SetProperty(PROP_PA_GROUP, 1, PROP_PA_PWR_LVL, txpw_temp);  
		
    *len = bui_rep_pkg_command(in_pkg,txpw_temp,1,out_pkg);  

    rc= APP_NET_PARA_SUCCESS;        
  } 
  return rc;  
}


/***************************************************************************
 * @fn          read_rssi
 *     
 * @brief       ���ڵ�RSSIֵ��DI1=0x0f��DI0=0����ȡ�������д洢�Ľڵ�RSSIֵ��
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS - �ɹ�
 *              APP_RSSI_READ_FALL   - ʧ��
 *              APP_RSSI_NO_ED_FIND  - �Ҳ�����Ӧ�ڵ�
 ***************************************************************************
 */ 
app_net_para_t read_rssi(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_para_t  rc= APP_RSSI_READ_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   ed_addr_1 = in_pkg->msg[5];
  uint8_t   ed_addr_0 = in_pkg->msg[6];
  uint16_t  ed_addr = (ed_addr_1<<8) | ed_addr_0;
  int8_t    rssi;
  if(msg_id_0==0)
  {  
    uint8_t readrssi_temp[3];
    struct ip_mac_id *p = NULL;
    readrssi_temp[1] = ed_addr_0;
    readrssi_temp[2] = ed_addr_1;
    
    if(psearch_ip(&ed_addr, &p))
    { 
      if(p->id != 0x00 && p->id != 0xff)
      {
        if(nwk_getEDRssi(p->id,&rssi))
        {
          readrssi_temp[0] = rssi;
          *len = bui_rep_pkg_command(in_pkg,readrssi_temp,3,out_pkg);
          rc= APP_NET_PARA_SUCCESS;
        }
      }
      else
      {
        rc= APP_RSSI_NO_ED_FIND;  
      }
    }
    else
    {
      rc= APP_RSSI_NO_ED_FIND;  
    }
  }
  else
  {
    rc= APP_RSSI_READ_FALL; 
  }
  return rc; 
}

/***************************************************************************
 * @fn          add_ap_rssi
 *     
 * @brief       ����������RSSIֵ����֡��
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS - �ɹ�
 *              APP_RSSI_READ_FALL   - ʧ��
 *              APP_RSSI_NO_ED_FIND  - �Ҳ�����Ӧ�ڵ�
 ***************************************************************************
 */ 
app_net_para_t add_ap_rssi(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_para_t  rc= APP_RSSI_READ_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   ed_addr_1 = (in_pkg->ed_addr)>>8;
  uint8_t   ed_addr_0 = in_pkg->ed_addr;
  uint16_t  ed_addr = (ed_addr_1<<8) | ed_addr_0;
  int8_t    rssi;
  if(msg_id_0==0)
  {  
    uint8_t readrssi_temp[4];
    struct ip_mac_id *p = NULL;
    readrssi_temp[3] = ed_addr_0;
    readrssi_temp[2] = ed_addr_1;
    readrssi_temp[0] = in_pkg->msg[4];
		
    if(psearch_ip(&ed_addr, &p))
    { 
      if(p->id != 0x00 && p->id != 0xff)
      {
        if(nwk_getEDRssi(p->id,&rssi))
        {
          readrssi_temp[1] = rssi;
          *len = bui_rssi_rep_pkg_command(in_pkg,readrssi_temp,4,out_pkg);
          rc= APP_NET_PARA_SUCCESS;
        }
      }
      else
      {
        rc= APP_RSSI_NO_ED_FIND;  
      }
    }
    else
    {
      rc= APP_RSSI_NO_ED_FIND;  
    }
  }
  else
  {
    rc= APP_RSSI_READ_FALL; 
  }
  return rc; 
}

/***************************************************************************
 * @fn          bandrate_proc
 *     
 * @brief       ���ڲ��������ö�ȡ��DI1 = 0x03��DI0 = 0 ʱΪ�����ڲ����ʣ�DI0 = 1ʱ
 *              Ϊ���ô��ڲ�����
 *     
 * @data        2017��05��08��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS    - �ɹ�
 *              APP_CHANSPC_READ_FALL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t bandrate_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_CHANSPC_PROC_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   band_rate_res[2];
  
  if(msg_id_0==0)
  {
		if(rate_to_arry(&band_rate,band_rate_res))
		{
		*len = bui_rep_pkg_command(in_pkg,band_rate_res,2,out_pkg);
    rc= APP_NET_PARA_SUCCESS;
		}
  }
  else if(msg_id_0 == 0x01)
  {
    band_rate_res[0] = in_pkg->msg[4];
    band_rate_res[1] = in_pkg->msg[5];
		
		if(change_to_rate((&(in_pkg->msg[4])),&band_rate))
    {	
    *len = bui_rep_pkg_command(in_pkg,band_rate_res,2,out_pkg);   
						
    rc= APP_NET_PARA_SUCCESS;  		
		}  
  } 
  return rc;
  
}

/***************************************************************************
 * @fn          device_type_proc
 *     
 * @brief       �豸���ͻ�ȡ��00Ϊ���أ�01Ϊ�ڵ�
 *     
 * @data        2017��05��08��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS    - �ɹ�
 *              APP_CHANSPC_READ_FALL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t device_type_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_CHANSPC_PROC_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   device_type[1]={0xFF};
  
  if(msg_id_0==0)
  {
#ifdef ACCESS_POINT
		device_type[0]=0x00;
#endif	
#ifdef END_DEVICE
		device_type[0]=0x01;
#endif		
		*len = bui_rep_pkg_command(in_pkg,device_type,1,out_pkg);
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {		
    return rc; 		
  } 
  return rc;
}


/***************************************************************************
 * @fn          begin_firmware_update
 *     
 * @brief       ��ʼ�̼���Ϣ,DI1=0x80,DI0=1������Ϊ4���ֽڣ���ʾ�汾�ţ���WXYZ��
 *              W:�汾�ţ�01��WiFi��02����������X����.Y����.Z���ա�
 *     
 * @data        2015��09��28��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS             - �ɹ�
 *              APP_BEGIN_FIRMWARE_UPDATE_FAIL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t begin_firmware_update(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_BEGIN_FIRMWARE_UPDATE_FAIL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  
  if(msg_id_0 == 0x01)
  {
    uint8_t i =0 ;
    uint32_t cur_version = *(uint32_t*) FIRMWARE_INFO_PTR;
    uint32_t fresh_version =  (in_pkg->msg[4]<<24) | (in_pkg->msg[5]<<16) |\
                              (in_pkg->msg[6]<<8) | in_pkg->msg[7];
    uint8_t cur_version_tmp[4] = {0};
    
    if(fresh_version>cur_version)
    {
      RefreshFirewareFlag = 1;  //׼����ʼ�̼�����
    }
    for(i=0; i<4; i++)
    {
      cur_version_tmp[i] = (cur_version>>((3-i)*8)); 
    }  

    *len = bui_rep_pkg_command(in_pkg,cur_version_tmp,4,out_pkg);//��������֡
        
    rc = APP_NET_PARA_SUCCESS;
  } 
  return rc;
  
}

/***************************************************************************
 * @fn          end_firmware_update
 *     
 * @brief       �����̼���Ϣ,DI1=0x88,DI0=1������Ϊ4���ֽڣ���ʾ�汾�ţ���WXYZ��
 *              W:�汾�ţ�01��WiFi��02����������X����.Y����.Z���ա��յ�����֡��
 *              ��������
 *     
 * @data        2015��09��28��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS             - �ɹ�
 *              APP_END_FIRMWARE_UPDATE_FAIL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t end_firmware_update(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_END_FIRMWARE_UPDATE_FAIL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  
  if(msg_id_0 == 0x01)
  {
    uint8_t i =0 ;
    uint32_t cur_version = *(uint32_t*) FIRMWARE_INFO_PTR;
    uint32_t fresh_version =  (in_pkg->msg[4]<<24) | (in_pkg->msg[5]<<16) |\
                              (in_pkg->msg[6]<<8) | in_pkg->msg[7];
    uint8_t cur_version_tmp[4] = {0};
    
    if(fresh_version>cur_version)
    {
       cur_version = fresh_version;
       RefreshFirewareFlag = 0;//�̼����½���
    }
    for(i=0; i<4; i++)
    {
      cur_version_tmp[i] = (cur_version>>((3-i)*8)); 
    }  

    *len = bui_rep_pkg_command(in_pkg,cur_version_tmp,4,out_pkg);//��������֡
    
    WriteCodeVersionInfoToFlash(&cur_version);//����FLASH�еı�����ַ
    
    reset_system_flag = 1;//����׼������
        
    rc = APP_NET_PARA_SUCCESS;
  } 
  return rc;
  
}
/***************************************************************************
 * @fn          set_my_device_address
 *     
 * @brief       ���������͡���ַ�ͼ�������ַд��flash,DI1=0xff,DI0=1������
 *              Ϊ10�ֽڣ����������У���������(AB)+���ص�ַ(CDEF)+������MAC
 *              ��ַ(GHIJ)����������Ǹ�λ��ǰ��λ�ں���
 *              00 fb + 00 00 00 01 + fe 08 09 19��
 *              ע�⣺������������豸����ʱʹ��һ�Σ�֮��Ӧ��ʹ�ã��Ҳ�Ӧ
 *              �ṩ���ͻ�ʹ�á�
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS   - �ɹ�
 *              APP_SET_ADDRESS_FALL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t set_my_device_address(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_SET_ADDRESS_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  
  if(msg_id_0 == 0x01)
  {
    /* ��������򳤶ȣ�������ַ�ĵ�1�ֽڲ���Ϊ0x00��0xff */
    uint8_t addr_temp[12];
    
    addr_temp[0] = in_pkg->msg[4];
    addr_temp[1] = in_pkg->msg[5];
    addr_temp[2] = in_pkg->msg[6];
    addr_temp[3] = in_pkg->msg[7];
    addr_temp[4] = in_pkg->msg[8];
    addr_temp[5] = in_pkg->msg[9];
    addr_temp[6] = in_pkg->msg[10];
    addr_temp[7] = in_pkg->msg[11];
    addr_temp[8] = in_pkg->msg[12];
    addr_temp[9] = in_pkg->msg[13];
    addr_temp[10]= in_pkg->msg[14];
    addr_temp[11]= in_pkg->msg[15];    
    WriteMyAddressToFlash(addr_temp);//����FLASH�еı�����ַ

    *len = bui_rep_pkg_command(in_pkg,addr_temp,12,out_pkg);//��������֡
    
    bsp_reset_ap_config = 1;//����׼������
        
    rc = APP_NET_PARA_SUCCESS;
  } 
  return rc;
  
}

uint8_t  change_to_rate(uint8_t * msgin,uint32_t * bandrate)
{
  uint16_t rate=0;
	uint32_t int_rate=0;
	uint32_t band_rate_arry[6]={4800,9600,19200,38400,57600,115200};
	uint16_t i=0,rate_seq=10,differ_value=0xffff;
	rate |= (((*msgin)<<8)&0xF000);
	rate |= ((*msgin)&0x000F);
	rate |= (((*(msgin+1))<<4)&0x0FF0);
	int_rate=(8000000/rate);
	
	for(i=0;i<6;i++)
  {
	  if(band_rate_arry[i]>int_rate)
    {
		  if((band_rate_arry[i]-int_rate)<differ_value)
      {
			  differ_value=(band_rate_arry[i]-int_rate);
			  rate_seq=i;
			}
		}
		else if(band_rate_arry[i]<=int_rate)
    {
			if((int_rate-band_rate_arry[i])<differ_value)
			{
			  differ_value=(int_rate-band_rate_arry[i]);
			  rate_seq=i;
		  }
	  }

	}
	*bandrate=band_rate_arry[rate_seq];
  
	for(i=0;i<6;i++)
  {
	  if(*bandrate==band_rate_arry[i])
    {
			return 1;
		}
	}
	
	return 0; 
}

uint8_t  rate_to_arry(uint32_t * bandrate,uint8_t * respond)
{
	uint8_t rc=1;
  switch(* bandrate)
  {
	  case 115200: *respond=0x05; *(respond+1)=0x04;
		             break;
		case 57600:  *respond=0x0b; *(respond+1)=0x08;
		             break;
		case 38400:  *respond=0x01; *(respond+1)=0x0d;
		             break;
		case 19200:  *respond=0x01; *(respond+1)=0x1a;
		             break;
		case 9600:   *respond=0x01; *(respond+1)=0x34;
		             break;
		case 4800:   *respond=0x03; *(respond+1)=0x68;
		             break;
		default:     rc=0;
			           break;
	}
	return rc;

}
