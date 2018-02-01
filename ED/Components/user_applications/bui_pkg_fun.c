/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   bui_pkg_fun.c
// Description: User APP.
//              SWSN网络帧的构建与解析.
// Author:      Leidi
// Version:     2.0
// Date:        2012-3-24
// History:     2014-9-25   Leidi 删除全局变量ApAddr,使用官方函数nwk_getAPAddress()获取AP地址. 
//              2014-9-26   Leidi bui_app_pkg()的输入参数添加const修饰符.
//              2014-9-27   Ledi  新增函数bui_app_frame(),bui_reply_msg(),bui_alarm_msg(),
//                                bui_order_msg()
//              2014-9-28   Leidi 新增函数bui_data_msg().
//              2014-9-29   Leidi 修改函数bui_app_frame()的参数和返回值, 使其能返回解析成功与失败.
//              2014-9-30   Leidi 删除以下函数定义:bui_app_pkg(),bui_rep_pkg_command(),bui_pkg_ack(),
//                                bui_pkg_alarm(),bui_68_frm(),bui_ap_inner_command_pkg().
//                                添加注释.
//                                本文件中的全局变量myIpAddr改为extern,在main_swsn_ed.c中定义.
//                                在函数bui_data_msg()中不更新帧计数器，移到其被调用处进行.同时移除本文件中相关的全局变量.
//              2014-10-8   Leidi 修复BUG: 在函数bui_alarm_msg()中, 应该把告警帧的通信方式设为<透传>.
//              2014-10-28  Leidi 在函数bui_data_msg()中，根据是否定义宏NEED_REPLY_FRAME,设置数据帧的需要应答位.
//              2014-10-29  Leidi 在各打包函数中,将控制码ctr_code中的comm_dir设为ED_TO_AP.
//              2014-11-10  Leidi 移除头文件stm8s_uart.h.
//                                添加用于配置的头文件user_app_config.h.
//              2015-01-07  Leidi 修复BUG:bui_order_msg中应该初始化控制码的传输状态为ONCE_TRANS.
//              2015-07-29  Leidi 删除引用头文件mrfi_spi.h,nwk_types.h.
//                                移除对全局变量myIpAddr的引用.
//                                引用头文件addr_control_fun.h,使用函数Get_IpAddress()来获取IP地址.
//              2015-08-07  Leidi 修改帧格式:集中器地址改为2字节,帧计数器改为1字节.
//              2015-09-02  Zengjia 改变节点IP和集中器IP的获取方式，使用
                                    SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
*****************************************************************************/


/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bui_pkg_fun.h"
#include "nwk_globals.h"
#include "nwk_api.h"
#include "bsp.h"
#include "uart_frm_proc.h"
#include <string.h>
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern linkID_t sLinkID1;
/* ------------------------------------------------------------------------------------------------
 *                                            Local Variables
 * ------------------------------------------------------------------------------------------------
 */



/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */


/**************************************************************************************************
 * @fn          gen_crc
 *
 * @brief       计算位于数组中多个无符号8位二进制数的累加和除以256的余数.
 *
 * @param       frm - 输入的字节数组
 *              len - 要计算的字节数组长度
 *
 * @return      计算结果
 **************************************************************************************************
 */
uint8_t gen_crc(const uint8_t *frm, uint8_t len)
{
  uint8_t crc_sum=0;
  uint8_t i;
  for( i =0; i <len; i++)
  {
      crc_sum = *frm + crc_sum;
      frm++;
  }
  return crc_sum;
}


/**************************************************************************************************
 * @fn          bui_app_frame
 *
 * @brief       将输入的字节数组进行解析并构建为帧结构体以方便获取帧结构的各个部分.
 *
 * @param       in_msg      - 输入的字节数组
 *              in_msg_len  - 输入的字节数组长度
 *              out_frame   - 构建好的帧结构体
 *
 * @return      APP_ERR     - 构建帧结构体失败,输入的字节数组格式不正确
 *              APP_SUCCESS - 构建帧结构体成功
 **************************************************************************************************
 */
