/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ed_buffer_fun.c
// Description: User APP.
//              节点收发缓冲区处理头文件.
// Author:      Leidi
// Version:     1.1
// Date:        2014-10-21
// History:     2014-10-21  Leidi 初始版本建立.
//              2014-10-31  Leidi 修改函数proc_tx_ed_info()的逻辑,不需要应答的帧直接发送.
//              2014-11-03  Leidi 修改函数Write_tx_ed_buffer(),增加组播帧(0x67帧)的处理.
//              2014-04-24  Leidi 在每个节点的缓冲区中增加读写ID，以形成环形队列.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "ed_tx_buffer_fun.h"
#include "gtway_frm_proc.h"
#include "firmware_info_proc.h"
#include "68_frm_proc.h"
#include "67_frm_proc.h"
#include "bui_pkg_fun.h"
#include "nwk_layer.h"
#include <string.h>
#include "net_list_proc.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
/* 节点的发送缓冲区 */
static ed_tx_buffer_t Ed_tx_buffer[TX_BUFFER_NUM_FOR_ALL_ED];

/* 节点的发送缓冲区使用量 */
static uint8_t ed_tx_buffer_usage;

/* 网关所连接的服务器IP地址 */
extern  addr_t sGtwayServerIP;
extern volatile uint8_t emergency_dispatch_flag ; 
extern volatile uint8_t start_emergency_dispatch_in_rtc ;
/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static ed_tx_buffer_t *search_ip_in_ed_tx_buffer(uint16_t ip);
static ed_tx_buffer_t *add_new_ed_in_ed_tx_buffer(uint16_t ip);
static gtway_frm_info_t *find_a_avaliable_ed_tx_info(ed_tx_buffer_t *pEd_tx_buffer);
static void write_a_ed_tx_info(gtway_frm_info_t *pEd_tx_info, const gtway_pkg_t *pIn_gtway_pkg);
static void write_usart1_data_info(gtway_frm_info_t *pEd_tx_info, uint8_t *pIn_pkg);

static void change_gtway_frm_info_for_reply(gtway_frm_info_t *pEd_tx_info);
static uint8_t bui_gtway_msg_from_gtway_frm_info(const gtway_frm_info_t *pEd_tx_info, uint8_t *pOut_msg);
static void proc_tx_ed_info(uint16_t ip, gtway_frm_info_t *pEd_tx_info);

/**************************************************************************************************
 * @fn          Write_tx_ed_buffer
 *
 * @brief       用69帧的信息写节点发送缓冲区.
 *
 * @param       pIn_gtway_pkg - 接收到的69帧信息
 *              pOut_msg      - 输出的返回帧(如告警帧)
 *              pOut_len      - 输出的返回帧长度
 *
 * @return      none
 **************************************************************************************************
 */
