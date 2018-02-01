/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_frm_proc.c
// Description: User APP.
//              SWSN网络帧初步处理源文件.
// Author:      Leidi
// Version:     1.0
// Date:        2012-3-24
// History:     2014-9-25   Leidi 删除全局变量ApAddr,使用官方函数nwk_getAPAddress()获取AP地址.
//              2014-9-26   Leidi 删除无用全局变量head_list.
//                                删除无用全局变量continue_msg[200].
//                                删除无用的全局变量net_ed_num, net_ed_add_sem.
//                                删除冗余头文件.
//                                修改sCB()的返回值,当非本机LID时返回1.
//                                net_frm_proc()的函数返回值改为void.
//                                修改proc_net_msg()的函数原型与定义,去除无用的参数lid.输入参数添加const修饰符.
//              2014-9-27   Leidi 修改proc_net_msg()的函数原型与定义,恢复参数lid.
//                                修改proc_net_msg()的内容.
//              2014-9-28   Leidi 移除ap_inner_fun.h,不调用ap_inner_proc(),此函数的功能已过时.
//              2014-9-29   Leidi proc_bcast_frm()由以下两个函数代替:bcast_frm_proc(),proc_bcast_msg().
//                                移除本文件中无用的宏定义MISSES_IN_A_ROW.
//              2014-9-30   Leidi 全局变量last_tx_frm_num和last_rx_frm_num在本文件中定义.
//              2014-10-8   Leidi 删除全局变量recv_cont_frm_flag和cont_frm_minu_count及相关代码.此变量用于在接收连续数据帧时不使用跳频.
//              2014-10-29  Leidi 在函数proc_net_msg()中,将原先只过滤透传、需要应答的重复帧,现过滤透传的重复帧.
//              2014-11-10  Leidi 移除头文件stm8s_uart.h.
//                                添加用于配置的头文件user_app_config.h.
//              2015-01-20  Leidi 修复Bug:函数net_frm_proc()中SMPL_Receive失败时不处理net_frm_cur_num.
//              2015-07-29  Leidi 删除引用头文件nwk_types.h.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "net_frm_proc.h"
#include "bui_pkg_fun.h"
#include "net_para_fun.h"
#include "net_status_fun.h"
#include "uart_frm_proc.h"
#include "nwk_api.h"
#include "nwk.h"
#include "bsp.h"
#include "stm32_usart2_gtway.h"
#include <string.h>
#include "ed_tx_fail_buffer_fun.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern linkID_t sLinkID1;        //连接标识符(LID)
volatile uint8_t net_frm_cur_num=0;  //底层接收队列中非广播帧的数量
volatile uint8_t bro_cast_sem=0;     //底层接收队列中广播帧的数量,读取完之后会减少
volatile uint8_t bro_cast_total_num =0; //底层接收队列中广播帧的数量，一直递增
extern volatile net_frm_back_t net_ack_back;

uint8_t send_usart_frm_count=0;

uint8_t last_tx_frm_num;//上一次发送的帧号
uint8_t last_rx_frm_num;//上一次成功接收的帧号

uint8_t reply_frame_received;   //有效应答帧接收标志:收到有效应答帧时非0


/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void proc_net_msg(linkID_t lid, const uint8_t *, uint8_t);
static void proc_bcast_msg(const uint8_t *msg, uint8_t len);
static void bui_usart68_frm(uint8_t * ,uint8_t *,uint8_t data_len,uint8_t);

extern uint8_t registerAppFuncDomain(uint8_t ,pAppFunc );
/**************************************************************************************************
 * @fn          sCB
 *
 * @brief       ED端的射频连接接收回调函数,在中断服务函数的运行.根据连接标识符(LID)识别收到的用户帧.
 *              帧的读取应该在用户应用程序而不是在此处进行.具体参见SimpliciTI API(SWRA221 Version 1.2)
 *              第7部分Callback Interface.
 *
 * @param       lid - 收到用户帧的连接标识符(LID)
 *
 * @return      0         - 有用的帧,收到的帧将留在输入队列中供用户稍后取回
 *              non-zero  - 无用的帧,收到的帧将将从输入队列中释放
 **************************************************************************************************
 */
uint8_t sCB(linkID_t lid)
{
  if(lid == sLinkID1)//如果是本机的LID
  {
    net_frm_cur_num++;
  }
  else if(lid == SMPL_LINKID_USER_UUD)//如果是无连接(广播)的LID
  {
    bro_cast_sem++;
    bro_cast_total_num++;
  }
  else//如果是非本机的LID 
  {
    return 1;
  }
 
  return 0;
}