app_status_t bui_app_frame(const uint8_t *in_msg, uint8_t in_msg_len, app_frame_t *out_frame)
{
  uint8_t data_len, i;
  app_status_t rc = APP_ERR;

  if(in_msg[0] == APP_FRAME_HEAD && in_msg[in_msg_len-1] == APP_FRAME_TAIL)
  {
    /* 帧起始符 */
    out_frame->frm_head = in_msg[0];
    
    /* 控制码 */
    out_frame->ctr_code.code = in_msg[1];
    
    /* 节点IP地址 */
    out_frame->ed_addr_1 = in_msg[2];
    out_frame->ed_addr_0 = in_msg[3];
    
    /* 集中器地址 */
    out_frame->ap_addr_1 = in_msg[4];
    out_frame->ap_addr_0 = in_msg[5];
    
    /* 帧计数器 */
    out_frame->frm_cnt = in_msg[6];
  
    /*数据域长度*/
    out_frame->data_len = in_msg[7];
    data_len = in_msg[7];
    
    /*数据域*/
    for(i = 0; i < data_len; i++)
    {
      out_frame->data[i] = in_msg[8+i];
    }
    
    /*校验码*/
    out_frame->cs = in_msg[8 + data_len];
    
    /*帧结束符*/
    out_frame->frm_tail = in_msg[9 + data_len];
    
    rc = APP_SUCCESS;
  }
  
  return rc;
}


/**************************************************************************************************
 * @fn          bui_reply_msg
 *
 * @brief       将输入的帧结构体以<应答帧>的形式构建为字节数组，以便于通过网络发送.
 *
 * @param       in_frame    - 输入的帧结构体
 *              out_msg     - 输出的字节数组
 *
 * @return      输出的字节数组长度
 **************************************************************************************************
 */
uint8_t bui_reply_msg(const app_frame_t *in_frame, uint8_t *out_msg)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};
    
  /* 获取本机IP和AP的IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr); 
  
  /* 获取原始控制码 */
  ctr_code.code = in_frame->ctr_code.code;
  
  /* 修改控制码：不需要应答的应答帧 */
  ctr_code.frame_type = REPLY_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_dir = ED_TO_AP;

  /* 填充帧内容 */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = in_frame->frm_cnt;  
  out_msg[7] = 0;//数据域为长度为0
  out_msg[8] = gen_crc(out_msg, 8);//CS校验码
  out_msg[9] = APP_FRAME_TAIL;
  
  /* 返回帧长 */
  return 10;
}


/**************************************************************************************************
 * @fn          bui_alarm_msg
 *
 * @brief       将输入的帧结构体以<告警帧>的形式构建为字节数组，以便于通过网络发送.
 *
 * @param       in_frame    - 输入的帧结构体
 *              err_info    - 输入的告警信息字节数组
 *              err_len     - 告警信息字节数组长度
 *              out_msg     - 输出的字节数组
 *
 * @return      输出的字节数组长度
 **************************************************************************************************
 */
uint8_t bui_alarm_msg(const app_frame_t *in_frame, const uint8_t *err_info, uint8_t err_len, uint8_t *out_msg)
{
  uint8_t i;
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //获得4字节IP地址
  static uint8_t alarm_frm_cnt=0;//发送应答帧的帧计数器
  
  /* 获取本机IP和AP的IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* 获取原始控制码 */
  if(in_frame != NULL)
  {
    ctr_code.code = in_frame->ctr_code.code;
  }
  else
  {
    ctr_code.code = 0x00;
  }
  
  /* 修改控制码：不需要应答的告警帧,透传 */
  ctr_code.frame_type = ALARM_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type = TRANSPARENT_COMM;
  ctr_code.comm_dir = ED_TO_AP;

  /* 填充帧内容 */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  if(in_frame != NULL)
  {
    out_msg[6] = in_frame->frm_cnt;  
  }
  else
  {
    out_msg[6] = alarm_frm_cnt;  
    alarm_frm_cnt++;
  }
  out_msg[7] = err_len;
  for(i=0; i<err_len; i++)
  {
    out_msg[8+i] = err_info[i];
  }
  out_msg[8+err_len] = gen_crc(out_msg, 8+err_len);//CS校验码
  out_msg[9+err_len] = APP_FRAME_TAIL;
  
  /* 返回帧长 */
  return 10+err_len;
}