void Write_tx_ed_buffer(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t* pOut_len)//用69帧的信息写节点发送缓冲区
{
  uint8_t           inner_frm_head = GET_FRAME_HEAD(pIn_gtway_pkg->msg); //获取内部数据帧帧头
  uint16_t          ed_ip;   //节点地址
  uint8_t           mult_ip_count;  //IP数
  uint16_t          mult_ip_group[IP_GROUP_MAX_NUM];//存储IP地址的数组
  uint8_t           i;
  ed_tx_buffer_t    *pEd_tx_buffer; //指向单个节点发送缓冲区的指针 
  gtway_frm_info_t  *pEd_tx_info;   //指向单条缓冲区的指针
  app_pkg_t         in_pkg;         //内部帧结构体变量  
  uint8_t           alarm_info[10]; //告警信息
	
	static uint8_t right_frm_num[1024]={0};
	static uint16_t frm_count=0;
	
  switch(inner_frm_head)
  {
    case 0x68:  /* 与节点通信:单播帧头 */
    {
      ed_ip = GET_ED_IP(pIn_gtway_pkg->msg);//获得节点地址
      
      pEd_tx_buffer = search_ip_in_ed_tx_buffer(ed_ip); //用IP地址匹配发送缓冲区地址�
      if (pEd_tx_buffer == NULL)//未找到
      {
        pEd_tx_buffer = add_new_ed_in_ed_tx_buffer(ed_ip); //在发送缓冲区添加节点IP
        if (pEd_tx_buffer == NULL)
        {
          /* 所有节点发送缓存区已经分配完 */
          goto BUILD_ALARM_FRM;
        }
      }
      
      pEd_tx_info = find_a_avaliable_ed_tx_info(pEd_tx_buffer);
			
      if (pEd_tx_info == NULL)
      {
        /* 此节点无可用的缓存区 */
//				Write_tx_fail_ed_buffer(uint8_t lid, uint8_t *pIn_msg, uint8_t* pIn_len);
				pEd_tx_buffer = add_new_ed_in_ed_tx_buffer(ed_ip);
        if (pEd_tx_buffer == NULL)
        {
          /* 所有节点发送缓存区已经分配完 */
          goto BUILD_ALARM_FRM;
        }
				pEd_tx_info = find_a_avaliable_ed_tx_info(pEd_tx_buffer); 
        if (pEd_tx_info == NULL)
        {
					goto BUILD_ALARM_FRM;
			  }
      }
			
      write_a_ed_tx_info(pEd_tx_info, pIn_gtway_pkg);
      
      break;
    }
    case 0x67:    /* 与节点通信:组播帧头 */
    {
      mult_ip_count = Change_67frm_to_68frm((gtway_pkg_t *)pIn_gtway_pkg, mult_ip_group);
      
      /* 检测组播帧中的IP数量,以防数组越界 */
      mult_ip_count = (mult_ip_count > IP_GROUP_MAX_NUM) ? IP_GROUP_MAX_NUM : mult_ip_count;

      for(i = 0; i<mult_ip_count; ++i)
      {
        ed_ip = mult_ip_group[i];
        
        pEd_tx_buffer = search_ip_in_ed_tx_buffer(ed_ip);
        if (pEd_tx_buffer == NULL)
        {
          pEd_tx_buffer = add_new_ed_in_ed_tx_buffer(ed_ip);
          if (pEd_tx_buffer == NULL)
          {
            /* 所有节点发送缓存区已经分配完 */
            goto BUILD_ALARM_FRM;
          }
        }
        
        pEd_tx_info = find_a_avaliable_ed_tx_info(pEd_tx_buffer);
        if (pEd_tx_info == NULL)
        {
          /* 此节点无可用的缓存区 */
          goto BUILD_ALARM_FRM;
        }
        write_a_ed_tx_info(pEd_tx_info, pIn_gtway_pkg);
      }
      
      break;
    }
    default:
    {
      /* 错误的帧头 */
    }
  }
  
  return;
  
BUILD_ALARM_FRM:  
  { 
    /* 建立告警帧 */
    
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x04;
    alarm_info[2] = 0x03;
    alarm_info[3] = 0x07;

    bui_app_pkg((uint8_t *)pIn_gtway_pkg->msg, &in_pkg);
    *pOut_len = bui_pkg_alarm(&in_pkg, alarm_info, 4, pOut_msg);
  }
}

/**************************************************************************************************
 * @fn          search_ip_in_ed_tx_buffer
 *
 * @brief       用IP地址查找匹配的节点发送缓冲区.
 *
 * @param       ip  - 节点IP地址
 *
 * @return      匹配的节点发送缓冲区
 **************************************************************************************************
 */
static ed_tx_buffer_t *search_ip_in_ed_tx_buffer(uint16_t ip)
{
  uint8_t i;

  for (i = 0; i < TX_BUFFER_NUM_FOR_ALL_ED; ++i)
  {
    if ((Ed_tx_buffer[i].in_use == BUFFER_IN_USE) \
      && (Ed_tx_buffer[i].ed_ip == ip))
    {
      return &Ed_tx_buffer[i];
    }
  }

  return NULL;
}

/**************************************************************************************************
 * @fn          add_new_ed_in_ed_tx_buffer
 *
 * @brief       用IP地址初始化新的节点发送缓冲区.
 *
 * @param       ip  - 节点IP地址
 *
 * @return      新的节点发送缓冲区
 **************************************************************************************************
 */
static ed_tx_buffer_t *add_new_ed_in_ed_tx_buffer(uint16_t ip)
{
  uint8_t i;

  for (i = 0; i < TX_BUFFER_NUM_FOR_ALL_ED; ++i)
  {
    if (Ed_tx_buffer[i].in_use == BUFFER_AVALIABLE)
    {
      Ed_tx_buffer[i].ed_ip = ip;
      Ed_tx_buffer[i].in_use = BUFFER_IN_USE;

      return &Ed_tx_buffer[i];
    }
  }

  return NULL;
}