/**************************************************************************************************
 * @fn          net_frm_proc
 *
 * @brief       根据本机LID取回非广播用户帧并交由处理函数解析.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void net_frm_proc(void)
{
  uint8_t msg[MAX_APP_PAYLOAD], len;
  uint8_t alarm_info[4];
  bspIState_t intState;
  
  //PRINT_DEBUG_MSG("Proc net frm...\n");

  if(net_frm_cur_num)
  {
    /* process all frames waiting */
    if (SMPL_SUCCESS == SMPL_Receive(sLinkID1, msg, &len))
    {
      /* 处理收到的帧 */
      proc_net_msg(sLinkID1, msg, len);
    }  
    else
    {
     /* 构建告警帧 */
     alarm_info[0]=0x00;
     alarm_info[1]=0x01;
     alarm_info[2]=0x01;
     alarm_info[3]=0xfe;
     len = bui_alarm_msg(NULL, alarm_info, 4, msg);
     
     /* 发送告警帧 */
     send_net_pkg(sLinkID1,msg,len);
    
     //PRINT_DEBUG_MSG("Process net frm ERROR:");
     //PRINT_DEBUG_BYTES(msg,len);
    }
        
    BSP_ENTER_CRITICAL_SECTION(intState);
    net_frm_cur_num--;
    BSP_EXIT_CRITICAL_SECTION(intState);
  }
}


/**************************************************************************************************
 * @fn          proc_net_msg
 *
 * @brief       对非广播用户帧格式进行解析并分类处理.
 *
 * @param       lid - 连接标示符,用于发送返回帧
 *              msg - 接收到的用户帧
 *              len - 接收到的用户帧长度
 *
 * @return      none
 **************************************************************************************************
 */
static void proc_net_msg(linkID_t lid, const uint8_t *msg, uint8_t len)
{
  app_frame_t   in_frame;
  frame_type_t  frame_type;
  comm_type_t   comm_type;
  //trans_type_t  trans_type;
  reply_type_t  reply_type;
  uint8_t out_msg[MAX_APP_PAYLOAD];
  uint8_t out_msg_len;
  uint8_t alarm_info[4];
 
//  PRINT_DEBUG_MSG("Proc net msg...\n");
  /* 解析从AP发来的帧 */
  if(APP_SUCCESS == bui_app_frame(msg, len, &in_frame))
  {
    frame_type  = (frame_type_t)in_frame.ctr_code.frame_type;  //帧类型
    comm_type   = (comm_type_t)in_frame.ctr_code.comm_type;    //通信方式
    //trans_type  = (trans_type_t)in_frame.ctr_code.trans_type;  //传输状态
    reply_type  = (reply_type_t)in_frame.ctr_code.reply_type;  //应答需求
       
    if(comm_type == TRANSPARENT_COMM)//集中器访问节点，数据透传
    {
      /*if(last_rx_frm_num == in_frame.frm_cnt)//新收到的帧与上次收到的帧号相同，说明是重发帧或不需要应答的帧
      {
        if(reply_type == NEED_REPLY)//如果需要应答，说明是重复帧
        {
          send_ack_msg(&in_frame);//构建应答帧
          //return;//不处理需要应答的重复帧
        }
        return;//不处理重复帧
      }*/
      /* 运行到此处说明不是重发帧或是不需要应答的帧 */
      
      if(frame_type == DATA_FRAME)//数据帧
      {
        //PRINT_DEBUG_MSG("Recv data msg:");
        //PRINT_DEBUG_BYTES(msg,len);
        
      //  SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//只输出帧负载  
        put_multi_hex_uart1(in_frame.data, in_frame.data_len);				
//        out_msg_len = bui_ed_to_ap_msg(msg,len,out_msg);
//        send_net_pkg(lid, out_msg, out_msg_len);
      }
      else //非数据帧
      {                    
       // PRINT_DEBUG_MSG("Recv msg:");
       // PRINT_DEBUG_BYTES(msg,len);
       // SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//只输出帧负载  
				put_multi_hex_uart1(in_frame.data, in_frame.data_len);	
      }
      
      /* 运行到此处说明有效帧已经通过串口输出 */
      last_rx_frm_num = in_frame.frm_cnt;//更新接收帧号
      if(reply_type == NEED_REPLY)
      {
        out_msg_len = bui_reply_msg(&in_frame, out_msg);//构建应答帧
        send_net_pkg(lid, out_msg, out_msg_len);//发送应答帧
      }
      
    }
    else if(comm_type == NO_TRANSPARENT_COMM)//集中器访问节点，数据不透传
    {
      if(frame_type == REPLY_FRAME)//应答帧
      {
        if(last_tx_frm_num == in_frame.frm_cnt)//新收到的帧与上次发送的帧号相同，说明是上次发送的应答帧
        {
          reply_frame_received = 0x01;
        }
        else//无效的应答帧
        {
          reply_frame_received = 0x00;
        }
      }
      else if(frame_type == ORDER_FRAME)//命令帧 
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
          alarm_info[0]=0x00;
          alarm_info[1]=0x01;
          alarm_info[2]=0x01;
          alarm_info[3]=0x08;
          send_alarm_msg(&in_frame, alarm_info, 4);
        }
        
       /* 发送回复帧 */
      //  send_net_pkg(lid, out_msg, out_msg_len);
      }
      else//告警帧或数据帧
      {
      //  PRINT_DEBUG_MSG("net frame type ERROR!\n");
      //  PRINT_DEBUG_BYTES(msg,len);
      }
    }
		
		if(comm_type == RESERVE_COMM)//   RESERVE
    {
      /*if(last_rx_frm_num == in_frame.frm_cnt)//新收到的帧与上次收到的帧号相同，说明是重发帧或不需要应答的帧
      {
        if(reply_type == NEED_REPLY)//如果需要应答，说明是重复帧
        {
          send_ack_msg(&in_frame);//构建应答帧
          //return;//不处理需要应答的重复帧
        }
        return;//不处理重复帧
      }*/
      /* 运行到此处说明不是重发帧或是不需要应答的帧 */
      
      if(frame_type == DATA_FRAME)//数据帧
      {
        //PRINT_DEBUG_MSG("Recv data msg:");
        //PRINT_DEBUG_BYTES(msg,len);
        
        //SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//只输出帧负载  
        put_multi_hex_uart1(in_frame.data, in_frame.data_len);				
        out_msg_len = bui_ed_to_ap_msg(msg,len,out_msg);
        send_net_pkg(lid, out_msg, out_msg_len);
      }
      else //非数据帧
      {                    
       // PRINT_DEBUG_MSG("Recv msg:");
       // PRINT_DEBUG_BYTES(msg,len);
       // SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//只输出帧负载  
				put_multi_hex_uart1(in_frame.data, in_frame.data_len);	
      }
      
      /* 运行到此处说明有效帧已经通过串口输出 */
      last_rx_frm_num = in_frame.frm_cnt;//更新接收帧号
      if(reply_type == NEED_REPLY)
      {
        out_msg_len = bui_reply_msg(&in_frame, out_msg);//构建应答帧
        send_net_pkg(lid, out_msg, out_msg_len);//发送应答帧
      }
      
    }
		
	  return;
  }
  else //非法帧:帧格式不正确
  {
   // PRINT_DEBUG_MSG("Recv illegal msg:");
    //PRINT_DEBUG_BYTES(msg,len);
		  return;
  }
	
}