/**************************************************************************************************
 * @fn          bui_order_msg
 *
 * @brief       将输入的帧结构体以<命令帧>的形式构建为字节数组，以便于通过网络发送.
 *
 * @param       comm_type   - 通信方式:
 *                            TRANSPARENT_COMM    - 透传,发送給集中器后集中器不处理,原样输出
 *                            NO_TRANSPARENT_COMM - 不透传,发送給集中器后集中器内部处理
 *              in_frame    - 输入的帧结构体
 *              reply_para  - 输入的命令返回结果字节数组
 *              para_len    - 命令返回结果字节数组长度
 *              out_msg     - 输出的字节数组
 *
 * @return      输出的字节数组长度
 **************************************************************************************************
 */
uint8_t bui_order_msg(comm_type_t comm_type, const app_frame_t *in_frame, const uint8_t *reply_para, uint8_t para_len, uint8_t *out_msg)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //获得4字节IP地址
  uint8_t i;

  /* 获取本机IP和AP的IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* 获取原始控制码 */
  ctr_code.code = in_frame->ctr_code.code;
  
  /* 修改控制码：不需要应答的命令帧,是否透传由参数决定 */
  ctr_code.frame_type = ORDER_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type  = comm_type;
  ctr_code.comm_dir = ED_TO_AP;
  ctr_code.trans_type = ONCE_TRANS;

  /* 填充帧内容 */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = in_frame->frm_cnt;  
  out_msg[7] = 4 + para_len;//数据域为长度为 命令标识+命令内容
  for(i=0; i<4; i++)
  {
    out_msg[8+i] = in_frame->data[i];
  }
  for(i=0; i<para_len; i++)
  {
     out_msg[12+i] = reply_para[i];
  }
  out_msg[12+para_len] = gen_crc(out_msg, 12+para_len);//CS校验码
  out_msg[13+para_len] = APP_FRAME_TAIL;
  
  /* 返回帧长 */
  return para_len+14;
}

/**************************************************************************************************
 * @fn          bui_data_msg
 *
 * @brief       将输入的帧结构体以<数据帧>的形式构建为字节数组，以便于通过网络发送.
 *
 * @param       in_msg      - 输入的字节数组,作为数据帧的数据域
 *              in_msg_len  - 输入的字节数组长度
 *              out_msg     - 输出的字节数组
 *
 * @return      输出的字节数组长度
 **************************************************************************************************
 */
uint8_t bui_data_msg(const uint8_t *in_msg, uint8_t in_msg_len, uint8_t *out_msg)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};
  uint8_t i;
  extern uint8_t last_tx_frm_num;//发送的帧号

  /* 获取本机IP和AP的IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* 设置控制码：需要应答的数据帧,透传,单次传输 */
  ctr_code.frame_type = DATA_FRAME;
#ifdef NEED_REPLY_FRAME
  ctr_code.reply_type = NEED_REPLY;
#else
  ctr_code.reply_type = NO_NEED_REPLY;
#endif
  ctr_code.comm_type  = TRANSPARENT_COMM;
  ctr_code.trans_type = ONCE_TRANS;
  ctr_code.comm_dir = ED_TO_AP;

  /* 填充帧内容 */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  
  /* 填充帧计数器(已更新) */
  out_msg[6] = last_tx_frm_num;
    
  out_msg[7] = in_msg_len;
  
  /* 填充数据域 */
  for(i=0; i<in_msg_len; i++)
  {
    out_msg[8+i] = in_msg[i];
  }
 
  out_msg[8+in_msg_len] = gen_crc(out_msg, 8+in_msg_len);//CS校验码
  out_msg[9+in_msg_len] = APP_FRAME_TAIL;
  
  /* 返回帧长 */
  return 10+in_msg_len;
}


/**************************************************************************************************
 * @fn          bui_switch_state_msg
 *
 * @brief       只对改造开关的程序的有效，用于获取开关的状态.
 *
 * @param       comm_type   - 通信方式:
 *                            TRANSPARENT_COMM    - 透传,发送給集中器后集中器不处理,原样输出
 *                            NO_TRANSPARENT_COMM - 不透传,发送給集中器后集中器内部处理
 *              in_frame    - 输入的帧结构体
 *              reply_para  - 输入的命令返回结果字节数组
 *              para_len    - 命令返回结果字节数组长度
 *              out_msg     - 输出的字节数组
 *
 * @return      输出的字节数组长度
 **************************************************************************************************
 */
