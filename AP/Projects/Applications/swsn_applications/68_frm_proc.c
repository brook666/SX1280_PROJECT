/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   68_frm_proc.c
// Description: User APP.
//              SWSN网络帧(0x68帧)初步处理源文件.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-21
// History:     2014-10-21  Leidi 重写原有68帧处理的全部函数以支持节点发送缓冲区机制.
//              2014-10-31  Leidi 添加函数send_bcast_frm(),用于发送广播帧并返回应答帧.各68帧处理函数均调用此函数.
//                                使用宏定义MISSES_IN_A_ROW表示向ED发送非广播帧的最大尝试次数.
//                                重写函数send_bcast_pkg(),使用广播LID(SMPL_LINKID_USER_UUD),包含头文件nwk.h.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <string.h>
#include <math.h>
#include "bsp.h"
#include "nwk_layer.h"
#include "bui_pkg_fun.h"
#include "net_para_fun.h"
#include "net_status_fun.h "
#include "user_def_fun.h"
#include "68_frm_proc.h"     
#include "gtway_frm_proc.h"
#include "net_frm_proc.h"
#include "ed_tx_rx_buffer.h"
#include "sx1276-LoRa.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MISSES_IN_A_ROW 1        //向ED发送非广播帧的最大尝试次数
#define MISSES_BCAST_IN_A_ROW 3  //向ED发送广播帧的最大尝试次数

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
 
 extern uint8_t gb_SF;
 extern uint8_t  gb_BW;
 extern uint8_t  CR;	//LR_RegModemConfig1 
 extern volatile net_frm_back_t net_frm_back ;
/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void proc_68_data_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len);
static void proc_68_command_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len);
static void proc_68_ack_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len);
static void proc_68_alarm_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len);
static void send_bcast_frm(const uint8_t *pIn_msg, uint8_t in_msg_len, uint8_t *pOut_msg, uint8_t *pOut_msg_len);
static uint32_t calculate_wait_time(uint8_t payoad_len);

/**************************************************************************************************
 * @fn          proc_68_frm
 *
 * @brief       对从串口输入的网外通信帧(0x69帧)中的SWSN网络帧(0x68帧)根据帧类型进行分类.
 *
 * @param       pIn_gtway_pkg - 输入的网外通信帧(0x69帧)结构体
 *              pOut_msg      - 输出的应答帧字节数组
 *              pOut_len      - 输出的字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t frame_type = GET_FRAME_TYPE(pIn_gtway_pkg->msg);//帧类型
  
  if(frame_type == 0x00)//数据帧
  {
    proc_68_data_frm(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
  else if(frame_type == 0x01)//命令帧
  {
    proc_68_command_frm(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
  else if(frame_type == 0x02) //应答帧
  {
    proc_68_ack_frm(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
  else if(frame_type == 0x03) //告警帧
  {
    proc_68_alarm_frm(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
  return;
}


/**************************************************************************************************
 * @fn          proc_68_data_frm
 *
 * @brief       处理串口输入的网外通信帧(0x69帧)中的SWSN网络帧(0x68帧)中的【数据帧】.
 *
 * @param       pIn_gtway_pkg - 输入的网外通信帧(0x69帧)结构体
 *              pOut_msg      - 输出的应答帧字节数组
 *              pOut_len      - 输出的字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_data_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t   comm_type = GET_COMM_TYPE(pIn_gtway_pkg->msg);//通信类型
  uint16_t  ed_ip     = GET_ED_IP(pIn_gtway_pkg->msg);//获取节点地址

  if (comm_type == 0x00 || comm_type == 0x04) //透传或不透传
  {
    if (ed_ip == 0x0000 || ed_ip == 0xffff) //广播地址
    {
      send_bcast_frm(pIn_gtway_pkg->msg, pIn_gtway_pkg->data_len, pOut_msg, pOut_len);
    }
    else  //单播地址
    {
      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    }
  }
  else if (comm_type == 0x08) //访问集中器
  {
    /* code */
  }
  else if (comm_type == 0x0C) //数据速率测试
  {
    Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
}

