/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   uart_frm_proc.c
// Description: �������Դ��ڵ���������ͨ��֡(0x69֡).
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-24
// History:     2014-10-24  Leidi ɾ��uart_frm_proc()���йر������������0x69֡��Ϣ�Ĵ���.
//                                ɾ���ⲿȫ�ֱ���resvr_pkg[MAX_RESVR_PKG]��resvr_pkg_num.
//                                ��ȫ�ֱ���pkg_clr_flag.
//              2014-10-31  Leidi ��дuart_frm_proc()���ж�ͨ�ŷ��򲿷ֵĴ���,���ͨ�ŷ���0x0e(�鲥).
//              2014-11-8   Leidi ��������gtway_info_check(),������������������ַ.
//                                �޸ĺ���uart_frm_proc()��ʵ��.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <string.h>
#include "uart_frm_proc.h"
#include "firmware_info_proc.h"
#include "gtway_frm_proc.h"
#include "69_frm_proc.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
extern addr_t sGtwayServerIP  ;
/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
uint8_t gtway_info_check(const gtway_pkg_t *in_pkg);
 
 
/**************************************************************************************************
 * @fn          uart_frm_proc
 *
 * @brief       �������Դ��ڵ���������ͨ��֡(0x69֡).
 *
 * @param       str - ���������֡
 *              len - ���������֡��
 *
 * @return      none
 **************************************************************************************************
 */
void uart_frm_proc(uint8_t *str,uint16_t len)
{
  gtway_pkg_t in_pkg;
  uint8_t trans_direction;
  uint8_t ack_msg[30];
  uint8_t ack_msg_len;
  uint8_t frame_type;
  
  /* ������ͨ��֡����װ������ͨ��֡�ṹ�� */
  bui_gtway_pkg(str,&in_pkg);
  
  /* �������ͨ��֡�е�������Ϣ�Ƿ���ȷ */
  if(gtway_info_check(&in_pkg) == 0)
  {
    return;
  }
  
  /* �������ͨ��֡�������еĴ��䷽���Ƿ���ȷ */
  //������(S) �ֻ�(M) ����(G) ����(C)
  trans_direction = (in_pkg.ctr_code>>8)&0x0f;
  switch(trans_direction)
  {
    case 0x00: return;  //M->S(X)
    case 0x01: return;  //S->M(X)
    case 0x02: return;  //G->S(X)
    case 0x03: reserve_server_ip(&in_pkg);
		           break;   //S->G
    case 0x04: break;   //M->G
    case 0x05: return;  //G->M(X)
    case 0x06: break;   //M->S->G
    case 0x07: return;  //G->S->M(X)
//    case 0x08: put_multi_hex_uart1(str,len);//��������ʱ������ת��
//               return;  //M->C
    case 0x09: //TCP_Send_Filter(str,len);//��������ʱ������ת��
               return;  //C->M
    case 0x0a: break;   //C->G
    case 0x0b: return;  //G->C(X)
//    case 0x0c: put_multi_hex_uart1(str,len);//��������ʱ������ת��
//               return;  //M->S->C
    case 0x0d: //TCP_Send_Filter(str,len);//��������ʱ������ת��
               return;  //C->S->M
    case 0x0e: break;   //Multicast
    case 0x0f: break;   //Broadcast
		case 0x10: return;
		case 0x11: return;
		case 0x12: return;
		case 0x13: return;
    default: return;
  }

  /* �������ͨ��֡�Ƿ����ü��� */
  if(in_pkg.ctr_code & 0x8000)
  {
    /* ��������ͨ��֡(δʵ��) */
  }

  /* �������ͨ��֡�Ƿ���ҪӦ�� */
  if(in_pkg.ctr_code & 0x4000)
  {
    /* �ڴ����֡ǰ�Ƚ���Ӧ���� */
    ack_msg_len = bui_gtway_ack(&in_pkg, ack_msg);
    Send_gtway_msg_to_uart(ack_msg, ack_msg_len);
  }

  /* �������ͨ��֡�������е�֡���� */
  frame_type = (in_pkg.ctr_code>>8) & 0x20;
  if(frame_type == 0x00)    //����֡
  {
    proc_69_data_frm(&in_pkg);
  }
  else if(frame_type == 0x20)//Ӧ��֡
  {
    /* ����Ӧ��֡(δʵ��) */
  }
  
  return;
}

/**************************************************************************************************
 * @fn          gtway_info_check
 *
 * @brief       ������Դ��ڵ���������ͨ��֡(0x69֡)�ġ��������͡��͡����ص�ַ���Ƿ���ȷ.
 *
 * @param       pIn_pkg - ���������ͨ��֡�ṹ��
 *
 * @return      1 - ������Ϣ��ȷ
 *              0 - ������Ϣ����
 **************************************************************************************************
 */
uint8_t gtway_info_check(const gtway_pkg_t *in_pkg)
{
  addr_t universal_gtway_addr = {0x00, 0x00, 0x00, 0x00};
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  addr_t my_gtway_addr;
  uint16_t my_gtway_type;
  uint8_t i;
  
  
  /* ����ʹ��ͨ�����ص�ַ */
  if(!memcmp(&universal_gtway_addr, in_pkg->gtway_addr, MAC_ADDR_SIZE))
  {
    return 1;
  }
  
  /* �������ص�ַ���ֽ��� */
  //����ͨ��֡ʹ�ô��(MSB)�ֽ���
  //�����ڲ�ʹ��С��(LSB)�ֽ���
  for(i = 0; i < MAC_ADDR_SIZE; i++)
  {
    my_gtway_addr.addr[i] = pMyGtwayAddr->addr[3-i]; //pMyGtwayAddr.addr={0x01,0x00,0x00,0x00}
  }
  
  /* ������ص�ַ�Ƿ�Ϊ�����ص�ַ */
  if(memcmp(&my_gtway_addr, in_pkg->gtway_addr, MAC_ADDR_SIZE))
  {
    return 0;
  }
  
  my_gtway_type = pMyGtwayAddr->type[0] | (pMyGtwayAddr->type[1] << 8); //pMyGtwayAddr.type={0xfc,0x00}
  
  /* ������������Ƿ�Ϊ���������� */
  if(in_pkg->gtway_type != my_gtway_type)
  {
    return 0;
  }
  
  return 1;
}


/**************************************************************************************************
 * @fn          reserve_server_ip
 *
 * @brief       ���������IP.
 *
 * @param       pIn_pkg   - ���������ͨ��֡�ṹ��
 *
 * @return      void
 **************************************************************************************************
 */
void reserve_server_ip(const gtway_pkg_t *pIn_pkg)
{
  
  memcpy(sGtwayServerIP.addr, pIn_pkg->server_addr, 4);
}
/**************************************************************************************************
 */