uint8_t bui_switch_state_msg(comm_type_t comm_type, const uint8_t *reply_para, uint8_t para_len, uint8_t *out_msg)
{
  uint8_t i;
  static uint8_t frm_cnt = 1;
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};
  
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  /* 获取原始控制码 */
  ctr_code.code = 0x81;
  
  /* 修改控制码：不需要应答的命令帧,是否透传由参数决定 */
  ctr_code.frame_type = ORDER_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type  = comm_type;
  ctr_code.comm_dir = ED_TO_AP;
  ctr_code.trans_type = ONCE_TRANS;

  /* 填充帧内容 */
  out_msg[0]  = APP_FRAME_HEAD;
  out_msg[1]  = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6]  = frm_cnt++;  
  out_msg[7]  = 4 + para_len;//数据域为长度为 命令标识+命令内容
  out_msg[8]  = 0xfa;
  out_msg[9]  = 0x80;
  out_msg[10] = 0x01;
  out_msg[11] = 0x00;

  for(i=0; i<para_len; i++)
  {
     out_msg[12+i] = reply_para[i];
  }
  out_msg[12+para_len] = gen_crc(out_msg, 12+para_len);//CS校验码
  out_msg[13+para_len] = APP_FRAME_TAIL;
  
  /* 返回帧长 */
  return para_len+14;
}


/**************************************************************************************************
 * @fn          send_reply_msg
 *
 * @brief       向集中器发送回复帧.
 *
 * @param       in_frame    - 输入的帧结构体
 *              reply_para  - 输入的命令返回结果字节数组
 *              para_len    - 命令返回结果字节数组长度
 *
 * @return      2，广播帧无需回复；1,发送成功；0，发送失败。
 **************************************************************************************************
 */
uint8_t send_reply_msg(const app_frame_t *in_frame, const uint8_t *reply_para, uint8_t para_len)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //获得4字节IP地址
  uint8_t i=0;
  uint8_t out_msg[MAX_APP_PAYLOAD] = {0};
  smplStatus_t rc;
  /* 获取本机IP和AP的IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* 获取原始控制码 */
  ctr_code.code = in_frame->ctr_code.code;
  
  /* 修改控制码：不需要应答的命令帧,是否透传由参数决定 */
  ctr_code.frame_type = ORDER_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
	if(in_frame->data[2]==0x2C)
  {
    ctr_code.comm_type  = NO_TRANSPARENT_COMM;	
	}
	else
	{
    ctr_code.comm_type  = TRANSPARENT_COMM;
	}
  ctr_code.comm_dir = ED_TO_AP;
  ctr_code.trans_type = ONCE_TRANS;

  /* 填充帧内容 */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = in_frame->frm_cnt;  
  out_msg[7] = 4 + para_len;//数据域为长度为 命令标识+命令内容
  for(i=0; i<4; i++)
  {
    out_msg[8+i] = in_frame->data[i];
  }
  for(i=0; i<para_len; i++)
  {
     out_msg[12+i] = reply_para[i];
  }
  out_msg[12+para_len] = gen_crc(out_msg, 12+para_len);//CS校验码
  out_msg[13+para_len] = APP_FRAME_TAIL;
  
  if((((in_frame->ed_addr_1<<8)|in_frame->ed_addr_0) != 0xffff) &&\
     (((in_frame->ed_addr_1<<8)|in_frame->ed_addr_0) != 0x0))
  {
    if(in_frame->ctr_code.comm_type == LOCAL_COMM)
    {
      // SEND_BYTES_TO_UART(out_msg, para_len+14);
			 put_multi_hex_uart1(out_msg, para_len+14);
       return 1;
    }
    else if(in_frame->ctr_code.comm_type == NO_TRANSPARENT_COMM)
    {
      rc = send_net_pkg(sLinkID1, out_msg, para_len+14);
      if(rc == SMPL_SUCCESS)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }
  return 2;
}