/**************************************************************************************************
 * @fn          proc_68_command_frm
 *
 * @brief       处理串口输入的网外通信帧(0x69帧)中的SWSN网络帧(0x68帧)中的【命令帧】.
 *
 * @param       pIn_gtway_pkg - 输入的网外通信帧(0x69帧)结构体
 *              pOut_msg      - 输出的应答帧字节数组
 *              pOut_len      - 输出的字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_command_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t   comm_type       = GET_COMM_TYPE(pIn_gtway_pkg->msg);//获取命令类型
  uint8_t   command_id_fun  = GET_COMMANA_ID_DOMAIN(pIn_gtway_pkg->msg);//命令标识
  uint16_t  ed_ip           = GET_ED_IP(pIn_gtway_pkg->msg);//节点地址
  app_pkg_t in_pkg;

  if (comm_type == 0x00 || comm_type == 0x04) //透传或不透传
  {
    if (ed_ip == 0x0000 || ed_ip == 0xffff) //广播地址
    {
      send_bcast_frm(pIn_gtway_pkg->msg, pIn_gtway_pkg->data_len, pOut_msg, pOut_len);
    }
    else  //单播地址
    {
      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    }
  }
  else if (comm_type == 0x08) //访问集中器
  {
    bui_app_pkg((uint8_t *)pIn_gtway_pkg->msg, &in_pkg);

    if (command_id_fun == 0x01) //网络参数命令帧
    {
      net_para_proc(&in_pkg, pOut_msg, pOut_len);
    }
    else if (command_id_fun == 0x02)  //网络状态命令帧
    {
      net_status_proc(&in_pkg, pOut_msg, pOut_len);
    }
    else if(command_id_fun == 0x80)
    {
      user_def_proc(&in_pkg, pOut_msg, pOut_len);
    }
    else
    {
      /* code */
    }
  }
  else
  {
  }
}

/**************************************************************************************************
 * @fn          proc_68_ack_frm
 *
 * @brief       处理串口输入的网外通信帧(0x69帧)中的SWSN网络帧(0x68帧)中的【应答帧】.
 *
 * @param       pIn_gtway_pkg - 输入的网外通信帧(0x69帧)结构体
 *              pOut_msg      - 输出的应答帧字节数组
 *              pOut_len      - 输出的字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_ack_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t   comm_type = GET_COMM_TYPE(pIn_gtway_pkg->msg);
  uint16_t  ed_ip     = GET_ED_IP(pIn_gtway_pkg->msg);

  if (comm_type == 0x00 || comm_type == 0x04) //透传或不透传
  {
    if (ed_ip == 0x0000 || ed_ip == 0xffff) //广播地址
    {
      send_bcast_frm(pIn_gtway_pkg->msg, pIn_gtway_pkg->data_len, pOut_msg, pOut_len);
    }
    else  //单播地址
    {
      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    }
  }
  else if (comm_type == 0x08) //访问集中器
  {
    /* code */
  }
  else if (comm_type == 0x0C) //用户登录相关
  {
    /* code */
  }
}

/**************************************************************************************************
 * @fn          proc_68_alarm_frm
 *
 * @brief       处理串口输入的网外通信帧(0x69帧)中的SWSN网络帧(0x68帧)中的【告警帧】.
 *
 * @param       pIn_gtway_pkg - 输入的网外通信帧(0x69帧)结构体
 *              pOut_msg      - 输出的应答帧字节数组
 *              pOut_len      - 输出的字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_alarm_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t   comm_type     = GET_COMM_TYPE(pIn_gtway_pkg->msg);
  uint16_t  ed_ip         = GET_ED_IP(pIn_gtway_pkg->msg);


  if (comm_type == 0x00 || comm_type == 0x04) //透传或不透传
  {
    if (ed_ip == 0x0000 || ed_ip == 0xffff) //广播地址
    {
      send_bcast_frm(pIn_gtway_pkg->msg, pIn_gtway_pkg->data_len, pOut_msg, pOut_len);
    }
    else  //单播地址
    {
      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    } 
  }
  else if (comm_type == 0x08) //访问集中器
  {
    /* code */
  }
  else 
  {
    /* code */
  }
}

/**************************************************************************************************
 * @fn          send_bcast_frm
 *
 * @brief       处理SWSN广播帧,并根据需要应答标志位返回应答帧.
 *
 * @param       pIn_msg       - 输入的SWSN广播帧字节数组
 *              in_msg_len    - 输入的字节数组长度
 *              pOut_msg      - 输出的应答帧字节数组
 *              pOut_msg_len  - 输出的字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void send_bcast_frm(const uint8_t *pIn_msg, uint8_t in_msg_len, uint8_t *pOut_msg, uint8_t *pOut_msg_len)
{
  uint8_t       reply_request_flag = GET_REPLY_REQUEST_FLAG(pIn_msg);
  uint8_t       send_result;
  app_pkg_t     in_pkg;
  uint8_t bcast_send_index = 0;
  
  //由主控设备(手机客户端、服务器等)要求的广播数据，发送两次
  for(bcast_send_index = 0;bcast_send_index<2;bcast_send_index++)
  {
    /* 直接发送.发送前不用清除应答标志位.ED不会对广播帧回复应答帧 */
    send_result = send_bcast_pkg((uint8_t *)pIn_msg, in_msg_len);
    SWSN_DELAY(20);
  }
  
  /* 需要应答且发送成功,由网关回复应答帧 */
  if ((reply_request_flag == 0x40) && (send_result == 1))
  {
    bui_app_pkg((uint8_t *)pIn_msg, &in_pkg);
    *pOut_msg_len = bui_pkg_ack(&in_pkg, pOut_msg);
  }
}


