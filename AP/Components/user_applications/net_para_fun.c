/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_para_fun.c
// Description: User APP.
//              SWSN网络射频参数的设置与获取.
// Author:      Leidi
// Version:     2.0
// Date:        2012-3-24
// History:     2014-9-26   Leidi 删除全局变量change_flag,check_flag. 
//                                删除函数set_channel().
//              2014-9-26   Leidi 删除功能代码从0x10~0x18的参数设置/读取函数.
//                                修改各函数参数,使用类型app_frame_t.
//                                删除原有调试信息.
//              2014-10-5   Leidi 告警帧改由net_para_proc()构建,而不是其子函数.
//              2014-10-8   Leidi 添加外部全局变量sLinkID1,通过此来获取RSSI和LQI,而不是使用定值0x01.
//              2014-11-10  Leidi 移除头文件stm8s_uart.h.
//                                添加用于配置的头文件user_app_config.h.
//              2015-07-28  Laidandan   修改本文件中的函数以适应Si4438射频芯片.
//                                      删除函数read_freqest().
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

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */


/**************************************************************************************************
 * @fn          net_para_proc
 *
 * @brief       处理网络射频参数的上层函数,具体处理过程交由其子函数.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_ERR     - 处理网络状态参数失败
 *              APP_SUCCESS - 处理网络状态参数成功
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
    case 0x05:  para_rc = rxbw_proc(in_frame);
                break;
    case 0x06:  para_rc = manchter_proc(in_frame);
                break;
    case 0x07:  para_rc = deviatn_proc(in_frame);
                break;
    case 0x08:  para_rc = modformat_proc(in_frame);
                break;
    case 0x09:  para_rc = whiting_proc(in_frame);
                break;
    case 0x0a:  para_rc = txpw_proc(in_frame);
                break;
    case 0x0b:  para_rc = read_partnum(in_frame);
                break;
    case 0x0c:  para_rc = read_version(in_frame);
                break;
    case 0x0e:  para_rc = read_lqi(in_frame);
                break;
    case 0x0f:  para_rc = read_rssi(in_frame);
                break;
    case 0x19:  para_rc = fec_proc(in_frame);
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
 * @brief       未使用的射频参数设置函数.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      NO_PAPA_FUN_FAIL     - 处理射频参数设置函数失败
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
 * @brief       设置/获取射频芯片的信号中心频率.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_FREQ_READ_FAIL    - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t freq_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_FREQ_READ_FAIL;
  uint8_t 		 msg_id_0 = in_frame->data[3];
  uint8_t        freq_temp[4];
  
  if(msg_id_0 == 0x00)
  {
    SMPL_GET_RADIO_PROPERTY(PROP_FREQ_CONTROL_GROUP, 4, PROP_FREQ_CONTROL_INTE, freq_temp);
    send_reply_msg(in_frame, freq_temp, 4);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    freq_temp[0] = in_frame->data[4];
    freq_temp[1] = in_frame->data[5];
    freq_temp[2] = in_frame->data[6];
    freq_temp[3] = in_frame->data[7];
      
    SMPL_SET_RADIO_PROPERTY(PROP_FREQ_CONTROL_GROUP, 4, PROP_FREQ_CONTROL_INTE, freq_temp);
    
    rc= APP_NET_PARA_SUCCESS;
  }

  return rc; 
}

/**************************************************************************************************
 * @fn          chan_proc
 *
 * @brief       设置/获取射频芯片的信道号.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_CHAN_READ_FALL    - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
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
 * @brief       设置/获取射频芯片的信道间隔.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_CHANSPC_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t chanspc_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_CHANSPC_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        chanspc_temp[2];
  
  if(msg_id_0 == 0x00)
  {
    SMPL_GET_RADIO_PROPERTY(PROP_FREQ_CONTROL_GROUP, 2, PROP_FREQ_CONTROL_CHANNEL_STEP_SIZE_1, chanspc_temp); 
    
    send_reply_msg(in_frame, chanspc_temp, 2);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    chanspc_temp[0] = in_frame->data[4];
    chanspc_temp[1] = in_frame->data[5];
    
    SMPL_SET_RADIO_PROPERTY(PROP_FREQ_CONTROL_GROUP, 2, PROP_FREQ_CONTROL_CHANNEL_STEP_SIZE_1, chanspc_temp);
    
    rc= APP_NET_PARA_SUCCESS;
  } 
  return rc;
}

/**************************************************************************************************
 * @fn          rate_proc
 *
 * @brief       设置/获取射频芯片的数据速率.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_RATE_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t rate_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_RATE_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        rate_temp[7];
  
  if(msg_id_0 == 0x00)
  {
    SMPL_GET_RADIO_PROPERTY(PROP_MODEM_GROUP, 7, PROP_MODEM_DATA_RATE_2, rate_temp);
    
    send_reply_msg(in_frame, rate_temp, 7);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    rate_temp[0] = in_frame->data[4];
    rate_temp[1] = in_frame->data[5];
    rate_temp[2] = in_frame->data[6];
    rate_temp[3] = in_frame->data[7];
    rate_temp[4] = in_frame->data[8];
    rate_temp[5] = in_frame->data[9];
    rate_temp[6] = in_frame->data[10];
    
    
    SMPL_SET_RADIO_PROPERTY(PROP_MODEM_GROUP, 7, PROP_MODEM_DATA_RATE_2, rate_temp); 
    
    rc= APP_NET_PARA_SUCCESS;
  }

  return rc;
}

/**************************************************************************************************
 * @fn          rxbw_proc
 *
 * @brief       设置/获取射频芯片的接收滤波器带宽.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_RXBW_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
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
    
    /* 未实现此功能 */
    //rc = APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    /* 未实现此功能 */
    
    //rc = APP_NET_PARA_SUCCESS;
  } 
  
  return rc;
}

