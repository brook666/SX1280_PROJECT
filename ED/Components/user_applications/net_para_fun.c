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
#include "bsp_mac_addr.h"
#include "nwk_api.h"
#include "mrfi_si4438_api.h"
#include "mrfi_spi.h"
#include "bsp.h"
#include "sx1276-LoRa.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */


/**************************************************************************************************
 * @fn          net_para_proc
 *
 * @brief       ����������Ƶ�������ϲ㺯��,���崦����̽������Ӻ���.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_ERR     - ��������״̬����ʧ��
 *              APP_SUCCESS - ��������״̬�����ɹ�
 **************************************************************************************************
 */
app_status_t net_para_proc(const app_frame_t *in_frame)
{
  uint8_t alarm_info[4];
  app_net_para_t para_rc= APP_NET_PARA_SUCCESS;
  
  switch(in_frame->data[2])
  {
    case 0x01:  para_rc = freq_proc(in_frame);
                break;
    case 0x02:  para_rc = chan_proc(in_frame);
                break;
    case 0x03:  para_rc = chanspc_proc(in_frame);
                break;
    case 0x04:  para_rc = rate_proc(in_frame);
                break;
    case 0x05:  //para_rc = rxbw_proc(in_frame);
                break;
    case 0x06:  //para_rc = manchter_proc(in_frame);
                break;
    case 0x07:  //para_rc = deviatn_proc(in_frame);
                break;
    case 0x08:  para_rc = modformat_proc(in_frame);
                break;
    case 0x09:  //para_rc = whiting_proc(in_frame);
                break;
//    case 0x0a:  para_rc = txpw_proc(in_frame);
//                break;
    case 0x0b:  //para_rc = read_partnum(in_frame);
                break;
    case 0x0c:  //para_rc = read_version(in_frame);
                break;
    case 0x0e:  //para_rc = read_lqi(in_frame);
                break;
    case 0x0f:  para_rc = read_rssi(in_frame);
                break;
    case 0x19:  //para_rc = fec_proc(in_frame);
                break;
		case 0x2C:  para_rc = read_rssi(in_frame);
                break;		
    case 0xff:  para_rc = set_my_device_address(in_frame);
                break;
    default:    para_rc = no_para_fun(in_frame);
  }

  if(para_rc == APP_NET_PARA_SUCCESS)
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
 * @fn          no_para_fun
 *
 * @brief       δʹ�õ���Ƶ�������ú���.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      NO_PAPA_FUN_FAIL     - ������Ƶ�������ú���ʧ��
 **************************************************************************************************
 */
app_net_para_t no_para_fun(const app_frame_t *in_frame)
{
  app_net_para_t  rc = NO_PAPA_FUN_FAIL;
  
  return rc;
}

/**************************************************************************************************
 * @fn          freq_proc
 *
 * @brief       ����/��ȡ��ƵоƬ���ź�����Ƶ��.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_FREQ_READ_FAIL    - ������Ƶ����ʧ��
 *              APP_NET_PARA_SUCCESS  - ������Ƶ�����ɹ�
 **************************************************************************************************
 */
app_net_para_t freq_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_FREQ_READ_FAIL;
  uint8_t 		 msg_id_0 = in_frame->data[3];
  uint8_t        freq_temp[4];
  
  if(msg_id_0 == 0x00)
  {
    //SMPL_GET_RADIO_PROPERTY(PROP_FREQ_CONTROL_GROUP, 4, PROP_FREQ_CONTROL_INTE, freq_temp);
		Get_Radio_fre(freq_temp);
    send_reply_msg(in_frame, freq_temp, 4);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {

		
		
  }

  return rc; 
}

/**************************************************************************************************
 * @fn          chan_proc
 *
 * @brief       ����/��ȡ��ƵоƬ���ŵ���.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_CHAN_READ_FALL    - ������Ƶ����ʧ��
 *              APP_NET_PARA_SUCCESS  - ������Ƶ�����ɹ�
 **************************************************************************************************
 */
app_net_para_t chan_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc=APP_CHAN_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        chan_temp[1];
  
  if(msg_id_0 == 0x00)
  {
    chan_temp[0] = MRFI_GetChannelNum();
    
    send_reply_msg(in_frame, chan_temp, 1);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    MRFI_SetChannelNum(in_frame->data[4]);
    
    rc= APP_NET_PARA_SUCCESS;
  }

  return rc;
}

/**************************************************************************************************
 * @fn          chan_proc
 *
 * @brief       ����/��ȡ��ƵоƬ���ŵ����.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_CHANSPC_READ_FALL - ������Ƶ����ʧ��
 *              APP_NET_PARA_SUCCESS  - ������Ƶ�����ɹ�
 **************************************************************************************************
 */
app_net_para_t chanspc_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_CHANSPC_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        chanspc_temp[2];
  
  if(msg_id_0 == 0x00)
  {
//    SMPL_GET_RADIO_PROPERTY(PROP_FREQ_CONTROL_GROUP, 2, PROP_FREQ_CONTROL_CHANNEL_STEP_SIZE_1, chanspc_temp); 
    chanspc_temp[0]=0x88;
		chanspc_temp[1]=0x88;
    send_reply_msg(in_frame, chanspc_temp, 2);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    chanspc_temp[0] = in_frame->data[4];
    chanspc_temp[1] = in_frame->data[5];
    
//    SMPL_SET_RADIO_PROPERTY(PROP_FREQ_CONTROL_GROUP, 2, PROP_FREQ_CONTROL_CHANNEL_STEP_SIZE_1, chanspc_temp);
    
    rc= APP_NET_PARA_SUCCESS;
  } 
  return rc;
}