/**************************************************************************************************
 * @fn          send_net_pkg
 *
 * @brief       用指定的连接标识符(LID)向ED发送数据.最大尝试次数由MISSES_IN_A_ROW定义.
 *
 * @param       lid     - 连接标识符(LID)
 *              string  - 输入的字节数组
 *              tx_len  - 字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
uint8_t send_net_pkg(linkID_t lid, uint8_t *string, uint8_t tx_len)
{
  uint8_t       misses;
  uint8_t       rc;
	uint8_t       send_frm_num;
	uint8_t       wait_time;
  uint16_t i = 0 ;
#ifdef DEBUG  
  put_string_uart1("Sending msg to ED...\n");
#endif
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"Sending msg to ED..."RTT_CTRL_RESET"\n");
#endif

   
  /* Try sending message MISSES_IN_A_ROW times looking for ack */
  for (misses = 0; misses < MISSES_IN_A_ROW; misses ++)
  {
#ifdef DEBUG  
    put_string_uart1("Trying send ");
    put_multi_char_uart1(&misses, 1);
    put_string_uart1(" times...");
#endif
		
    send_frm_num=string[6];
		
    rc = send_user_msg(lid, string, tx_len);
    if (1 == rc)
    {
#ifdef DEBUG
      put_string_uart1("Send success!\n");
      put_multi_char_uart1(string, tx_len);
#endif
			wait_time=calculate_wait_time(tx_len+15);    //
			
      for(i = 0; i < wait_time; i++)
      {
        SWSN_DELAY(1);   
        if(((net_frm_back.lid == lid) && (net_frm_back.isBack == 1)&& (net_frm_back.frm_count == send_frm_num)) || ((string[1]&0x0F)==0x00) )
        {
          memset((void*)&net_frm_back,0x0,sizeof(net_frm_back));
#ifdef IWDG_START 
          IWDG_ReloadCounter();  
#endif
					//SWSN_DELAY(10); 
          return rc;
        }
      } 
			
			rc=0;
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"WRITE FAILED BUFFER..."RTT_CTRL_RESET"\n");
#endif
      //Write_tx_fail_ed_buffer(lid, string, &tx_len); 
    }
    else 
    {
    }
  }
  
  if(rc !=1)  
  {
//    Write_tx_fail_ed_buffer(lid, string, &tx_len);  
  }
	//SWSN_DELAY(10); 
#ifdef IWDG_START 
   IWDG_ReloadCounter();  
#endif  
  return rc;           
}

/**************************************************************************************************
 * @fn          send_bcast_pkg
 *
 * @brief       向ED发送广播数据.
 *
 * @param       string  - 输入的字节数组
 *              len     - 字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
uint8_t send_bcast_pkg(uint8_t *string, uint8_t len)
{
  uint8_t  misses;
  uint8_t  rc;
  
#ifdef DEBUG 
  put_string_uart1("Sending Bcast msg...\n");
#endif 
  /* get radio ready...awakens in idle state */
//  SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
  /* Try sending message MISSES_IN_A_ROW times looking for ack */
  
  //调整为只发送一次，如果失败，则在另外的队列里面选择发送
  for (misses = 0; misses < MISSES_BCAST_IN_A_ROW; misses ++)
  {    
    rc = send_bcast_msg(string, len);
    if(1 == rc)
    {
#ifdef DEBUG 
      put_string_uart1("Send Bcast success!\n");
      put_multi_char_uart1(string, len);
#endif 
      break;
    }
    else 
    {
#ifdef DEBUG 
      put_string_uart1("Send Bcast failed!\n");
#endif 
    }
  }
  
  return rc;
}


/**************************************************************************************************
 * @fn          calculate_wait_time
 *
 * @brief       计算发送完成后的等待时间
 *
 * @param       payload_len     - 待发送帧长度
 *
 * @return      等待时间
 **************************************************************************************************
 */
uint32_t calculate_wait_time(uint8_t payoad_len)
{
  float sf,bw;
	float t_premable,t_payload,t_header,total_time;
	float premable_len;
	float bw_frm[10]={7.1,10.4,15.6,20.8,31.2,41.7,62.5,125,250,500};
	float sf_frm[7]={6,7,8,9,10,11,12,};
  float header_len=5;

	sf=sf_frm[gb_SF];
	bw=bw_frm[gb_BW];
	
  premable_len=((SPIRead((u8)(LR_RegPreambleMsb>>8)))<<8)|(SPIRead((u8)(LR_RegPreambleLsb>>8)))+4;  
	t_premable=(premable_len*pow(2,sf)/bw);
	t_payload=(payoad_len*1000)/((sf*bw*1000*4/(4+CR))/(pow(2,sf)*8));
	t_header=(header_len*1000)/((sf*bw*1000/2)/(pow(2,sf)*8));
	
	total_time=(uint32_t)(t_premable+t_payload+t_header);
	
	return total_time+7;
}

/**************************************************************************************************
 */