/**************************************************************************************************
 * @fn          send_alarm_msg
 *
 * @brief       对集中器发来的帧进行告警，指示帧处理中出现的错误.
 *
 * @param       in_frame    - 输入的帧结构体
 *              err_info    - 输入的告警信息字节数组
 *              err_len     - 告警信息字节数组长度
 *
 * @return      2，广播帧无需告警；1,发送成功；0，发送失败。
 **************************************************************************************************
 */
uint8_t send_alarm_msg(const app_frame_t *in_frame, const uint8_t *err_info, uint8_t err_len)
{
  uint8_t i;
  uint8_t out_msg[MAX_APP_PAYLOAD]={0};
  ctr_code_t ctr_code;
  smplStatus_t rc;
  ip_addr_t ipAddr = {0};   //获得4字节IP地址
  static uint8_t alarm_frm_cnt=0;//发送应答帧的帧计数器
  
  /* 获取本机IP和AP的IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* 获取原始控制码 */
  if(in_frame != NULL)
  {
    ctr_code.code = in_frame->ctr_code.code;
  }
  else
  {
    ctr_code.code = 0x00;
  }
  
  /* 修改控制码：不需要应答的告警帧,透传 */
  ctr_code.frame_type = ALARM_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type = TRANSPARENT_COMM;
  ctr_code.comm_dir = ED_TO_AP;

  /* 填充帧内容 */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  if(in_frame != NULL)
  {
    out_msg[6] = in_frame->frm_cnt;  
  }
  else
  {
    out_msg[6] = alarm_frm_cnt;  
    alarm_frm_cnt++;
  }
  out_msg[7] = err_len;
  for(i=0; i<err_len; i++)
  {
    out_msg[8+i] = err_info[i];
  }
  out_msg[8+err_len] = gen_crc(out_msg, 8+err_len);//CS校验码
  out_msg[9+err_len] = APP_FRAME_TAIL;

  if(((in_frame->ed_addr_1<<8)|in_frame->ed_addr_0) != 0xffff)
  {
    if(in_frame->ctr_code.comm_type == LOCAL_COMM)
    {
       //SEND_BYTES_TO_UART(out_msg, 10+err_len);
			 put_multi_hex_uart1(out_msg, 10+err_len);
       return 1;
    }
    else if(in_frame->ctr_code.comm_type == NO_TRANSPARENT_COMM)
    {
      rc = send_net_pkg(sLinkID1, out_msg, 10+err_len);
      if(rc == SMPL_SUCCESS)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }

  return 2;
}

/**************************************************************************************************
 * @fn          send_command_msg 
 *
 * @brief       节点主动向上发送命令帧，包括命令标识和数据.
 *
 * @param       commandID    - 命令标识，4个字节
 *              data         - 数据内容
 *              data_len     - 数据长度
 *
 * @return      1，发送成功；2，发送失败。
 **************************************************************************************************
 */
uint8_t send_command_msg(uint8_t *commandID, uint8_t *data, uint8_t data_len)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //获得4字节IP地址
  uint8_t i;
  uint8_t out_msg[MAX_APP_PAYLOAD] = {0};
  smplStatus_t rc;
  static uint8_t order_frm_cnt = 0;
  /* 获取本机IP和AP的IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* 修改控制码：不需要应答的命令帧,是否透传由参数决定 */
  ctr_code.frame_type = ORDER_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_type  = TRANSPARENT_COMM;
  ctr_code.comm_dir = ED_TO_AP;
  ctr_code.trans_type = ONCE_TRANS;

  /* 填充帧内容 */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = order_frm_cnt++;  
  out_msg[7] = 4 + data_len;//数据域为长度为 命令标识+命令内容
  for(i=0; i<4; i++)
  {
    out_msg[8+i] = commandID[i];
  }
  for(i=0; i<data_len; i++)
  {
     out_msg[12+i] = data[i];
  }
  out_msg[12+data_len] = gen_crc(out_msg, 12+data_len);//CS校验码
  out_msg[13+data_len] = APP_FRAME_TAIL;
  
  rc = send_net_pkg(sLinkID1, out_msg, data_len+14);
  if(rc == SMPL_SUCCESS)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/**************************************************************************************************
 * @fn          send_ack_msg
 *
 * @brief       发送应答帧.
 *
 * @param       in_frame    - 输入的帧结构体
 *
 * @return      2，广播帧不需应答；1，发送应答成功；0，发送应答失败。
 **************************************************************************************************
 */
uint8_t send_ack_msg(const app_frame_t *in_frame)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};
  uint8_t out_msg[MAX_APP_PAYLOAD] = {0};
  smplStatus_t rc;  
  /* 获取本机IP和AP的IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr); 
  
  /* 获取原始控制码 */
  ctr_code.code = in_frame->ctr_code.code;
  
  /* 修改控制码：不需要应答的应答帧 */
  ctr_code.frame_type = REPLY_FRAME;
  ctr_code.reply_type = NO_NEED_REPLY;
  ctr_code.comm_dir = ED_TO_AP;

  /* 填充帧内容 */
  out_msg[0] = APP_FRAME_HEAD;
  out_msg[1] = ctr_code.code;
  out_msg[2] = ipAddr.edAddr[1];
  out_msg[3] = ipAddr.edAddr[0];
  out_msg[4] = ipAddr.apAddr[1];
  out_msg[5] = ipAddr.apAddr[0];
  out_msg[6] = in_frame->frm_cnt;  
  out_msg[7] = 0;//数据域为长度为0
  out_msg[8] = gen_crc(out_msg, 8);//CS校验码
  out_msg[9] = APP_FRAME_TAIL;
  
  if(((in_frame->ed_addr_1<<8)|in_frame->ed_addr_0) != 0xffff)
  {
    if(in_frame->ctr_code.comm_type == LOCAL_COMM)
    {
      // SEND_BYTES_TO_UART(out_msg, 10);
			 put_multi_hex_uart1(out_msg, 10);
       return 1;
    }
    else
    {
      rc = send_net_pkg(sLinkID1, out_msg, 10);
      if(rc == SMPL_SUCCESS)
      {
        return 1;
      }
      else
      {
        return 0;
      }
    }
  }
  else
  {
    return 2;
  }
}

