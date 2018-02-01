/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   gtway_frm_proc.h
// Description: ��������ͨ��֡.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-23
// History:     2014-10-23  Leidi ��ӷ�����IP��ַ��ȫ�ֱ���.
//                                ��������Send_gtway_msg_to_uart().
//                                ��������Reverse_trans_direction().
//                                ��������Bui_bcast_gtway_msg_from_inner_msg().
//              2014-10-24  Leidi ɾ���ⲿȫ�ֱ���resvr_pkg[MAX_RESVR_PKG]��resvr_pkg_num.
//                                ɾ������select_resvr_pkg().
//                                ɾ������rebuild_69_frm().
//                                ɾ������rebuild_69_alarm().
//                                ɾ������decrement_resvr_pkg_num().
//              2015-01-21  Leidi ɾ������bui_gtway_bcast().
//                                �ع�����bui_gtway_alarm().
//              2015-01-21  Leidi �ع�����bui_gtway_ack().
//                                �ع�����bui_outnet_frm().
//              2015-01-28  Leidi ɾ������get_innet_frm().
//              2015-01-29  Leidi �ع�����bui_gtway_pkg().
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "gtway_frm_proc.h"
#include "bui_pkg_fun.h"
#include "smpl_config.h"
#include "firmware_info_proc.h"
#include "tcp_demo.h"
#include <string.h>

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ���������ӵķ�����IP��ַ */
addr_t sGtwayServerIP = GTWAY_SERVER_IP;

/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */


/**************************************************************************************************
 * @fn          bui_gtway_pkg
 *
 * @brief       ������ͨ��֡�����װΪ����ͨ��֡�ṹ��.
 *
 * @param       pIn_msg   - ���������ͨ��֡����
 *              pOut_pkg  - ���������ͨ��֡�ṹ��
 *
 * @return      none
 **************************************************************************************************
 */
