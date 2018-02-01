#include "ed_frm_proc.h"
#include "bui_pkg_fun.h"
#include "smpl_config.h"
#include "firmware_info_proc.h"
#include <string.h>

static ed_addr_t sMyEdAddr;
addr_t sEdServerIP = ED_SERVER_IP;
extern uint8_t sSwsnMacAddr[MAC_ADDR_SIZE];
void bui_ed_pkg(uint8_t *pIn_msg, ed_pkg_t *pOut_pkg)
{
  uint16_t data_len;

  pOut_pkg->frm_head = pIn_msg[0];                                //Frame head
  pOut_pkg->reserve1 = pIn_msg[1]; 
  pOut_pkg->ctr_code   = ((uint16_t)pIn_msg[2]<<8) | pIn_msg[3];  //Control code
  pOut_pkg->ed_type = ((uint16_t)pIn_msg[4]<<8) | pIn_msg[5];  //Gateway type
  memcpy(pOut_pkg->ed_addr, &pIn_msg[6], 4);                   //Gateway Addr
  memcpy(pOut_pkg->server_addr, &pIn_msg[10], 4);                 //Gateway Server IP
  memcpy(pOut_pkg->mobile_addr, &pIn_msg[14], 6);                 //Mobile Phone MAC
  pOut_pkg->pid_num  = ((uint16_t)pIn_msg[20]<<8) | pIn_msg[21];  //Frame counter
  data_len = ((uint16_t)pIn_msg[22]<<8) | pIn_msg[23];            //Data Length
  pOut_pkg->data_len = data_len;
  memcpy(pOut_pkg->msg, &pIn_msg[24], data_len);                  //Data
  pOut_pkg->cs = pIn_msg[24+data_len];                            //Check sum
  pOut_pkg->frm_end = pIn_msg[25+data_len];                       //Frame tail
}

/**************************************************************************************************
 * @fn          Reverse_trans_direction
 *
 * @brief       改变网外通信帧的控制码中的传输方向位.将目的到网关的通信方向改为相应的由网关发出的方
 *              向,其余传输方向的控制码不变.应答请求复位.
 *
 * @param       pMsg - 要改变的控制码
 *
 * @return      改变后的控制码
 **************************************************************************************************
 */
uint16_t Reverse_trans_direction(uint16_t control_code)
{
  uint8_t trans_direc;

  trans_direc = (control_code >> 8) & 0x0f;
  switch(trans_direc)
  {
    case 0x04: /* Go down */
    case 0x06: /* Go down */
	  case 0x12: /* Go down */
	  case 0x11: /* Go down */
    case 0x0c: control_code += 0x0100; break;
    case 0x03: control_code -= 0x0100; break;
		case 0x0A: control_code += 0x0100; break;
    default: break;/* Do nothing */
  }
  control_code &= 0xbfff; //No need reply

  return control_code;
}


/**************************************************************************************************
 * @fn          bui_outnet_frm
 *
 * @brief       将内部帧填充到要输出的网外通信帧中.
 *
 * @param       pIn_pkg         - 输入的网外通信帧
 *              pInner_msg      - 内部帧
 *              inner_msg_len   - 内部帧长度
 *              pOut_msg        - 输出的网外通信帧
 *
 * @return      输出的网外通信帧长度
 **************************************************************************************************
 */
uint16_t bui_outnet_frm(ed_pkg_t *pIn_pkg, uint8_t *pInner_msg, uint16_t inner_msg_len, uint8_t *pOut_msg)
{
  uint16_t ctr_code;
  ed_addr_t * pMyEdAddr = get_ed_addr();
  
  if (pIn_pkg == NULL || pOut_msg == NULL)
  {
    return 0;
  }
  
  /* 重新设置控制码的传输方向位 */
  ctr_code = Reverse_trans_direction(pIn_pkg->ctr_code);

  pOut_msg[0] = 0x69;
  pOut_msg[1] = 0x00; //Reserve
  pOut_msg[2] = (uint8_t)(ctr_code >> 8); //Control code(MSB)
  pOut_msg[3] = (uint8_t)(ctr_code);      //Control code(LSB)
  pOut_msg[4] = 0x00; //Gateway Type
  pOut_msg[5] = 0x01;
  pOut_msg[6] = sSwsnMacAddr[0]; //Gateway Addr
  pOut_msg[7] = sSwsnMacAddr[1];
  pOut_msg[8] = sSwsnMacAddr[2];
  pOut_msg[9] = sSwsnMacAddr[3];
  memcpy(&pOut_msg[10], sEdServerIP.addr, 4);  //Gateway Server IP
  memcpy(&pOut_msg[14], pIn_pkg->mobile_addr, 6); //Mobile Phone MAC
  pOut_msg[20] = (uint8_t)(pIn_pkg->pid_num>>8);  //Frame counter(MSB)
  pOut_msg[21] = (uint8_t)(pIn_pkg->pid_num);     //Frame counter(LSB)
  pOut_msg[22] = (uint8_t)(inner_msg_len >> 8);   //Data Length(MSB)
  pOut_msg[23] = (uint8_t)(inner_msg_len);        //Data Length(LSB)
  memcpy(&pOut_msg[24], pInner_msg, inner_msg_len); //Data
  pOut_msg[24 + inner_msg_len] = gen_crc(pOut_msg, 24 + inner_msg_len); //Check sum
  pOut_msg[25 + inner_msg_len] = 0x17;

  return (26 + inner_msg_len);
}