/**************************************************************************************************
 * @fn          find_a_avaliable_ed_tx_info
 *
 * @brief       在指定节点的发送缓冲区中找一个可用的位置.
 *
 * @param       pEd_tx_buffer - 指定节点的发送缓冲区
 *
 * @return      可用的位置,可以存放1条68帧和69帧中的有用信息
 **************************************************************************************************
 */
static gtway_frm_info_t *find_a_avaliable_ed_tx_info(ed_tx_buffer_t *pEd_tx_buffer)
{
  uint8_t read_id = pEd_tx_buffer->ed_tx_info_read_id;
  uint8_t write_id = pEd_tx_buffer->ed_tx_info_write_id;
  uint8_t i;
  /* 下一个write_id */
  write_id = (write_id == TX_BUFFER_NUM_FOR_ONE_ED - 1) ? 0x00 : ++write_id;
  
//  if((write_id == read_id) && 
//    (pEd_tx_buffer->ed_tx_info[write_id].in_use == BUFFER_IN_USE))
//  {
//    return NULL;  //缓冲区已满
//  }
	
	for(i=0;i<TX_BUFFER_NUM_FOR_ONE_ED;i++)
  {
	  if(pEd_tx_buffer->ed_tx_info[i].in_use == BUFFER_AVALIABLE)
    {
    return &pEd_tx_buffer->ed_tx_info[i];		  
		}
	}
	
    return NULL;  //缓冲区已满
	
//  else  //缓冲区未满
//  {
//    /* 更新write_id */
//    pEd_tx_buffer->ed_tx_info_write_id = write_id;
//    
//    /* 返回对应的缓冲区 */
//    return &pEd_tx_buffer->ed_tx_info[write_id];
//  }
	
}

/**************************************************************************************************
 * @fn          write_a_ed_tx_info
 *
 * @brief       将一条69帧中的有用信息写入指定节点的发送缓冲区中.
 *
 * @param       pEd_tx_info   - 节点的发送缓冲区
 *              pIn_gtway_pkg - 69帧结构
 *
 * @return      none
 **************************************************************************************************
 */
static void write_a_ed_tx_info(gtway_frm_info_t *pEd_tx_info, const gtway_pkg_t *pIn_gtway_pkg)
{
  uint8_t i;
  uint8_t inner_msg_len = GET_FRAME_LEN(pIn_gtway_pkg->msg);
  
  pEd_tx_info->ctr_code = pIn_gtway_pkg->ctr_code;

//  for (i = 0; i < 4; ++i)
//  {
//    pEd_tx_info->server_addr[i] = pIn_gtway_pkg->server_addr[i];
//  }

  for (i = 0; i < 6; ++i)
  {
    pEd_tx_info->mobile_addr[i] = pIn_gtway_pkg->mobile_addr[i];
  }

  pEd_tx_info->pid_num = pIn_gtway_pkg->pid_num;
  
  pEd_tx_info->inner_msg_len = inner_msg_len;

  memcpy(pEd_tx_info->inner_msg, pIn_gtway_pkg->msg, inner_msg_len);
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"TX BUFFER DATA"RTT_CTRL_RESET"\n");
	SEGGER_RTT_put_multi_char((uint8_t *)pEd_tx_info->inner_msg,inner_msg_len);
#endif 
  pEd_tx_info->in_use = BUFFER_IN_USE;
  
  ed_tx_buffer_usage++;
}