/**************************************************************************************************
 * @fn          manchter_proc
 *
 * @brief       设置/获取射频芯片的曼彻斯特编码选项.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_MANCHTER_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t manchter_proc(const app_frame_t *in_frame)
{
  app_net_para_t  rc = APP_MANCHTER_READ_FALL;
  uint8_t 	      msg_id_0 = in_frame->data[3];
  uint8_t         manchter_temp[1];
  
  if(msg_id_0 == 0x00)
  {
    SMPL_GET_RADIO_PROPERTY(PROP_PREAMBLE_GROUP, 1, PROP_PREAMBLE_CONFIG, manchter_temp);
    
    send_reply_msg(in_frame, manchter_temp, 1);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    manchter_temp[0] = in_frame->data[4];
    
    SMPL_SET_RADIO_PROPERTY(PROP_PREAMBLE_GROUP, 1, PROP_PREAMBLE_CONFIG, manchter_temp);
    
    rc= APP_NET_PARA_SUCCESS;
  }

  return rc;
}

/**************************************************************************************************
 * @fn          deviatn_proc
 *
 * @brief       设置/获取射频芯片的调制器频偏.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_DEVIATN_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t deviatn_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc= APP_DEVIATN_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        deviatn_temp[3];
  
  if(msg_id_0 == 0x00)
  {
    SMPL_GET_RADIO_PROPERTY(PROP_MODEM_GROUP, 3, PROP_MODEM_FREQ_DEV_2, deviatn_temp);
    
    send_reply_msg(in_frame, deviatn_temp, 3);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    deviatn_temp[0] = in_frame->data[4];
    deviatn_temp[1] = in_frame->data[5];
    deviatn_temp[2] = in_frame->data[6];
    
    SMPL_SET_RADIO_PROPERTY(PROP_MODEM_GROUP, 3, PROP_MODEM_FREQ_DEV_2, deviatn_temp);
    
    rc= APP_NET_PARA_SUCCESS;
  } 

  return rc;
}

/**************************************************************************************************
 * @fn          modformat_proc
 *
 * @brief       设置/获取射频芯片的调制方式.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_MODFORMAT_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t modformat_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_MODFORMAT_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        modformat_temp[7];
  
  if(msg_id_0 == 0x00)
  {
    SMPL_GET_RADIO_PROPERTY(PROP_MODEM_GROUP, 6, PROP_MODEM_MOD_TYPE, modformat_temp);

    send_reply_msg(in_frame, modformat_temp, 6);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    modformat_temp[0] = in_frame->data[4];
    modformat_temp[1] = in_frame->data[5];
    modformat_temp[2] = in_frame->data[6];
    modformat_temp[3] = in_frame->data[7];
    modformat_temp[4] = in_frame->data[8];
    modformat_temp[5] = in_frame->data[9];
    modformat_temp[6] = in_frame->data[10];
    
    SMPL_SET_RADIO_PROPERTY(PROP_MODEM_GROUP, 7, PROP_MODEM_MOD_TYPE, modformat_temp);
    
    rc= APP_NET_PARA_SUCCESS;
   }

  return rc;
}

/**************************************************************************************************
 * @fn          whiting_proc
 *
 * @brief       设置/获取射频芯片的数据白化选项.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_WHITING_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t whiting_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_WHITING_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        whiting_temp[7];
 
  if(msg_id_0 == 0x00)
  {
    SMPL_GET_RADIO_PROPERTY(PROP_PKT_GROUP, 2, PROP_PKT_WHT_POLY_15_8, whiting_temp); 
    
    send_reply_msg(in_frame, whiting_temp, 2);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    whiting_temp[0] = in_frame->data[4];
    whiting_temp[1] = in_frame->data[5];
    whiting_temp[2] = in_frame->data[6];
    whiting_temp[3] = in_frame->data[7];
    whiting_temp[4] = in_frame->data[8];
    whiting_temp[5] = in_frame->data[9];
    whiting_temp[6] = in_frame->data[10];
    
    
    SMPL_SET_RADIO_PROPERTY(PROP_PKT_GROUP, 2, PROP_PKT_WHT_POLY_15_8, whiting_temp);
    SMPL_SET_RADIO_PROPERTY(PROP_PKT_GROUP, 5, PROP_PKT_FIELD_1_CONFIG, &whiting_temp[2]);
    
    rc= APP_NET_PARA_SUCCESS;
   }  

  return rc;
}

/**************************************************************************************************
 * @fn          whiting_proc
 *
 * @brief       设置/获取射频芯片的信号发射功率.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_WHITING_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t txpw_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_TXPW_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        txpw_temp[1];
  
  if(msg_id_0 == 0x00)
  {
    SMPL_GET_RADIO_PROPERTY(PROP_PA_GROUP, 1, PROP_PA_PWR_LVL, txpw_temp); 
    
    send_reply_msg(in_frame, txpw_temp, 1);
    
    rc= APP_NET_PARA_SUCCESS;	
  }
  else if(msg_id_0 == 0x01)
  {
    txpw_temp[0] = in_frame->data[4];
    
    SMPL_SET_RADIO_PROPERTY(PROP_PA_GROUP, 1, PROP_PA_PWR_LVL, txpw_temp);
    
    rc= APP_NET_PARA_SUCCESS;
  } 

  return rc; 
}

/**************************************************************************************************
 * @fn          read_partnum
 *
 * @brief       获取射频芯片的部件号. (e.g., si4438 will return - 0x4438). 
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_PARTNUM_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t read_partnum(const app_frame_t *in_frame)
{
  app_net_para_t  rc = APP_PARTNUM_READ_FALL;
  uint8_t 	      msg_id_0 = in_frame->data[3];
  uint8_t         partnum_temp[2]; 
  
  if(msg_id_0 == 0x00)
  {
    uint8_t part_info[8];
    
    SMPL_GET_RADIO_PART_INFO(part_info);
    
    partnum_temp[0] = part_info[1]; //PART[15:8]
    partnum_temp[1] = part_info[2]; //PART[7:0]
    
    send_reply_msg(in_frame, partnum_temp, 2);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  
  return rc; 
}

/**************************************************************************************************
 * @fn          read_version
 *
 * @brief       获取射频芯片的版本号.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_VERSION_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t read_version(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_VERSION_READ_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        version_temp[1]; 
  
  if(msg_id_0 == 0x00)
  {
    uint8_t part_info[8];
    
    SMPL_GET_RADIO_PART_INFO(part_info);
    
    version_temp[0] = part_info[0]; //CHIPREV
    
    send_reply_msg(in_frame, version_temp, 1);
    
    rc= APP_NET_PARA_SUCCESS;
  }
  
  return rc; 
}

/**************************************************************************************************
 * @fn          read_lqi
 *
 * @brief       获取射频芯片的LQI.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_LQI_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t read_lqi(const app_frame_t *in_frame)
{
  app_net_para_t  rc = APP_LQI_READ_FALL;
  uint8_t 	      msg_id_0 = in_frame->data[3];
  uint8_t         readlqi_temp[1];
  extern linkID_t sLinkID1;      //连接标示符
  
  if(msg_id_0 == 0x00)
  { 
    ioctlRadioSiginfo_t sample;  //用于保存LQI样本的变量
    sample.lid = sLinkID1;       //查询与AP通信的连接
    
    if(SMPL_SUCCESS==SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SIGINFO, &sample))
    {
      readlqi_temp[0] = sample.sigInfo.lqi;
      
      send_reply_msg(in_frame, readlqi_temp,1);
      
      rc= APP_NET_PARA_SUCCESS;
    }
  }
  
  return rc; 
}

/**************************************************************************************************
 * @fn          read_rssi
 *
 * @brief       获取射频芯片的RSSI.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_RSSI_READ_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t read_rssi(const app_frame_t *in_frame)
{
  app_net_para_t  rc = APP_RSSI_READ_FALL;
  uint8_t 	      msg_id_0 = in_frame->data[3];
  uint8_t         readrssi_temp[1];
  extern linkID_t sLinkID1;      //连接标示符
  
  if(msg_id_0 == 0x00)
  {
    ioctlRadioSiginfo_t sample; //用于保存RSSI样本的变量
    sample.lid = sLinkID1;      //查询与AP通信的连接
    
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
 * @fn          fec_proc
 *
 * @brief       设置/获取射频芯片的FEC选项.
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_FEC_PROC_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t fec_proc(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_FEC_PROC_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  uint8_t        fec_temp[1];
  
  if(msg_id_0 == 0x00)
  {
    send_reply_msg(in_frame, fec_temp, 1);
    
    /* Si4438无此功能 */
    //rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {
    fec_temp[1] = in_frame->data[4];
    
    /* Si4438无此功能 */
    //rc= APP_NET_PARA_SUCCESS;
  }
  
  return rc;  
}