/**************************************************************************************************
 * @fn          bui_gtway_ack
 *
 * @brief       将网外通信帧构建成相应的应答帧.
 *
 * @param       pIn_pkg       - 输入的网外通信帧,若为NULL则不包内部帧
 *              pOut_msg      - 输出的网外通信帧
 *
 * @return      输出的网外通信帧长度
 **************************************************************************************************
 */
uint8_t bui_ed_ack(ed_pkg_t *pIn_pkg, uint8_t *pOut_msg)
{
  pIn_pkg->ctr_code &= ~0x4000;  //清除应答请求
  pIn_pkg->ctr_code |= 0x2000;   //置位应答帧标志
  
  return bui_outnet_frm(pIn_pkg, NULL, 0, pOut_msg);
}


/**************************************************************************************************
 * @fn          Send_ed_msg_to_uart
 *
 * @brief       向串口发送网外通信帧，通过检查控制码中的通信位，将帧发往正确的串口.
 *
 * @param       pMsg    - 要发送的网外通信帧，必须符合帧格式
 *              msg_len - 网外通信帧长度
 *
 * @return      none
 **************************************************************************************************
 */
void Send_ed_msg_to_uart(uint8_t *pMsg, uint16_t msg_len)
{
  uint8_t trans_direc;

  trans_direc = GET_ED_FRM_TRANS_DIRECTION(pMsg);
	
  if (trans_direc == 0x0b)
  {
//    put_multi_hex_uart1(pMsg, msg_len);//Send to computer
		  put_multi_hex_uart2(pMsg, msg_len);
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"USART SEND"RTT_CTRL_RESET"\n");
	SEGGER_RTT_put_multi_char((uint8_t *)pMsg,msg_len);
#endif 
  }
  else if(trans_direc == 0x0f)
  {
//    put_multi_hex_uart1(pMsg, msg_len);//Send to computer
  	  put_multi_hex_uart2(pMsg, msg_len);
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"USART SEND"RTT_CTRL_RESET"\n");
	SEGGER_RTT_put_multi_char((uint8_t *)pMsg,msg_len);
#endif 
//    TCP_Send_Filter(pMsg, msg_len);//Send to wifi
  }
  else
  {
//    TCP_Send_Filter(pMsg, msg_len);//Send to wifi
  }
	if((pMsg[25]&0x03) == 0x03)
	{
		if(pMsg[2] != 0x02)
		{
			pMsg[2] = 0x02;
			pMsg[msg_len-2] = gen_crc(pMsg,msg_len-2);
			/*发送告警帧给服务器，便于服务器统计*/
//			TCP_Send_Filter(pMsg, msg_len);//Send to wifi	
		}
	}
}

/**************************************************************************************************
 * @fn          bui_ed_alarm
 *
 * @brief       将网外通信帧构建成相应的告警帧.
 *
 * @param       pIn_pkg       - 输入的网外通信帧,若为NULL则不包内部帧
 *              pErr_info     - 告警信息
 *              err_info_len  - 告警信息长度
 *              pOut_msg      - 输出的网外通信帧
 *
 * @return      输出的网外通信帧长度
 **************************************************************************************************
 */