/**************************************************************************************************
 * @fn          Iterates_tx_ed_buffer
 *
 * @brief       遍历所有节点的发送缓冲区,处理其中的信息.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Iterates_tx_ed_buffer(void)
{
	uint8_t avalible_index;
  uint8_t i = 0;
  uint16_t ed_ip;
  uint8_t read_id;
  uint8_t write_id;
  volatile uint8_t tmp = 0;
    
  if (ed_tx_buffer_usage == 0)
  {
    return;
  }

  for (i=0; i < TX_BUFFER_NUM_FOR_ALL_ED; ++i)
  {
    if (Ed_tx_buffer[i].in_use == BUFFER_IN_USE)
    {
      ed_ip = Ed_tx_buffer[i].ed_ip;
//      read_id = Ed_tx_buffer[i].ed_tx_info_read_id;
//      write_id = Ed_tx_buffer[i].ed_tx_info_write_id;
//      
//      /* 当缓冲区未读空时 */
//      while((read_id != write_id) || 
//            (Ed_tx_buffer[i].ed_tx_info[read_id].in_use == BUFFER_IN_USE))
//      {
//        /* 下一个read_id */
//        read_id = (read_id == TX_BUFFER_NUM_FOR_ONE_ED - 1) ? 0x00 : ++read_id;
//        
//        /* 处理对应的缓冲区 */
//        proc_tx_ed_info(ed_ip, &Ed_tx_buffer[i].ed_tx_info[read_id]);
//      }
//      
//      /* 更新read_id */
//      Ed_tx_buffer[i].ed_tx_info_read_id = read_id;
      
			for(avalible_index=0;avalible_index<TX_BUFFER_NUM_FOR_ONE_ED;avalible_index++)
			{
			  if(Ed_tx_buffer[i].ed_tx_info[avalible_index].in_use == BUFFER_IN_USE)
        {
				  proc_tx_ed_info(ed_ip, &Ed_tx_buffer[i].ed_tx_info[avalible_index]); 
				}
			}
			
      /* 使用完后立即释放这个节点占用的缓冲区 */
      Ed_tx_buffer[i].in_use = BUFFER_AVALIABLE;
    }
  }
	if(i == TX_BUFFER_NUM_FOR_ALL_ED)
	{
		start_emergency_dispatch_in_rtc = 1;
		emergency_dispatch_flag = 1; //缓冲区发送完毕之后，可以立刻执行一次对先前发送的数据进行重发。
	}
}

/**************************************************************************************************
 * @fn          proc_tx_ed_info
 *
 * @brief       处理指定节点的发送缓冲区中的信息.
 *
 * @param       ip          - 节点的IP地址
 *              pEd_tx_info - 节点的发送缓冲区
 *
 * @return      none
 **************************************************************************************************
 */
static void proc_tx_ed_info(uint16_t ip, gtway_frm_info_t *pEd_tx_info)
{
  struct ip_mac_id *p = NULL;
  uint8_t       inner_msg_len = pEd_tx_info->inner_msg_len;
  uint8_t       reply_request_flag = GET_REPLY_REQUEST_FLAG(pEd_tx_info->inner_msg);
  uint8_t       send_result;
  app_pkg_t     in_pkg;
  uint8_t       output_msg[256];
  uint8_t       output_msg_len;
  uint8_t       alarm_info[10];
  uint8_t       alarm_msg[MAX_APP_PAYLOAD];
  uint8_t       alarm_msg_len;

  if(!psearch_ip(&ip, &p))
  {
    /* IP Not in the table */
    goto BUILD_ALARM_FRM;
  }

  if (p->id == 0x00)
  {
    /* LID error */
    goto BUILD_ALARM_FRM;
  }

  /* 发送往ED的帧是否需要应答 */
  if(reply_request_flag == 0x40)  //需要应答
  {
    /* 发送前清除应答标志位.若不清除,ED将会回复应答帧 */
    CLEAN_REPLY_REQUEST_FLAG(pEd_tx_info->inner_msg);
    
    /* 清除应答标志位后应重新计算校验和 */
    pEd_tx_info->inner_msg[inner_msg_len - 2] = gen_crc(pEd_tx_info->inner_msg, (inner_msg_len - 2));
    
    /* 发往ED */
    send_result = send_net_pkg(p->id, pEd_tx_info->inner_msg, inner_msg_len);
    
    /* 发送成功,由网关回复应答帧 */
    if(send_result == 1)
    {
      change_gtway_frm_info_for_reply(pEd_tx_info);
      output_msg_len = bui_gtway_msg_from_gtway_frm_info(pEd_tx_info, output_msg);
      
      Send_gtway_msg_to_uart(output_msg, output_msg_len);
    }
  }
  else  //不需要应答
  {
    /* 直接发送 */
    //SMPL_SendOpt(p->id, pEd_tx_info->inner_msg, inner_msg_len, SMPL_TXOPTION_NONE);
    send_net_pkg(p->id, pEd_tx_info->inner_msg, inner_msg_len);
  }

  goto FREE_BUFFER;


BUILD_ALARM_FRM:  
  {
    /* 建立告警帧 */
    
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x04;
    alarm_info[2] = 0x03;
    alarm_info[3] = 0x05;

    bui_app_pkg((uint8_t *)pEd_tx_info->inner_msg, &in_pkg);
    alarm_msg_len = bui_pkg_alarm(&in_pkg, alarm_info, 4, alarm_msg);
    output_msg_len = Bui_bcast_gtway_msg_from_inner_msg(alarm_msg, alarm_msg_len, output_msg);
    Send_gtway_msg_to_uart(output_msg, output_msg_len);
		
  }

FREE_BUFFER:
  {
    pEd_tx_info->in_use = BUFFER_AVALIABLE;
    if (ed_tx_buffer_usage != 0)
    {
      ed_tx_buffer_usage--;
    }
  }
  
  return;
}