/**************************************************************************************************
 * @fn          set_my_device_address
 *
 * @brief       设置本机的物理地址.(注意：此接口仅内部使用)
 *
 * @param       in_frame  - 输入的帧结构体
 *
 * @return      APP_SET_ADDRESS_FALL - 处理射频参数失败
 *              APP_NET_PARA_SUCCESS  - 处理射频参数成功
 **************************************************************************************************
 */
app_net_para_t set_my_device_address(const app_frame_t *in_frame)
{
  app_net_para_t rc = APP_SET_ADDRESS_FALL;
  uint8_t 	     msg_id_0 = in_frame->data[3];
  extern uint8_t delete_ed_connect_flag;  //ED断开连接标志,此标志不为0时ED将断开与AP的连接
  
   if(msg_id_0 == 0x01)
   {
     /* 检测数据域长度，本机地址的第1字节不能为0x00或0xff */
     if((in_frame->data_len == 8) && (in_frame->data[4]!= 0x00) && (in_frame->data[4]!= 0xff))
     {  
       uint8_t myDeviceAddr[4];
       myDeviceAddr[0] = in_frame->data[4];
       myDeviceAddr[1] = in_frame->data[5];
       myDeviceAddr[2] = in_frame->data[6];
       myDeviceAddr[3] = in_frame->data[7];
      
       WriteMacAddressToFlash(myDeviceAddr);//更新FLASH中的本机地址
       
       send_reply_msg(in_frame, myDeviceAddr,4);//构建返回帧
        
       delete_ed_connect_flag = TRUE;//节点准备重置
        
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
       
       WriteModuleInfoToFlash(moudleInfo);//更新FLASH中的本机地址
       
       
       send_reply_msg(in_frame, moudleInfo,6);//构建返回帧
       
       delete_ed_connect_flag = TRUE;//节点准备重置
        
       rc= APP_NET_PARA_SUCCESS;      
     }
  } 
  
  return rc;
}

/**************************************************************************************************
 */
