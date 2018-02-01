/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   uart_frm_proc.c
// Description: User APP.
//              处理从串口输入的帧.
// Author:      Leidi
// Version:     1.0
// Date:        2012-3-24
// History:     2014-9-29   Leidi 修改函数uart_frm_proc(),去除与数据帧无关的语句.
//              2014-9-30   Leidi 修改函数proc_command_frm(),使用类型app_frame_t.
//                                添加头文件user_def_fun.h.
//                                使用宏定义MISSES_IN_A_ROW表示向AP发送的最大尝试次数.
//                                清理proc_alarm_frm(),proc_net_ack_frm(),send_net_pkg(),send_bcast_pkg()的代码.
//                                清理无用的全局变量send_status,data_frm_cnt,data_buffer[100],data_cnt.
//                                清理无用的头文件.
//                                将私有的函数原型从.h文件中移入本文件并用static修饰.
//                                引入外全局变量last_tx_frm_num,局变量proc_times_for_uart_frm_rbuf1/2,
//                                以便在调用函数bui_data_msg()前更新发送帧计数器.
//              2014-10-28  Leidi 修改函数SMPL_SendOpt()的参数为SMPL_TXOPTION_ACKREQ.
//                                如果没有定义宏NEED_REPLY_FRAME,proc_data_frm()会根据发送成功与否设置ERR引脚电平.
//              2014-11-10  Leidi 移除头文件stm8s_uart.h.
//                                添加用于配置的头文件user_app_config.h.
//              2015-07-29  Leidi 删除引用头文件nwk_globals.h,nwk_types.h.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "uart_frm_proc.h"
#include "net_para_fun.h"
#include "net_status_fun.h"
#include "nwk_api.h"
#include "nwk.h"
#include "nwk_types.h"
#include "bsp.h"
#include <string.h>

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MISSES_IN_A_ROW 3  //向AP发送的最大尝试次数


/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern linkID_t sLinkID1;

extern uint8_t last_tx_frm_num;//上一次发送的帧号

uint8_t proc_times_for_uart_frm_rbuf1;//同一串口缓存1被处理的次数
uint8_t proc_times_for_uart_frm_rbuf2;//同一串口缓存2被处理的次数

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void proc_data_frm(uint8_t *,uint8_t);
//static void proc_command_frm( uint8_t *,uint8_t); //本应用中暂未使用
//static void proc_alarm_frm( uint8_t *,uint8_t);   //本应用中暂未使用
//static void proc_net_ack_frm(uint8_t *,uint8_t);  //本应用中暂未使用


/**************************************************************************************************
 * @fn          uart_frm_proc
 *
 * @brief       处理从串口输入的数据.
 *
 * @param       str - 串口输入的字节数组
 *              len - 字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void uart_frm_proc(uint8_t *str, uint8_t len)
{
  /* 透传模块将串口输入的数据全当<数据帧>处理 */
  proc_data_frm(str, len);
  
  return;    
}


/**************************************************************************************************
 * @fn          proc_data_frm
 *
 * @brief       处理从串口输入的数据并形成<数据帧>发送.
 *
 * @param       str - 串口输入的字节数组
 *              len - 字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_data_frm(uint8_t* str, uint8_t len)
{
  uint8_t msg[MAX_APP_PAYLOAD];
  uint8_t msg_len;

  /* 检测是否为非重传帧,只有非重传帧的帧号才递增 */
  if((proc_times_for_uart_frm_rbuf1 == 0)
   &&(proc_times_for_uart_frm_rbuf2 == 0))
  {
    last_tx_frm_num++;//上一次使用的帧号加1后为当前帧号
  }
  
  /* 将串口输入数据构建成<数据帧>的负载 */
  msg_len = bui_data_msg(str, len, msg);
  
  /* 向AP 发送数据帧  */
  if(SMPL_SUCCESS == send_net_pkg(sLinkID1, msg, msg_len))
  {
    PRINT_DEBUG_MSG("Send data frame to AP success!\n");
  }
  else
  {
    PRINT_DEBUG_MSG("Send data frame to AP failed!\n");
  }
}


/**************************************************************************************************
 * @fn          proc_command_frm
 *
 * @brief       处理从串口输入的<命令帧>.
 *
 * @param       str - 串口输入的字节数组
 *              len - 字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_command_frm(uint8_t* str, uint8_t len)
{
  app_frame_t   in_frame;
  comm_type_t   comm_type;

  //uint8_t alarm_info[4];
  
  if(APP_SUCCESS == bui_app_frame(str, len, &in_frame))
  {
    comm_type   = (comm_type_t)in_frame.ctr_code.comm_type;    //通信方式
    
    if(comm_type == LOCAL_COMM)//访问本节点
    {
      uint8_t appIndex = 0;
      for(appIndex=0;appIndex<MAX_APP_FUNC;appIndex++)
      {
        if(in_frame.data[1] == appFuncDomain[appIndex].commandId_DI2)
        {
          appFuncDomain[appIndex].pFunx(&in_frame);
          break;
        }
      }
      if(appIndex == MAX_APP_FUNC)
      {
        //alarm_info[0]=0x00;
        //alarm_info[1]=0x01;
        //alarm_info[0]=0x01;
        //alarm_info[1]=0x08;
       // out_msg_len = bui_alarm_msg(&in_frame, alarm_info, 4,out_msg);
      }

      PRINT_DEBUG_MSG("Order frame execute result:");
      PRINT_DEBUG_BYTES(out_msg, out_msg_len);
      
      //SEND_BYTES_TO_UART(out_msg, out_msg_len);
    }
    else if((comm_type == NO_TRANSPARENT_COMM) || (comm_type == TRANSPARENT_COMM))//访问集中器
    {
      /* 向AP发送命令帧 */
      if(SMPL_SUCCESS == send_net_pkg(sLinkID1, str, len))
      {
        PRINT_DEBUG_MSG("Send order frame to AP success!\n");
      }
      else
      {
        PRINT_DEBUG_MSG("Send order frame to AP failed!\n");
      }
    }
  }
  else //非法帧:帧格式不正确
  {
    PRINT_DEBUG_MSG("Recv illegal msg:");
    PRINT_DEBUG_BYTES(str, len);
  }
}