/**************************************************************************************************
 * @fn          change_gtway_frm_info_for_reply
 *
 * @brief       将指定节点的发送缓冲区中的帧改写为相应的应答帧.
 *
 * @param       pEd_tx_info   - 节点的发送缓冲区
 *
 * @return      none
 **************************************************************************************************
 */
static void change_gtway_frm_info_for_reply(gtway_frm_info_t *pEd_tx_info)
{
  uint8_t control_code, check_sum;

  if (pEd_tx_info == NULL)
  {
    return;
  }

  /* 修改控制码:应答帧,不需要应答,通信方式不变 */
  control_code = GET_CONTROL_CODE(pEd_tx_info->inner_msg);
  control_code &= 0x0c;
  control_code |= 0x02;
  SET_CONTROL_CODE(pEd_tx_info->inner_msg, control_code);

  /* 修改帧中的数据域长度:应答帧数据域长度为0 */
  SET_DATA_LEN(pEd_tx_info->inner_msg, 0);
  
  /* 重新计算应答帧的校验和 */
  check_sum = gen_crc(pEd_tx_info->inner_msg, 8);
  pEd_tx_info->inner_msg[8] = check_sum;

  /* 重新添加应答帧的帧尾 */
  pEd_tx_info->inner_msg[9] = INNER_FRAME_TAIL;
  
  /* 重新设置缓冲区中的帧长度 */
  pEd_tx_info->inner_msg_len = GET_FRAME_LEN(pEd_tx_info->inner_msg);
}


/**************************************************************************************************
 * @fn          bui_gtway_msg_from_gtway_frm_info
 *
 * @brief       将指定节点的发送缓冲区中的帧构建为相应的网外通信帧.
 *
 * @param       pEd_tx_info   - 节点的发送缓冲区
 *              pOut_msg      - 输出的网外通信帧
 *
 * @return      网外通信帧长度
 **************************************************************************************************
 */
static uint8_t bui_gtway_msg_from_gtway_frm_info(const gtway_frm_info_t *pEd_tx_info, uint8_t *pOut_msg)
{
  uint16_t control_code, frame_count, date_len;
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  if (pEd_tx_info == NULL)
  {
    return 0;
  }

  control_code = Reverse_trans_direction(pEd_tx_info->ctr_code);
  frame_count = pEd_tx_info->pid_num;
  date_len = pEd_tx_info->inner_msg_len;

  pOut_msg[0] = GTWAY_FRAME_HEAD;
  pOut_msg[1] = 0x00; //Reserve
  pOut_msg[2] = (uint8_t)(control_code >> 8); //Control code(MSB)
  pOut_msg[3] = (uint8_t)(control_code);      //Control code(LSB)
  pOut_msg[4] = pMyGtwayAddr->type[1]; //Gateway Type
  pOut_msg[5] = pMyGtwayAddr->type[0];
  pOut_msg[6] = pMyGtwayAddr->addr[3]; //Gateway Addr
  pOut_msg[7] = pMyGtwayAddr->addr[2];
  pOut_msg[8] = pMyGtwayAddr->addr[1];
  pOut_msg[9] = pMyGtwayAddr->addr[0];
  memcpy(&pOut_msg[10], sGtwayServerIP.addr, 4);      //Gateway Server IP
  memcpy(&pOut_msg[14], pEd_tx_info->mobile_addr, 6); //Mobile Phone MAC
  pOut_msg[20] = (uint8_t)(frame_count >> 8);   //Frame counter(MSB)
  pOut_msg[21] = (uint8_t)(frame_count);        //Frame counter(LSB)
  pOut_msg[22] = (uint8_t)(date_len >> 8);      //Data Length(MSB)
  pOut_msg[23] = (uint8_t)(date_len);           //Data Length(LSB)
  memcpy(&pOut_msg[24], pEd_tx_info->inner_msg, date_len); //Data
  pOut_msg[24 + date_len] = gen_crc(pOut_msg, 24 + date_len); //Check sum
  pOut_msg[25 + date_len] = GTWAY_FRAME_TAIL;

  return (26 + date_len);
}