void bui_gtway_pkg(uint8_t *pIn_msg, gtway_pkg_t *pOut_pkg)
{
  uint16_t data_len;

  pOut_pkg->frm_head = pIn_msg[0];                                //Frame head
  pOut_pkg->reserve1 = pIn_msg[1]; 
  pOut_pkg->ctr_code   = ((uint16_t)pIn_msg[2]<<8) | pIn_msg[3];  //Control code
  pOut_pkg->gtway_type = ((uint16_t)pIn_msg[4]<<8) | pIn_msg[5];  //Gateway type
  memcpy(pOut_pkg->gtway_addr, &pIn_msg[6], 4);                   //Gateway Addr
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
uint16_t bui_outnet_frm(gtway_pkg_t *pIn_pkg, uint8_t *pInner_msg, uint16_t inner_msg_len, uint8_t *pOut_msg)
{
  uint16_t ctr_code;
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  
  if (pIn_pkg == NULL || pOut_msg == NULL)
  {
    return 0;
  }
  
  /* �������ÿ�����Ĵ��䷽��λ */
  ctr_code = Reverse_trans_direction(pIn_pkg->ctr_code);

  pOut_msg[0] = GTWAY_FRAME_HEAD;
  pOut_msg[1] = 0x00; //Reserve
  pOut_msg[2] = (uint8_t)(ctr_code >> 8); //Control code(MSB)
  pOut_msg[3] = (uint8_t)(ctr_code);      //Control code(LSB)
  pOut_msg[4] = pMyGtwayAddr->type[1]; //Gateway Type
  pOut_msg[5] = pMyGtwayAddr->type[0];
  pOut_msg[6] = pMyGtwayAddr->addr[3]; //Gateway Addr
  pOut_msg[7] = pMyGtwayAddr->addr[2];
  pOut_msg[8] = pMyGtwayAddr->addr[1];
  pOut_msg[9] = pMyGtwayAddr->addr[0];
  memcpy(&pOut_msg[10], sGtwayServerIP.addr, 4);  //Gateway Server IP
  memcpy(&pOut_msg[14], pIn_pkg->mobile_addr, 6); //Mobile Phone MAC
  pOut_msg[20] = (uint8_t)(pIn_pkg->pid_num>>8);  //Frame counter(MSB)
  pOut_msg[21] = (uint8_t)(pIn_pkg->pid_num);     //Frame counter(LSB)
  pOut_msg[22] = (uint8_t)(inner_msg_len >> 8);   //Data Length(MSB)
  pOut_msg[23] = (uint8_t)(inner_msg_len);        //Data Length(LSB)
  memcpy(&pOut_msg[24], pInner_msg, inner_msg_len); //Data
  pOut_msg[24 + inner_msg_len] = gen_crc(pOut_msg, 24 + inner_msg_len); //Check sum
  pOut_msg[25 + inner_msg_len] = GTWAY_FRAME_TAIL;

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
uint8_t bui_gtway_ack(gtway_pkg_t *pIn_pkg, uint8_t *pOut_msg)
{
  pIn_pkg->ctr_code &= ~0x4000;  //���Ӧ������
  pIn_pkg->ctr_code |= 0x2000;   //��λӦ��֡��־
  
  return bui_outnet_frm(pIn_pkg, NULL, 0, pOut_msg);
}

/**************************************************************************************************
 * @fn          bui_gtway_alarm
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
uint8_t bui_gtway_alarm(gtway_pkg_t *pIn_pkg, uint8_t *pErr_info, uint8_t err_info_len, uint8_t *pOut_msg)
{
  uint8_t inner_frame[20];
  uint8_t inner_frame_len;

  if(pIn_pkg == NULL)
  {
    return Bui_bcast_gtway_msg_from_inner_msg(pErr_info, err_info_len, pOut_msg);
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
 * @fn          Send_gtway_msg_to_uart
 *
 * @brief       �򴮿ڷ�������ͨ��֡��ͨ�����������е�ͨ��λ����֡������ȷ�Ĵ���.
 *
 * @param       pMsg    - Ҫ���͵�����ͨ��֡���������֡��ʽ
 *              msg_len - ����ͨ��֡����
 *
 * @return      none
 **************************************************************************************************
 */
void Send_gtway_msg_to_uart(uint8_t *pMsg, uint16_t msg_len)
{
  uint8_t trans_direc;

  trans_direc = GET_GTWAT_FRM_TRANS_DIRECTION(pMsg);

  if (trans_direc == 0x0b)
  {
//    put_multi_hex_uart1(pMsg, msg_len);//Send to computer
  }
  else if(trans_direc == 0x0f)
  {
//    put_multi_hex_uart1(pMsg, msg_len);//Send to computer
    TCP_Send_Filter(pMsg, msg_len);//Send to wifi
  }
  else
  {
    TCP_Send_Filter(pMsg, msg_len);//Send to wifi
  }
	if((pMsg[25]&0x03) == 0x03)
	{
		if(pMsg[2] != 0x02)
		{
			pMsg[2] = 0x02;
			pMsg[msg_len-2] = gen_crc(pMsg,msg_len-2);
			/*���͸澯֡�������������ڷ�����ͳ��*/
			TCP_Send_Filter(pMsg, msg_len);//Send to wifi	
		}
	}
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
    default: break;/* Do nothing */
  }
  control_code &= 0xbfff; //No need reply

  return control_code;
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
uint8_t Bui_bcast_gtway_msg_from_inner_msg(const uint8_t *pInner_msg, uint8_t inner_msg_len, uint8_t *pOut_msg)
{
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  if (pInner_msg == NULL)
  {
    return 0;
  }
  
  pOut_msg[0] = GTWAY_FRAME_HEAD;
  pOut_msg[1] = 0x00; //Reserve
  pOut_msg[2] = 0x0f; //Direction:broadcast
  pOut_msg[3] = 0x00; //Reserve
  pOut_msg[4] = pMyGtwayAddr->type[1]; //Gateway Type
  pOut_msg[5] = pMyGtwayAddr->type[0];
  pOut_msg[6] = pMyGtwayAddr->addr[3]; //Gateway Addr
  pOut_msg[7] = pMyGtwayAddr->addr[2];
  pOut_msg[8] = pMyGtwayAddr->addr[1];
  pOut_msg[9] = pMyGtwayAddr->addr[0];
  memcpy(&pOut_msg[10], sGtwayServerIP.addr, 4);  //Gateway Server IP
  memset(&pOut_msg[14], 0xff, 6); //Mobile Phone MAC
  pOut_msg[20] = 0x00;  //Frame counter(MSB)
  pOut_msg[21] = 0x00;  //Frame counter(LSB)
  pOut_msg[22] = (uint8_t)(inner_msg_len >> 8); //Data Length(MSB)
  pOut_msg[23] = (uint8_t)(inner_msg_len);      //Data Length(LSB)
  memcpy(&pOut_msg[24], pInner_msg, inner_msg_len); //Data
  pOut_msg[24 + inner_msg_len] = gen_crc(pOut_msg, 24 + inner_msg_len); //Check sum
  pOut_msg[25 + inner_msg_len] = GTWAY_FRAME_TAIL;

  return (26 + inner_msg_len);
}


/**************************************************************************************************
 * @fn          pro_gtway_data_ack
 *
 * @brief       ����68��88��77֡��Ӧ��,67֡���⣬ֻ�Է��ʼ�������֡���д���
 *
 * @param       pIn_pkg      - �����ڲ�֡
 *
 * @return      void
 **************************************************************************************************
 */
void pro_gtway_data_ack(gtway_pkg_t *pIn_pkg)
{
  uint8_t reply_request_flag = GET_REPLY_REQUEST_FLAG(pIn_pkg->msg); 
  uint8_t comm_type = GET_COMM_TYPE(pIn_pkg->msg);
  app_pkg_t in_pkg;
  app_77_pkg_t in_pkg_77;
  uint8_t ack_msg[20];
  uint8_t out_msg[50];
  uint8_t ack_msg_len = 0;
  uint8_t out_msg_len = 0;

  if(reply_request_flag == 0x40)   //���ʼ�������֡�����˴���  (comm_type == 0x08)&&(
  {
    if(pIn_pkg->msg[0] == 0x67)
    {
      return;
    }
    else if(pIn_pkg->msg[0] == 0x68)
    {
      if(comm_type == 0x08)
      {
        bui_app_pkg((uint8_t *)pIn_pkg->msg, &in_pkg);
        ack_msg_len = bui_pkg_ack(&in_pkg,ack_msg);
        out_msg_len = bui_outnet_frm(pIn_pkg,ack_msg,ack_msg_len,out_msg);
      }
    }
    else if(pIn_pkg->msg[0] == 0x88)
    {
      bui_app_pkg((uint8_t *)pIn_pkg->msg, &in_pkg);
      ack_msg_len = bui_pkg_ack(&in_pkg,ack_msg);
      out_msg_len = bui_outnet_frm(pIn_pkg,ack_msg,ack_msg_len,out_msg);      
    }
    else if(pIn_pkg->msg[0] == 0x77)
    {
      bui_77_app_pkg((uint8_t *)pIn_pkg->msg, &in_pkg_77);
      ack_msg_len = bui_77_pkg_ack(&in_pkg_77,ack_msg);
      out_msg_len = bui_outnet_frm(pIn_pkg,ack_msg,ack_msg_len,out_msg);    
    }
    else
    {
    }
    Send_gtway_msg_to_uart(out_msg,out_msg_len);
  }
  else
  {
  }
}

/**************************************************************************************************
 * @fn          Bui_bcast_gtway_msg_from_inner_msg
 *
 * @brief       ���ڲ�֡����������ͨ�ŵĹ㲥֡.
 *
 * @param       pInner_msg      - �쳣��Ϣ
 *              inner_msg_len   - �쳣��Ϣ����
 *              pOut_msg        - ����쳣֡
 *
 * @return      ����쳣֡����
 **************************************************************************************************
 */
uint8_t Bui_exception_msg(uint8_t *pInner_msg, uint8_t inner_msg_len, uint8_t *pOut_msg)
{
	static uint8_t alarm_cnt = 0;
	uint8_t i = 0;
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  if (pInner_msg == NULL)
  {
    return 0;
  }
  
  pOut_msg[0] = GTWAY_FRAME_HEAD;
  pOut_msg[1] = 0x00; //Reserve
  pOut_msg[2] = 0x02; //Direction:broadcast
  pOut_msg[3] = 0x00; //Reserve
  pOut_msg[4] = pMyGtwayAddr->type[1]; //Gateway Type
  pOut_msg[5] = pMyGtwayAddr->type[0];
  pOut_msg[6] = pMyGtwayAddr->addr[3]; //Gateway Addr
  pOut_msg[7] = pMyGtwayAddr->addr[2];
  pOut_msg[8] = pMyGtwayAddr->addr[1];
  pOut_msg[9] = pMyGtwayAddr->addr[0];
  memcpy(&pOut_msg[10], sGtwayServerIP.addr, 4);  //Gateway Server IP
  memset(&pOut_msg[14], 0xff, 6); //Mobile Phone MAC
  pOut_msg[20] = 0x00;  //Frame counter(MSB)
  pOut_msg[21] = 0x00;  //Frame counter(LSB)
	
  pOut_msg[22] = (uint8_t)((10+inner_msg_len) >> 8); //Data Length(MSB)
  pOut_msg[23] = (uint8_t)(10+inner_msg_len);      //Data Length(LSB)
	
  pOut_msg[24] = 0x88;      
  pOut_msg[25] = 0x03;
  for(i=0;i<2;i++)
	{
		pOut_msg[26+i] = 0xee;
	}
  pOut_msg[28] = 0xff;
  pOut_msg[29] = 0xff; 
	
  pOut_msg[30] = alarm_cnt++;   
  pOut_msg[31] = inner_msg_len;
  for(i=0;i<inner_msg_len;i++)
	{
		pOut_msg[32+i] = *(pInner_msg+i);      
	}		
  pOut_msg[32+inner_msg_len] =gen_crc(&pOut_msg[24],8+inner_msg_len);//CSУ����
	
  pOut_msg[33+inner_msg_len]=0x16;
	
	
  pOut_msg[34 + inner_msg_len] = gen_crc(pOut_msg, 34 + inner_msg_len); //Check sum
  pOut_msg[35 + inner_msg_len] = GTWAY_FRAME_TAIL;

  return (36 + inner_msg_len);
}

/**************************************************************************************************
 * @fn          Bui_send_to_cloudserver_msg
 *
 * @brief       ���ڲ�֡����������ͨ�ŵĹ㲥֡.
 *
 * @param       pInner_msg      - 68��67��77��88֡
 *              inner_msg_len   - �쳣��Ϣ����
 *              pOut_msg        - ����쳣֡
 *
 * @return      ����쳣֡����
 **************************************************************************************************
 */
uint8_t Bui_send_to_cloudserver_msg(uint8_t *pInner_msg, uint8_t inner_msg_len, uint8_t *pOut_msg)
{
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  if (pInner_msg == NULL)
  {
    return 0;
  }
  
  pOut_msg[0] = GTWAY_FRAME_HEAD;
  pOut_msg[1] = 0x00; //Reserve
  pOut_msg[2] = 0x11; //Direction:���ص�������
  pOut_msg[3] = 0x00; //Reserve
  pOut_msg[4] = pMyGtwayAddr->type[1]; //Gateway Type
  pOut_msg[5] = pMyGtwayAddr->type[0];
  pOut_msg[6] = pMyGtwayAddr->addr[3]; //Gateway Addr
  pOut_msg[7] = pMyGtwayAddr->addr[2];
  pOut_msg[8] = pMyGtwayAddr->addr[1];
  pOut_msg[9] = pMyGtwayAddr->addr[0];
  memcpy(&pOut_msg[10], sGtwayServerIP.addr, 4);  //Gateway Server IP
  memset(&pOut_msg[14], 0xff, 6); //Mobile Phone MAC
  pOut_msg[20] = 0x00;  //Frame counter(MSB)
  pOut_msg[21] = 0x00;  //Frame counter(LSB)
	
  pOut_msg[22] = (uint8_t)((inner_msg_len) >> 8); //Data Length(MSB)
  pOut_msg[23] = (uint8_t)(inner_msg_len);      //Data Length(LSB)
	
	memcpy(&pOut_msg[24],pInner_msg,inner_msg_len);
	
	
  pOut_msg[24 + inner_msg_len] = gen_crc(pOut_msg, 24 + inner_msg_len); //Check sum
  pOut_msg[25 + inner_msg_len] = GTWAY_FRAME_TAIL;

  return (26 + inner_msg_len);
}


//void test_exception_msg(void)
//{
//	uint8_t inner_msg[4] = {0x00,0x04,0x05,0x01};
//	uint8_t pout_msg[50];
//	uint8_t len;
//	len = Bui_exception_msg(inner_msg,4,pout_msg);
//	
//	SEGGER_RTT_put_multi_char(pout_msg, len);
//}


/**************************************************************************************************
 */