/***************************************************************************
 * @fn          bui_pkg_ack
 *     
 * @brief       构建68、88帧的应答帧
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg     - 输入的68、88帧
 *              out_pkg    - 返回的字节数组
 *     
 * @return      回复帧长度
 ***************************************************************************
 */ 
uint8_t bui_pkg_ack(app_pkg_t *in_pkg,uint8_t *out_pkg)
{
  uint8_t ctr_code;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  ctr_code &= ~BIT0;
  ctr_code |= BIT1;
  *out_pkg = in_pkg->frm_head;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  *(out_pkg+7) = 0;
    
   
  *(out_pkg+8)=gen_crc(out_pkg,8);//CS校验码
  *(out_pkg+9)=0x16; 
  return 10;
}

/***************************************************************************
 * @fn          bui_pkg_alarm
 *     
 * @brief       构建68帧的告警帧
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg     - 输入的88帧
 *              err_info   - 告警内容
 *              err_len    - 告警来源
 *              out_pkg    - 返回的字节数组
 *     
 * @return      告警帧长度
 ***************************************************************************
 */ 
uint8_t bui_pkg_alarm(app_pkg_t *in_pkg,uint8_t *err_info,uint8_t err_len,uint8_t *out_pkg)
{
  uint8_t ctr_code;
  uint8_t i;
  uint8_t k;
  static uint8_t alarm_frm_cnt=0;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  ctr_code |= BIT0;
  ctr_code |= BIT1;
  *out_pkg = in_pkg->frm_head;
  if(in_pkg != NULL)
  {
    *(out_pkg+1) = ctr_code;
    *(out_pkg+2) = in_pkg->ed_addr >>8;
    *(out_pkg+3) = in_pkg->ed_addr;
    *(out_pkg+4) = in_pkg->ap_addr >>8;
    *(out_pkg+5) = in_pkg->ap_addr; 
    *(out_pkg+6) = in_pkg->frm_cnt;  
    *(out_pkg+7) = err_len;
    for(k=0;k<err_len;k++)
      *(out_pkg+8+k) = *(err_info+k);             
    *(out_pkg+8+err_len)=gen_crc(out_pkg,8+err_len);//CS校验码
    *(out_pkg+9+err_len)=0x16;
    return 10+err_len;  
  }
  else
  { 
    alarm_frm_cnt ++;
        
   *(out_pkg+1) = 0x03;
    for(i=0;i<2;i++)
      *(out_pkg+i+2) = 0xee;
    *(out_pkg+4) = 0xff;
    *(out_pkg+5) = 0xff; 
    *(out_pkg+6) = alarm_frm_cnt;   
    //*(out_pkg+7) = 1;
    *(out_pkg+7) = err_len;
    for(k=0;k<err_len;k++)
      *(out_pkg+8+k) = *(err_info+k);             
    *(out_pkg+8+err_len)=gen_crc(out_pkg,8+err_len);//CS校验码
    *(out_pkg+9+err_len)=0x16;
    return 10+err_len;  
  } 
}



