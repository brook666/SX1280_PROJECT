/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   67_frm_proc.c
// Description: User APP.
//              内部组播帧(0x67帧)处理源文件.
// Author:      Leidi
// Version:     1.0
// Date:        2014-11-1
// History:     2014-11-1   Leidi 初始版本建立.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <string.h>
#include "gtway_frm_proc.h"
#include "bui_pkg_fun.h"
#include "ed_tx_rx_buffer.h"
#include "67_frm_proc.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
group_buffer_t   Ed_group_buffer[IP_GROUP_MAX_NUM] = {0};
/* 组播帧缓冲区使用量 */
static uint8_t group_buffer_num = 0 ;


/**************************************************************************************************
 * @fn          proc_67_frm
 *
 * @brief       处理从串口输入的网外通信帧(0x69帧)中的SWSN网络组播帧(0x67帧).
 *
 * @param       pIn_gtway_pkg - 输入的网外通信帧(0x69帧)结构体
 *              pOut_msg      - 输出的应答帧字节数组
 *              pOut_len      - 输出的字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */    //67frame用于对1个网关下挂多个节点组播
void proc_67_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
	uint8_t mult_ip_count = pIn_gtway_pkg->msg[1];//要组播节点的IP个数
  uint8_t mult_field_len = 2 * mult_ip_count + 2;//IP地址组长度
  uint8_t comm_type = GET_COMM_TYPE((pIn_gtway_pkg->msg + mult_field_len));//获取通信方式
  uint16_t ed_ip    = GET_ED_IP((pIn_gtway_pkg->msg + mult_field_len));//获取节点地址
  
  if (comm_type == 0x00 || comm_type == 0x04) //透传或不透传   //00表示访问网络上的节点数据透传，04表示数据不透传
  {
    if (ed_ip == 0xeeee)  //组播地址
    {
//      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
      write_group_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    }
  }
  if (comm_type == 0x0C) 
  {
    if (ed_ip == 0xeeee)  //组播地址
    {
//      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
      write_group_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    }
  }	
  return;
}

/**************************************************************************************************
 * @fn          Change_67frm_to_68frm
 *
 * @brief       将SWSN网络组播帧(0x67帧)改写为与节点通信使用的0x68帧,并提取0x67帧中的IP地址组.
 *
 * @param       pIn_gtway_pkg - 输入的网外通信帧(0x69帧)结构体
 *              pOut_ip_group - 输出的IP地址组
 *
 * @return      IP地址组中的IP数量
 **************************************************************************************************
 */
uint8_t Change_67frm_to_68frm(gtway_pkg_t *pIn_gtway_pkg, uint16_t *pOut_ip_group)
{
  uint8_t mult_ip_count = pIn_gtway_pkg->msg[1];
  uint8_t mult_field_len = 2 * mult_ip_count + 2;
  uint8_t inner_frm_len  = mult_field_len + pIn_gtway_pkg->msg[mult_field_len + 7] + 10;
  uint8_t i;
  
  /* 获取0x67帧中的所有IP地址 */
  for(i = 0; i < mult_ip_count; i++)
  {
    pOut_ip_group[i] = ((uint16_t)pIn_gtway_pkg->msg[2 + 2 * i] << 8) | pIn_gtway_pkg->msg[3 + 2 * i];
  }
  
  /* 将0x67帧中的0x68帧移动到常规0x68帧的位置 */
  for(i = 0; i < inner_frm_len - mult_field_len; i++)
  {
    pIn_gtway_pkg->msg[i] = pIn_gtway_pkg->msg[mult_field_len + i];
  }
  
  /* 重新设置外部帧中数据域的长度 */
  pIn_gtway_pkg->data_len = inner_frm_len - mult_field_len;
  
  /* 不用管外部帧的校验和,因为不会再用到 */
  
  return mult_ip_count;
}


/**************************************************************************************************
 * @fn          Change_67frm_to_69frm
 *
 * @brief       将SWSN网络组播帧(0x67帧)变为69帧.
 *
 * @param       pIn_msg       - 输入67帧
 *              pIn_gtway_pkg - 输出的网外通信帧(0x69帧)结构体
 *              len           - 输入时为67帧长度，输出实为69帧长度
 *
 * @return      69帧长度
 **************************************************************************************************
 */