/**************************************************************************************************
 * @fn          bcast_frm_proc
 *
 * @brief       取回广播用户帧并交由处理函数解析.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void bcast_frm_proc(void)
{
  uint8_t msg[MAX_APP_PAYLOAD], len;
  bspIState_t intState;
  
//  PRINT_DEBUG_MSG("Proc BCast frm...\n");

  if(bro_cast_sem)
  {
    /* process all frames waiting */
    if (SMPL_SUCCESS == SMPL_Receive(SMPL_LINKID_USER_UUD, msg, &len))
    {
      /* 处理收到的帧 */
      proc_bcast_msg(msg, len);
      
      BSP_ENTER_CRITICAL_SECTION(intState);
      bro_cast_sem--;
      BSP_EXIT_CRITICAL_SECTION(intState);
     }  
     else
     {
//       PRINT_DEBUG_MSG("Recv BCast frm ERROR!\n");
     } 
  }
}


/**************************************************************************************************
 * @fn          proc_bcast_msg
 *
 * @brief       对广播用户帧格式进行解析并分类处理.
 *
 * @param       msg - 接收到的用户帧
 *              len - 接收到的用户帧长度
 *
 * @return      none
 **************************************************************************************************
 */
void proc_bcast_msg(const uint8_t *msg, uint8_t len)
{
  app_frame_t   in_frame;
  frame_type_t  frame_type;
  comm_type_t   comm_type;
  uint8_t alarm_info[4];
//  PRINT_DEBUG_MSG("Proc BCast msg...\n");

  /* 解析从AP发来的帧 */
  if(APP_SUCCESS == bui_app_frame(msg, len, &in_frame))
  {
    frame_type  = (frame_type_t)in_frame.ctr_code.frame_type;  //帧类型
    comm_type   = (comm_type_t)in_frame.ctr_code.comm_type;    //通信方式
   
    last_rx_frm_num = in_frame.frm_cnt;//获取接收帧号
    
    if(comm_type == TRANSPARENT_COMM)//集中器访问节点，数据透传
    {
//      PRINT_DEBUG_MSG("Recv BCast msg:");
//      PRINT_DEBUG_BYTES(msg,len);
      
      //SEND_BYTES_TO_UART(msg, len);//输出整条帧
		//	put_multi_hex_uart1(msg, len);	
//      SEND_BYTES_TO_UART(in_frame.data, in_frame.data_len);//只输出帧负载
			put_multi_hex_uart1(in_frame.data, in_frame.data_len);	
    }
    else if(comm_type == NO_TRANSPARENT_COMM)//集中器访问节点，数据不透传
    {
      if(frame_type == ORDER_FRAME)//命令帧
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
          alarm_info[0]=0x00;
          alarm_info[1]=0x01;
          alarm_info[2]=0x01;
          alarm_info[3]=0x08;
          send_alarm_msg(&in_frame, alarm_info, 4);;
        }
//        PRINT_DEBUG_MSG("BCast order frame execute result:");
        //PRINT_DEBUG_BYTES(out_msg, out_msg_len);
      }
      else//广播帧不支持此类型:告警帧或应答帧
      {
//        PRINT_DEBUG_MSG("BCast frame type ERROR!\n");
        //PRINT_DEBUG_BYTES(msg,len);
      }
    }
    else//通信方式错误：广播帧不应有此通信方式
    {
//      PRINT_DEBUG_MSG("BCast Comm type ERROR!\n");
//      PRINT_DEBUG_BYTES(msg,len);
    }
  }
  else //非法帧:帧格式不正确
  {
//    PRINT_DEBUG_MSG("Recv illegal BCast msg:");
//    PRINT_DEBUG_BYTES(msg,len);
  }
}