uint8_t bui_rep_pkg_command(app_pkg_t *in_pkg,uint8_t *reply_para,uint8_t para_len,uint8_t *out_pkg)
{

  uint8_t ctr_code;
  uint8_t j,k;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  *out_pkg = 0x68;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  *(out_pkg+7) = para_len+4;
   for(j=0;j<4;j++)
    *(out_pkg+8+j) = in_pkg->msg[j];
   for(k=0;k<para_len;k++)
     *(out_pkg+12+k) = *(reply_para+k);
   
  *(out_pkg+12+para_len)=gen_crc(out_pkg,12+para_len);//CS校验码
  *(out_pkg+13+para_len)=0x16;
  
  return para_len+14;
}

/***************************************************************************
 * @fn          bui_app_pkg
 *     
 * @brief       将帧字节数组转换成结构体数据，便于后续处理
 *     
 * @data        2015年08月07日
 *     
 * @param       msg        - 帧字节数组
 *              app_in_pkg - 转换后的结构体数据
 *     
 * @return      void
 ***************************************************************************
 */ 
void bui_app_pkg(uint8_t *msg,app_pkg_t *app_in_pkg)
{
  uint8_t j = 0;

  /*构造帧起始符1*/
  app_in_pkg->frm_head  =*msg;//
  /*构造帧地址a0-A1*/
  app_in_pkg->ctr_code  =*(msg+1);
  app_in_pkg->ed_addr   =*(msg+2);
  app_in_pkg->ed_addr   = (app_in_pkg->ed_addr<<8) |  *(msg+3);
  app_in_pkg->ap_addr   =*(msg+4);
  app_in_pkg->ap_addr   =(app_in_pkg->ap_addr<<8) | *(msg+5);
  app_in_pkg->frm_cnt   = *(msg+6);
 
  /*构造帧数据域长度*/
  app_in_pkg->data_len  =*(msg+7);
 
  /*构造帧数据域*/
  for( j=0;j<app_in_pkg->data_len;j++)
    app_in_pkg->msg[j]  =*(msg+8+j);

  /*构造帧校验码CS*/
  app_in_pkg->cs=*(msg+8+app_in_pkg->data_len);
 
  /*构造帧结束符*/
  app_in_pkg->frm_end =*(msg+9+app_in_pkg->data_len);

}


/**************************************************************************************************
 * @fn          bui_ed_to_ap_msg
 *
 * @brief       将输入的帧结构体以<数据帧>的形式构建为字节数组，以便于通过网络发送.
 *
 * @param       in_msg      - 输入的字节数组,作为数据帧的数据域
 *              in_msg_len  - 输入的字节数组长度
 *              out_msg     - 输出的字节数组
 *
 * @return      输出的字节数组长度
 **************************************************************************************************
 */
uint8_t bui_ed_to_ap_msg(const uint8_t *in_msg, uint8_t in_msg_len, uint8_t *out_msg)
{
  memcpy(out_msg,in_msg,in_msg_len);
  out_msg[1] |= 0x80;
  out_msg[in_msg_len-2] += 0x80;
  return in_msg_len;
}
/**************************************************************************************************
 */