/**************************************************************************************************
 * @fn          proc_alarm_frm
 *
 * @brief       处理从串口输入的<告警帧>并向AP发送.
 *
 * @param       str - 串口输入的字节数组
 *              len - 字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_alarm_frm(uint8_t* str, uint8_t len)
{
  uint8_t out_msg[MAX_APP_PAYLOAD];
  uint8_t out_msg_len;
  uint8_t alarm_info[4];
  
  /* 向AP发送告警帧 */
  if(SMPL_SUCCESS == send_net_pkg(sLinkID1, str, len))
  {
    PRINT_DEBUG_MSG("Send alarm frame to AP success!\n");
  }
  else
  {
    PRINT_DEBUG_MSG("Send alarm frame to AP failed!\n");
    
    alarm_info[0]=0x00;
    alarm_info[1]=0x01;
    alarm_info[2]=0x01;
    alarm_info[3]=0x08;
    out_msg_len = bui_alarm_msg(NULL, alarm_info, 4, out_msg);
    
    SEND_BYTES_TO_UART(out_msg, out_msg_len);
    
    PRINT_DEBUG_BYTES(out_msg, out_msg_len);
  }
}


/**************************************************************************************************
 * @fn          proc_alarm_frm
 *
 * @brief       处理从串口输入的<应答帧>并向AP发送.
 *
 * @param       str - 串口输入的字节数组
 *              len - 字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_net_ack_frm( uint8_t* str, uint8_t len)
{
  uint8_t out_msg[MAX_APP_PAYLOAD];
  uint8_t out_msg_len;
  uint8_t alarm_info[4];
  
  /* 向AP发送应答帧 */
  if(SMPL_SUCCESS == send_net_pkg(sLinkID1, str, len))
  {
    PRINT_DEBUG_MSG("Send reply frame to AP success!\n");
  }
  else
  {
    PRINT_DEBUG_MSG("Send reply frame to AP failed!\n");
    
    alarm_info[0]=0x00;
    alarm_info[1]=0x01;
    alarm_info[2]=0x01;
    alarm_info[3]=0x08;
    out_msg_len = bui_alarm_msg(NULL, alarm_info, 2, out_msg);
    
    SEND_BYTES_TO_UART(out_msg, out_msg_len);
    
    PRINT_DEBUG_BYTES(out_msg, out_msg_len);
  }
}


/**************************************************************************************************
 * @fn          send_net_pkg
 *
 * @brief       用指定的连接标识符(LID)向AP发送数据.最大尝试次数由MISSES_IN_A_ROW定义.
 *
 * @param       lid     - 连接标识符(LID)
 *              string  - 输入的字节数组
 *              tx_len  - 字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
smplStatus_t send_net_pkg(linkID_t lid, uint8_t *string, uint8_t tx_len)
{
  uint8_t     misses;
  smplStatus_t rc;

  PRINT_DEBUG_MSG("Sending msg to AP...\n");
  
  /* get radio ready...awakens in idle state */
  SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
  
  /* Try sending message MISSES_IN_A_ROW times looking for ack */
  for (misses = 0; misses < MISSES_IN_A_ROW; misses++)
  {
    PRINT_DEBUG_MSG("Trying send ");
    PRINT_DEBUG_BYTES(&misses, 1);
    PRINT_DEBUG_MSG(" times...");
    
    rc = SMPL_SendOpt(lid, string, tx_len, SMPL_TXOPTION_ACKREQ);//SMPL_TXOPTION_NONE///
    
    if(SMPL_SUCCESS == rc)
    {
      PRINT_DEBUG_MSG("Send success!\n");
      
      PRINT_DEBUG_BYTES(string, tx_len);
      
      break;
    }
    else 
    {
     // NWK_DELAY(5);
      PRINT_DEBUG_MSG("Send failed!\n");
    }
  }
  return rc;       
}


/**************************************************************************************************
 * @fn          send_bcast_pkg
 *
 * @brief       发送广播数据.
 *
 * @param       string  - 输入的字节数组
 *              len     - 字节数组长度
 *
 * @return      none
 **************************************************************************************************
 */
smplStatus_t send_bcast_pkg(uint8_t *string, uint8_t len)
{
  smplStatus_t rc;
  
  PRINT_DEBUG_MSG("Sending Bcast msg...\n");
  
  rc = SMPL_SendOpt(SMPL_LINKID_USER_UUD, string, len, SMPL_TXOPTION_NONE);
  if(SMPL_SUCCESS == rc)
  {
    PRINT_DEBUG_MSG("Send Bcast success!\n");
    
    PRINT_DEBUG_BYTES(string, len);
  }
  else 
  {
    PRINT_DEBUG_MSG("Send Bcast failed!\n");
  }
  
  return rc;
}


/**************************************************************************************************
 */