/**************************************************************************************************
 * @fn          rate_proc
 *
 * @brief       ����/��ȡ��ƵоƬ����������.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_RATE_READ_FALL - ������Ƶ����ʧ��
 *              APP_NET_PARA_SUCCESS  - ������Ƶ�����ɹ�
 **************************************************************************************************
 */
app_net_para_t rate_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_RATE_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        rate_temp[3];
  
  if(msg_id_0 == 0x00)
  {
    //SMPL_GET_RADIO_PROPERTY(PROP_MODEM_GROUP, 7, PROP_MODEM_DATA_RATE_2, rate_temp);
    Get_Radio_Rate(rate_temp);
    send_reply_msg(in_frame, rate_temp, 3);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {

  }

  return rc;
}

/**************************************************************************************************
 * @fn          rxbw_proc
 *
 * @brief       ����/��ȡ��ƵоƬ�Ľ����˲�������.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_RXBW_READ_FALL - ������Ƶ����ʧ��
 *              APP_NET_PARA_SUCCESS  - ������Ƶ�����ɹ�
 **************************************************************************************************
 */
app_net_para_t rxbw_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_RXBW_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        rxbw_temp[1];
  
  if(msg_id_0 == 0x00)
  {
    
    send_reply_msg(in_frame, rxbw_temp,1);
    
    /* δʵ�ִ˹��� */
    //rc = APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    /* δʵ�ִ˹��� */
    
    //rc = APP_NET_PARA_SUCCESS;
  } 
  
  return rc;
}

/**************************************************************************************************
 * @fn          modformat_proc
 *
 * @brief       ����/��ȡ��ƵоƬ�ĵ��Ʒ�ʽ.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_MODFORMAT_READ_FALL - ������Ƶ����ʧ��
 *              APP_NET_PARA_SUCCESS  - ������Ƶ�����ɹ�
 **************************************************************************************************
 */
app_net_para_t modformat_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_MODFORMAT_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        modformat_temp[7];
  
  if(msg_id_0 == 0x00)
  {
    //SMPL_GET_RADIO_PROPERTY(PROP_MODEM_GROUP, 6, PROP_MODEM_MOD_TYPE, modformat_temp);
		Get_Radio_mode( modformat_temp);
    send_reply_msg(in_frame, modformat_temp, 6);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {

   }

  return rc;
}

/**************************************************************************************************
 * @fn          read_rssi
 *
 * @brief       ��ȡ��ƵоƬ��RSSI.
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_RSSI_READ_FALL - ������Ƶ����ʧ��
 *              APP_NET_PARA_SUCCESS  - ������Ƶ�����ɹ�
 **************************************************************************************************
 */
app_net_para_t read_rssi(const app_frame_t *in_frame)
{
  app_net_para_t  rc = APP_RSSI_READ_FALL;
  uint8_t 	      msg_id_0 = in_frame->data[3];
  uint8_t         readrssi_temp[1];
  extern linkID_t sLinkID1;      //���ӱ�ʾ��
  
  if(msg_id_0 == 0x00)
  {
    ioctlRadioSiginfo_t sample; //���ڱ���RSSI�����ı���
    sample.lid = sLinkID1;      //��ѯ��APͨ�ŵ�����
    
    if(SMPL_SUCCESS==SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SIGINFO, &sample))
    {
      readrssi_temp[0] = sample.sigInfo.rssi;
      
      send_reply_msg(in_frame, readrssi_temp,1);
      
      rc= APP_NET_PARA_SUCCESS;
    }
  }

  return rc; 
}



/**************************************************************************************************
 * @fn          set_my_device_address
 *
 * @brief       ���ñ����������ַ.(ע�⣺�˽ӿڽ��ڲ�ʹ��)
 *
 * @param       in_frame  - �����֡�ṹ��
 *
 * @return      APP_SET_ADDRESS_FALL - ������Ƶ����ʧ��
 *              APP_NET_PARA_SUCCESS  - ������Ƶ�����ɹ�
 **************************************************************************************************
 */
app_net_para_t set_my_device_address(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_SET_ADDRESS_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  extern uint8_t delete_ed_connect_flag;  //ED�Ͽ����ӱ�־,�˱�־��Ϊ0ʱED���Ͽ���AP������
  
   if(msg_id_0 == 0x01)
   {
     /* ��������򳤶ȣ�������ַ�ĵ�1�ֽڲ���Ϊ0x00��0xff */
     if((in_frame->data_len == 8) && (in_frame->data[4]!= 0x00) && (in_frame->data[4]!= 0xff))
     {  
       uint8_t myDeviceAddr[4];
       myDeviceAddr[0] = in_frame->data[4];
       myDeviceAddr[1] = in_frame->data[5];
       myDeviceAddr[2] = in_frame->data[6];
       myDeviceAddr[3] = in_frame->data[7];
      
       WriteMacAddressToFlash(myDeviceAddr);//����FLASH�еı�����ַ
       
       send_reply_msg(in_frame, myDeviceAddr,4);//��������֡
        
       delete_ed_connect_flag = TRUE;//�ڵ�׼������
        
       rc= APP_NET_PARA_SUCCESS;
     }
     else if((in_frame->data_len == 10) && (in_frame->data[4]!= 0x00) && (in_frame->data[4]!= 0xff))
     {
       uint8_t moudleInfo[6] = {0};
       uint8_t i = 0;
       
       for(i=0;i<6;i++)
       {
         moudleInfo[i] = in_frame->data[4+i];
       }
       
       WriteModuleInfoToFlash(moudleInfo);//����FLASH�еı�����ַ
       
       
       send_reply_msg(in_frame, moudleInfo,6);//��������֡
       
       delete_ed_connect_flag = TRUE;//�ڵ�׼������
        
       rc= APP_NET_PARA_SUCCESS;      
     }
  } 
  
  return rc;
}

/**************************************************************************************************
 */