smplStatus_t	send_usartdata_to_ap(uint8_t * usart_data,uint8_t data_len)
{
	if(data_len!=0)
  {
	  uint8_t out_msg[MAX_APP_PAYLOAD] = {0};		
		send_usart_frm_count++;		
		
    bui_usart68_frm(usart_data,out_msg,data_len,send_usart_frm_count);			
		
	  send_usart_data_pkg(sLinkID1, out_msg, 10+data_len);
  }
}

/**************************************************************************************************
 * @fn          send_usart_data_pkg
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
uint8_t send_usart_data_pkg(linkID_t lid, uint8_t *string, uint8_t tx_len)
{
  uint8_t       rc;
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"Sending msg to AP..."RTT_CTRL_RESET"\n");
#endif
    
    rc = send_user_msg(lid, string, tx_len);
    if (1 == rc)
    {
#ifdef DEBUG
      put_string_uart1("Send success!\n");
      put_multi_char_uart1(string, tx_len);
#endif
//      for(i = 0; i < 30; i++)
//      {
//        SWSN_DELAY(1);   
//        if((net_ack_back.lid == lid) && (net_ack_back.isBack == 1))
//        {
//          memset((void*)&net_ack_back,0x0,sizeof(net_ack_back));
#ifdef IWDG_START 
          IWDG_ReloadCounter();  
#endif
          return rc;
//        }
//      } 
//      Write_tx_fail_ed_buffer(lid, string, &tx_len); 
    }
    else 
    {
    }
  
  if(rc !=1)  
  {			
    Write_tx_fail_ed_buffer(lid, string, &tx_len);  
  }
#ifdef IWDG_START 
   IWDG_ReloadCounter();  
#endif  
  return rc;           
}	
	
void bui_usart68_frm(uint8_t * usart_data,uint8_t * out_msg,uint8_t data_len,uint8_t frm_cnt)
{
  ctr_code_t ctr_code;
  ip_addr_t ipAddr = {0};   //获得4字节IP地址
  uint8_t i=0;

  /* 获取本机IP和AP的IP */
  SMPL_Ioctl(IOCTL_OBJ_IP_ADDR,IOCTL_ACT_GET,(void *)&ipAddr);
  
  /* 修改控制码：不需要应答的命令帧,是否透传由参数决定 */
  ctr_code.frame_type = DATA_FRAME;
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
  out_msg[6] = frm_cnt;  
  out_msg[7] = data_len;//数据域为长度为 命令标识+命令内容
  for(i=0; i<data_len; i++)
  {
    out_msg[8+i] = *(usart_data+i);
  }
  out_msg[8+data_len] = gen_crc(out_msg, 8+data_len);//CS校验码
  out_msg[8+1+data_len] = APP_FRAME_TAIL;
	
}

/**************************************************************************************************
 */