/**************************************************************************************************
 * @fn          Modify_ip_in_tx_ed_buffer
 *
 * @brief       修改用于标识不同节点发送缓冲区的IP地址.若确实不存在此节点或尚未为此节点分配缓冲区,
 *              返回BUFFER_IP_MODIFY_FAIL.此函数参数不允许使用0x0000或0xffff作为IP地址.
 *
 * @param       current_ed_ip   - 当前发送缓冲区中的节点IP
 *              modified_ed_ip  - 修改后的节点IP
 *
 * @return      BUFFER_SUCCESS        - 修改成功
 *              BUFFER_IP_MODIFY_FAIL - 修改失败,未对缓冲区进行任何处理
 **************************************************************************************************
 */
buffer_result_t Modify_ip_in_tx_ed_buffer(uint16_t current_ed_ip, uint16_t modified_ed_ip)
{
  uint8_t i;

  if (current_ed_ip == 0x0000 || current_ed_ip == 0xffff)
  {
    return BUFFER_IP_MODIFY_FAIL;
  }

  if (modified_ed_ip == 0x0000 || modified_ed_ip == 0xffff)
  {
    return BUFFER_IP_MODIFY_FAIL;
  }

  for (i = 0; i < TX_BUFFER_NUM_FOR_ALL_ED; ++i)
  {
    if ((Ed_tx_buffer[i].in_use == BUFFER_IN_USE) \
      && (Ed_tx_buffer[i].ed_ip == current_ed_ip))
    {
      Ed_tx_buffer[i].ed_ip = modified_ed_ip;

      return BUFFER_SUCCESS;
    }
  }

  return BUFFER_IP_MODIFY_FAIL;
}

/**************************************************************************************************
 * @fn          Free_tx_ed_buffer
 *
 * @brief       释放某一节点的发送缓冲区,缓冲区被释放后可以被其他节点使用.
 *
 * @param       ed_ip   - 标识要释放缓冲区的节点IP
 *
 * @return      BUFFER_SUCCESS    - 释放缓冲区
 *              BUFFER_FREE_FAIL  - 释放缓冲区失败,未对缓冲区进行任何处理
 **************************************************************************************************
 */
buffer_result_t Free_tx_ed_buffer(uint16_t ed_ip)
{
  uint8_t i, j;

  if (ed_ip == 0x0000 || ed_ip == 0xffff)
  {
    return BUFFER_FREE_FAIL;
  }

  for (i = 0; i < TX_BUFFER_NUM_FOR_ALL_ED; ++i)
  {
    if ((Ed_tx_buffer[i].in_use == BUFFER_IN_USE) \
      && (Ed_tx_buffer[i].ed_ip == ed_ip))
    {
      Ed_tx_buffer[i].ed_ip = 0x0000;
      Ed_tx_buffer[i].in_use = BUFFER_AVALIABLE;

      for (j = 0; j < TX_BUFFER_NUM_FOR_ONE_ED; ++j)
      {
        if (Ed_tx_buffer[i].ed_tx_info[j].in_use == BUFFER_IN_USE)
        {
          Ed_tx_buffer[i].ed_tx_info[j].in_use = BUFFER_AVALIABLE;
          if (ed_tx_buffer_usage != 0)
          {
            ed_tx_buffer_usage--;
          }
        }
      }
      
      Ed_tx_buffer[i].ed_tx_info_read_id = 0x00;
      Ed_tx_buffer[i].ed_tx_info_write_id = 0x00;

      return BUFFER_SUCCESS;
    }
  }

  return BUFFER_FREE_FAIL;
}