uint16_t Change_67frm_to_69frm(uint8_t * pIn_msg,gtway_pkg_t *pOut_gtway_pkg, uint16_t *len)
{

  pOut_gtway_pkg->frm_head = GTWAY_FRAME_HEAD; 
  pOut_gtway_pkg->reserve1 = 0x00;
  pOut_gtway_pkg->ctr_code = 0x00;
  pOut_gtway_pkg->gtway_type = 0x00;
  pOut_gtway_pkg->gtway_addr[0] = 0x00;
  pOut_gtway_pkg->gtway_addr[1] = 0x00;
  pOut_gtway_pkg->gtway_addr[2] = 0x00;
  pOut_gtway_pkg->gtway_addr[3] = 0x00;
  
  pOut_gtway_pkg->server_addr[0] = 0x00;
  pOut_gtway_pkg->server_addr[1] = 0x00;
  pOut_gtway_pkg->server_addr[2] = 0x00;
  pOut_gtway_pkg->server_addr[3] = 0x00;
  
  pOut_gtway_pkg->mobile_addr[0] = 0x00;
  pOut_gtway_pkg->mobile_addr[1] = 0x00;
  pOut_gtway_pkg->mobile_addr[2] = 0x00;
  pOut_gtway_pkg->mobile_addr[3] = 0x00;
  pOut_gtway_pkg->mobile_addr[4] = 0x00;
  pOut_gtway_pkg->mobile_addr[5] = 0x00;
  
  pOut_gtway_pkg->pid_num = 0x00;
  pOut_gtway_pkg->data_len = *len;
  memcpy(pOut_gtway_pkg->msg, pIn_msg,*len);  //Gateway Server IP
  pOut_gtway_pkg->cs = 0x00;
  pOut_gtway_pkg->frm_end = 0x17;
  
  return (*len+26);
}

/**************************************************************************************************
 * @fn          write_group_buffer
 *
 * @brief       将67帧写入组播缓冲区.
 *
 * @param       pIn_gtway_pkg - 接收到的69帧信息
 *              pOut_msg      - 输出的返回帧(如告警帧)
 *              pOut_len      - 输出的返回帧长度
 *
 * @return      none
 **************************************************************************************************
 */
void write_group_buffer(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t* pOut_len)
{
  uint8_t i = 0;
//  uint8_t curOrderStamp = 0 ;
  app_pkg_t         in_pkg;
  uint8_t           alarm_info[10];
//  for(i=0;i<IP_GROUP_MAX_NUM;i++)
//  {
//    if(curOrderStamp < Ed_group_buffer[i].orderStamp)
//      curOrderStamp = Ed_group_buffer[i].orderStamp;
//  }
//  curOrderStamp += 1;
  
  for(i=0;i<IP_GROUP_FRAME_BUFFER;i++)
  {
    if(Ed_group_buffer[i].usage == GROUP_BUFFER_AVALIABLE)
    {
      group_buffer_num++;
      memcpy(Ed_group_buffer[i].msg,pIn_gtway_pkg->msg,pIn_gtway_pkg->data_len);
      Ed_group_buffer[i].orderStamp = group_buffer_num;
      Ed_group_buffer[i].len = pIn_gtway_pkg->data_len;
      Ed_group_buffer[i].usage = GROUP_BUFFER_IN_USE;
#ifdef SEGGER_DEBUG 
        SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"IP GROUP FRAME:"RTT_CTRL_RESET"\n");        
        SEGGER_RTT_put_multi_char(Ed_group_buffer[i].msg,Ed_group_buffer[i].len);
#endif          
      return;
    }
  }
  
  /* 建立告警帧 */
  
  alarm_info[0] = 0x00;
  alarm_info[1] = 0x04;
  alarm_info[2] = 0x03;
  alarm_info[3] = 0x06;

  bui_app_pkg((uint8_t *)pIn_gtway_pkg->msg, &in_pkg);
  *pOut_len = bui_pkg_alarm(&in_pkg, alarm_info, 4, pOut_msg);
  return;
}

/**************************************************************************************************
 * @fn          write_67frm_to_ed_tx_buffer
 *
 * @brief       将67帧写入节点发送缓冲区.
 *
 * @param       空
 *
 * @return      none
 **************************************************************************************************
 */
void write_67frm_to_ed_tx_buffer(void)
{
  uint8_t i=0;
  uint8_t pOutMsg[20];
  uint8_t len = 0;
  
  gtway_pkg_t gtway_pkg = {0};
  
  if(group_buffer_num == 0)
    return;
  if(isReadyToWriteGroupFrm() == 0)
    return ;
  
  for(i=0;i<IP_GROUP_FRAME_BUFFER;i++)
  {
     if((Ed_group_buffer[i].usage == GROUP_BUFFER_IN_USE)&&\
         (Ed_group_buffer[i].orderStamp == 1))
     {
       Change_67frm_to_69frm(Ed_group_buffer[i].msg,&gtway_pkg,&Ed_group_buffer[i].len);
       Write_tx_ed_buffer(&gtway_pkg, pOutMsg, &len);
       Ed_group_buffer[i].orderStamp = 0;
       Ed_group_buffer[i].usage = GROUP_BUFFER_AVALIABLE;
       group_buffer_num--;
       break;
     }
  }
  
  for(i=0;i<IP_GROUP_FRAME_BUFFER;i++)
  {
    if(Ed_group_buffer[i].orderStamp == 0)
    {
      continue;
    }
    else
    {
      Ed_group_buffer[i].orderStamp--;
    }
  }
}
/**************************************************************************************************
 */
