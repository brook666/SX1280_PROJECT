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
 * @brief       �ı�����ͨ��֡�Ŀ������еĴ��䷽��λ.��Ŀ�ĵ����ص�ͨ�ŷ����Ϊ��Ӧ�������ط����ķ�
 *              ��,���ഫ�䷽��Ŀ����벻��.Ӧ������λ.
 *
 * @param       pMsg - Ҫ�ı�Ŀ�����
 *
 * @return      �ı��Ŀ�����
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
 * @brief       ���ڲ�֡��䵽Ҫ���������ͨ��֡��.
 *
 * @param       pIn_pkg         - ���������ͨ��֡
 *              pInner_msg      - �ڲ�֡
 *              inner_msg_len   - �ڲ�֡����
 *              pOut_msg        - ���������ͨ��֡
 *
 * @return      ���������ͨ��֡����
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
  
  /* �������ÿ�����Ĵ��䷽��λ */
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
 * @brief       ������ͨ��֡��������Ӧ��Ӧ��֡.
 *
 * @param       pIn_pkg       - ���������ͨ��֡,��ΪNULL�򲻰��ڲ�֡
 *              pOut_msg      - ���������ͨ��֡
 *
 * @return      ���������ͨ��֡����
 **************************************************************************************************
 */
uint8_t bui_ed_ack(ed_pkg_t *pIn_pkg, uint8_t *pOut_msg)
{
  pIn_pkg->ctr_code &= ~0x4000;  //���Ӧ������
  pIn_pkg->ctr_code |= 0x2000;   //��λӦ��֡��־
  
  return bui_outnet_frm(pIn_pkg, NULL, 0, pOut_msg);
}


/**************************************************************************************************
 * @fn          Send_ed_msg_to_uart
 *
 * @brief       �򴮿ڷ�������ͨ��֡��ͨ�����������е�ͨ��λ����֡������ȷ�Ĵ���.
 *
 * @param       pMsg    - Ҫ���͵�����ͨ��֡���������֡��ʽ
 *              msg_len - ����ͨ��֡����
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
			/*���͸澯֡�������������ڷ�����ͳ��*/
//			TCP_Send_Filter(pMsg, msg_len);//Send to wifi	
		}
	}
}

/**************************************************************************************************
 * @fn          bui_ed_alarm
 *
 * @brief       ������ͨ��֡��������Ӧ�ĸ澯֡.
 *
 * @param       pIn_pkg       - ���������ͨ��֡,��ΪNULL�򲻰��ڲ�֡
 *              pErr_info     - �澯��Ϣ
 *              err_info_len  - �澯��Ϣ����
 *              pOut_msg      - ���������ͨ��֡
 *
 * @return      ���������ͨ��֡����
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
    /* �����ڲ��澯֡ */
    memcpy(inner_frame, pIn_pkg->msg, 7);              //����ԭʼ֡
    inner_frame[1] |= 0x03;                            //֡���͸�Ϊ�澯֡
    inner_frame[7] = err_info_len;                     //�澯��Ϣ����
    memcpy(&inner_frame[8], pErr_info, err_info_len);  //���澯��Ϣ
    inner_frame[8+err_info_len] = gen_crc(inner_frame, 8+err_info_len);//����У���
    inner_frame[9+err_info_len] = 0x16;                //֡β
    inner_frame_len = 10 + err_info_len;               //�ڲ��澯֡����
    
    return bui_outnet_frm(pIn_pkg, inner_frame, inner_frame_len, pOut_msg);
  } 
}

/**************************************************************************************************
 * @fn          Bui_bcast_gtway_msg_from_inner_msg
 *
 * @brief       ���ڲ�֡����������ͨ�ŵĹ㲥֡.
 *
 * @param       pInner_msg      - �����ڲ�֡
 *              inner_msg_len   - �����ڲ�֡����
 *              pOut_msg        - �������ͨ�ŵĹ㲥֡
 *
 * @return      �������ͨ�ŵĹ㲥֡����
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
 * @brief       ����68��Ӧ��
 *
 * @param       pIn_pkg      - �����ڲ�֡
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

  if(reply_request_flag == 0x40)   //���ʼ�������֡�����˴���  (comm_type == 0x08)&&(
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
 * @brief       ��ȡ�ڵ��ַ������
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      ָ�����ص�ַ�ṹ���ָ��
 ***************************************************************************
 */ 
ed_addr_t * get_ed_addr(void)
{
   return (&sMyEdAddr);
}