/**************************************************************************************************
 * @fn          isReadyToWriteGroupFrm
 *
 * @brief       判断是否能写入组播帧.
 *
 * @param       ed_ip   - 标识要释放缓冲区的节点IP
 *
 * @return      BUFFER_SUCCESS    - 释放缓冲区
 *              BUFFER_FREE_FAIL  - 释放缓冲区失败,未对缓冲区进行任何处理
 **************************************************************************************************
 */
uint8_t isReadyToWriteGroupFrm(void)
{
  uint8_t i = 0;
  uint8_t valid_buffer_num = 0;
  
  for(i=0;i<TX_BUFFER_NUM_FOR_ALL_ED;i++)
  {
    if(Ed_tx_buffer[i].in_use == BUFFER_AVALIABLE)
      valid_buffer_num++;
  }
  
  if(valid_buffer_num >= 10)
    return 1;
  else
    return 0;
}
/**************************************************************************************************
 */

/**************************************************************************************************
 * @fn          Write_usart1_data_txbuffer
 *
 * @brief       将串口1的数据写入发送缓冲区
 *
 * @param       pIn_gtway_pkg - 接收到的68帧信息
 *              pOut_msg      - 输出的返回帧(如告警帧)
 *              pOut_len      - 输出的返回帧长度
 *
 * @return      none
 **************************************************************************************************
 */
void Write_usart1_data_txbuffer(uint8_t * pIn_pkg, uint8_t *pOut_msg, uint8_t* pOut_len)
{
//  uint8_t           inner_frm_head = GET_FRAME_HEAD(pIn_gtway_pkg->msg);
  uint16_t          ed_ip;
//  uint8_t           mult_ip_count;
//  uint16_t          mult_ip_group[IP_GROUP_MAX_NUM];
//  uint8_t           i;
  ed_tx_buffer_t    *pEd_tx_buffer;
  gtway_frm_info_t  *pEd_tx_info;
//  app_pkg_t         in_pkg;
//  uint8_t           alarm_info[10];

  switch(0x68)
  {
    case 0x68:  /* 与节点通信:单播帧头 */
    {
      ed_ip = ((pIn_pkg[2]<<8)|pIn_pkg[3]);
      
      pEd_tx_buffer = search_ip_in_ed_tx_buffer(ed_ip);
      if (pEd_tx_buffer == NULL)
      {
        pEd_tx_buffer = add_new_ed_in_ed_tx_buffer(ed_ip);
        if (pEd_tx_buffer == NULL)
        {
          /* 所有节点发送缓存区已经分配完 */
          break;
        }
      }
      
      pEd_tx_info = find_a_avaliable_ed_tx_info(pEd_tx_buffer);
      if (pEd_tx_info == NULL)
      {
        /* 此节点无可用的缓存区 */
        break;
      }

//      write_a_ed_tx_info(pEd_tx_info, pIn_gtway_pkg);
        write_usart1_data_info(pEd_tx_info,pIn_pkg);
      break;
    }
    default:
    {
      /* 错误的帧头 */
    }
  }
  
  return;
}

/**************************************************************************************************
 * @fn          write_a_ed_tx_info
 *
 * @brief       将接收到的串口1数据写入缓冲区
 *
 * @param       pEd_tx_info   - 节点的发送缓冲区
 *              pIn_gtway_pkg - 69帧结构
 *
 * @return      none
 **************************************************************************************************
 */
static void write_usart1_data_info(gtway_frm_info_t *pEd_tx_info, uint8_t *pIn_pkg)
{
//  uint8_t i;
  uint8_t inner_msg_len = 10+pIn_pkg[7];
  
  pEd_tx_info->ctr_code = 0x0E;

//  for (i = 0; i < 4; ++i)
//  {
//    pEd_tx_info->server_addr[i] = pIn_gtway_pkg->server_addr[i];
//  }

//  for (i = 0; i < 6; ++i)
//  {
//    pEd_tx_info->mobile_addr[i] = pIn_gtway_pkg->mobile_addr[i];
//  }

  pEd_tx_info->pid_num = pIn_pkg[6];
  
  pEd_tx_info->inner_msg_len = inner_msg_len;

  memcpy(pEd_tx_info->inner_msg, pIn_pkg, inner_msg_len);

  pEd_tx_info->in_use = BUFFER_IN_USE;
  
  ed_tx_buffer_usage++;
}