uint8_t bui_ed_alarm(ed_pkg_t *pIn_pkg, uint8_t *pErr_info, uint8_t err_info_len, uint8_t *pOut_msg)
{
  uint8_t inner_frame[20];
  uint8_t inner_frame_len;

  if(pIn_pkg == NULL)
  {
    return Bui_bcast_ed_msg_from_inner_msg(pErr_info, err_info_len, pOut_msg);
  }
  else
  {
    /* 生成内部告警帧 */
    memcpy(inner_frame, pIn_pkg->msg, 7);              //复制原始帧
    inner_frame[1] |= 0x03;                            //帧类型改为告警帧
    inner_frame[7] = err_info_len;                     //告警信息长度
    memcpy(&inner_frame[8], pErr_info, err_info_len);  //填充告警信息
    inner_frame[8+err_info_len] = gen_crc(inner_frame, 8+err_info_len);//计算校验和
    inner_frame[9+err_info_len] = 0x16;                //帧尾
    inner_frame_len = 10 + err_info_len;               //内部告警帧长度
    
    return bui_outnet_frm(pIn_pkg, inner_frame, inner_frame_len, pOut_msg);
  } 
}

/**************************************************************************************************
 * @fn          Bui_bcast_gtway_msg_from_inner_msg
 *
 * @brief       将内部帧构建成网外通信的广播帧.
 *
 * @param       pInner_msg      - 输入内部帧
 *              inner_msg_len   - 输入内部帧长度
 *              pOut_msg        - 输出网外通信的广播帧
 *
 * @return      输出网外通信的广播帧长度
 **************************************************************************************************
 */
uint8_t Bui_bcast_ed_msg_from_inner_msg(const uint8_t *pInner_msg, uint8_t inner_msg_len, uint8_t *pOut_msg)
{
  if (pInner_msg == NULL)
  {
    return 0;
  }
  
  pOut_msg[0] = 0x69;
  pOut_msg[1] = 0x00; //Reserve
  pOut_msg[2] = 0x0f; //Direction:broadcast
  pOut_msg[3] = 0x00; //Reserve
  pOut_msg[4] = 0x00; //Gateway Type
  pOut_msg[5] = 0x01;
  pOut_msg[6] = sSwsnMacAddr[0]; //Gateway Addr
  pOut_msg[7] = sSwsnMacAddr[1];
  pOut_msg[8] = sSwsnMacAddr[2];
  pOut_msg[9] = sSwsnMacAddr[3];
  memcpy(&pOut_msg[10], sEdServerIP.addr, 4);  //Gateway Server IP
  memset(&pOut_msg[14], 0xff, 6); //Mobile Phone MAC
  pOut_msg[20] = 0x00;  //Frame counter(MSB)
  pOut_msg[21] = 0x00;  //Frame counter(LSB)
  pOut_msg[22] = (uint8_t)(inner_msg_len >> 8); //Data Length(MSB)
  pOut_msg[23] = (uint8_t)(inner_msg_len);      //Data Length(LSB)
  memcpy(&pOut_msg[24], pInner_msg, inner_msg_len); //Data
  pOut_msg[24 + inner_msg_len] = gen_crc(pOut_msg, 24 + inner_msg_len); //Check sum
  pOut_msg[25 + inner_msg_len] = 0x17;

  return (26 + inner_msg_len);
}

/**************************************************************************************************
 * @fn          pro_gtway_data_ack
 *
 * @brief       处理68的应答
 *
 * @param       pIn_pkg      - 输入内部帧
 *
 * @return      void
 **************************************************************************************************
 */
void pro_ed_data_ack(ed_pkg_t *pIn_pkg)
{
	app_pkg_t in_pkg;
//  app_77_pkg_t in_pkg_77;
  uint8_t ack_msg[20];
  uint8_t out_msg[50];
	uint8_t ack_msg_len = 0;
  uint8_t out_msg_len = 0;
	uint8_t reply_request_flag;
	reply_request_flag = GET_REPLY_REQUEST_FLAG(pIn_pkg->msg); 

  if(reply_request_flag == 0x40)   //访问集中器的帧才做此处理  (comm_type == 0x08)&&(
  {
    if(pIn_pkg->msg[0] == 0x68)
    {
//      if(comm_type == 0x08)
//      {
        bui_app_pkg((uint8_t *)pIn_pkg->msg, &in_pkg);
        ack_msg_len = bui_pkg_ack(&in_pkg,ack_msg);
        out_msg_len = bui_outnet_frm(pIn_pkg,ack_msg,ack_msg_len,out_msg);
//      }
    }
    else
    {
    }
    Send_ed_msg_to_uart(out_msg,out_msg_len);
  }
  else
  {
  }
}

/***************************************************************************
 * @fn          get_ed_addr
 *     
 * @brief       获取节点地址和类型
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      指向网关地址结构体的指针
 ***************************************************************************
 */ 
ed_addr_t * get_ed_addr(void)
{
   return (&sMyEdAddr);
}
