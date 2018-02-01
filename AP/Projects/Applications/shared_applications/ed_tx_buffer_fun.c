/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ed_buffer_fun.c
// Description: User APP.
//              �ڵ��շ�����������ͷ�ļ�.
// Author:      Leidi
// Version:     1.1
// Date:        2014-10-21
// History:     2014-10-21  Leidi ��ʼ�汾����.
//              2014-10-31  Leidi �޸ĺ���proc_tx_ed_info()���߼�,����ҪӦ���ֱ֡�ӷ���.
//              2014-11-03  Leidi �޸ĺ���Write_tx_ed_buffer(),�����鲥֡(0x67֡)�Ĵ���.
//              2014-04-24  Leidi ��ÿ���ڵ�Ļ����������Ӷ�дID�����γɻ��ζ���.
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
/* �ڵ�ķ��ͻ����� */
static ed_tx_buffer_t Ed_tx_buffer[TX_BUFFER_NUM_FOR_ALL_ED];

/* �ڵ�ķ��ͻ�����ʹ���� */
static uint8_t ed_tx_buffer_usage;

/* ���������ӵķ�����IP��ַ */
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
 * @brief       ��69֡����Ϣд�ڵ㷢�ͻ�����.
 *
 * @param       pIn_gtway_pkg - ���յ���69֡��Ϣ
 *              pOut_msg      - ����ķ���֡(��澯֡)
 *              pOut_len      - ����ķ���֡����
 *
 * @return      none
 **************************************************************************************************
 */
void Write_tx_ed_buffer(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t* pOut_len)//��69֡����Ϣд�ڵ㷢�ͻ�����
{
  uint8_t           inner_frm_head = GET_FRAME_HEAD(pIn_gtway_pkg->msg); //��ȡ�ڲ�����֡֡ͷ
  uint16_t          ed_ip;   //�ڵ��ַ
  uint8_t           mult_ip_count;  //IP��
  uint16_t          mult_ip_group[IP_GROUP_MAX_NUM];//�洢IP��ַ������
  uint8_t           i;
  ed_tx_buffer_t    *pEd_tx_buffer; //ָ�򵥸��ڵ㷢�ͻ�������ָ�� 
  gtway_frm_info_t  *pEd_tx_info;   //ָ������������ָ��
  app_pkg_t         in_pkg;         //�ڲ�֡�ṹ�����  
  uint8_t           alarm_info[10]; //�澯��Ϣ
	
	static uint8_t right_frm_num[1024]={0};
	static uint16_t frm_count=0;
	
  switch(inner_frm_head)
  {
    case 0x68:  /* ��ڵ�ͨ��:����֡ͷ */
    {
      ed_ip = GET_ED_IP(pIn_gtway_pkg->msg);//��ýڵ��ַ
      
      pEd_tx_buffer = search_ip_in_ed_tx_buffer(ed_ip); //��IP��ַƥ�䷢�ͻ�������ַ�
      if (pEd_tx_buffer == NULL)//δ�ҵ�
      {
        pEd_tx_buffer = add_new_ed_in_ed_tx_buffer(ed_ip); //�ڷ��ͻ�������ӽڵ�IP
        if (pEd_tx_buffer == NULL)
        {
          /* ���нڵ㷢�ͻ������Ѿ������� */
          goto BUILD_ALARM_FRM;
        }
      }
      
      pEd_tx_info = find_a_avaliable_ed_tx_info(pEd_tx_buffer);
			
      if (pEd_tx_info == NULL)
      {
        /* �˽ڵ��޿��õĻ����� */
//				Write_tx_fail_ed_buffer(uint8_t lid, uint8_t *pIn_msg, uint8_t* pIn_len);
				pEd_tx_buffer = add_new_ed_in_ed_tx_buffer(ed_ip);
        if (pEd_tx_buffer == NULL)
        {
          /* ���нڵ㷢�ͻ������Ѿ������� */
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
    case 0x67:    /* ��ڵ�ͨ��:�鲥֡ͷ */
    {
      mult_ip_count = Change_67frm_to_68frm((gtway_pkg_t *)pIn_gtway_pkg, mult_ip_group);
      
      /* ����鲥֡�е�IP����,�Է�����Խ�� */
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
            /* ���нڵ㷢�ͻ������Ѿ������� */
            goto BUILD_ALARM_FRM;
          }
        }
        
        pEd_tx_info = find_a_avaliable_ed_tx_info(pEd_tx_buffer);
        if (pEd_tx_info == NULL)
        {
          /* �˽ڵ��޿��õĻ����� */
          goto BUILD_ALARM_FRM;
        }
        write_a_ed_tx_info(pEd_tx_info, pIn_gtway_pkg);
      }
      
      break;
    }
    default:
    {
      /* �����֡ͷ */
    }
  }
  
  return;
  
BUILD_ALARM_FRM:  
  { 
    /* �����澯֡ */
    
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
 * @brief       ��IP��ַ����ƥ��Ľڵ㷢�ͻ�����.
 *
 * @param       ip  - �ڵ�IP��ַ
 *
 * @return      ƥ��Ľڵ㷢�ͻ�����
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
 * @brief       ��IP��ַ��ʼ���µĽڵ㷢�ͻ�����.
 *
 * @param       ip  - �ڵ�IP��ַ
 *
 * @return      �µĽڵ㷢�ͻ�����
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
 * @brief       ��ָ���ڵ�ķ��ͻ���������һ�����õ�λ��.
 *
 * @param       pEd_tx_buffer - ָ���ڵ�ķ��ͻ�����
 *
 * @return      ���õ�λ��,���Դ��1��68֡��69֡�е�������Ϣ
 **************************************************************************************************
 */
static gtway_frm_info_t *find_a_avaliable_ed_tx_info(ed_tx_buffer_t *pEd_tx_buffer)
{
  uint8_t read_id = pEd_tx_buffer->ed_tx_info_read_id;
  uint8_t write_id = pEd_tx_buffer->ed_tx_info_write_id;
  uint8_t i;
  /* ��һ��write_id */
  write_id = (write_id == TX_BUFFER_NUM_FOR_ONE_ED - 1) ? 0x00 : ++write_id;
  
//  if((write_id == read_id) && 
//    (pEd_tx_buffer->ed_tx_info[write_id].in_use == BUFFER_IN_USE))
//  {
//    return NULL;  //����������
//  }
	
	for(i=0;i<TX_BUFFER_NUM_FOR_ONE_ED;i++)
  {
	  if(pEd_tx_buffer->ed_tx_info[i].in_use == BUFFER_AVALIABLE)
    {
    return &pEd_tx_buffer->ed_tx_info[i];		  
		}
	}
	
    return NULL;  //����������
	
//  else  //������δ��
//  {
//    /* ����write_id */
//    pEd_tx_buffer->ed_tx_info_write_id = write_id;
//    
//    /* ���ض�Ӧ�Ļ����� */
//    return &pEd_tx_buffer->ed_tx_info[write_id];
//  }
	
}

/**************************************************************************************************
 * @fn          write_a_ed_tx_info
 *
 * @brief       ��һ��69֡�е�������Ϣд��ָ���ڵ�ķ��ͻ�������.
 *
 * @param       pEd_tx_info   - �ڵ�ķ��ͻ�����
 *              pIn_gtway_pkg - 69֡�ṹ
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
 * @brief       �������нڵ�ķ��ͻ�����,�������е���Ϣ.
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
//      /* ��������δ����ʱ */
//      while((read_id != write_id) || 
//            (Ed_tx_buffer[i].ed_tx_info[read_id].in_use == BUFFER_IN_USE))
//      {
//        /* ��һ��read_id */
//        read_id = (read_id == TX_BUFFER_NUM_FOR_ONE_ED - 1) ? 0x00 : ++read_id;
//        
//        /* �����Ӧ�Ļ����� */
//        proc_tx_ed_info(ed_ip, &Ed_tx_buffer[i].ed_tx_info[read_id]);
//      }
//      
//      /* ����read_id */
//      Ed_tx_buffer[i].ed_tx_info_read_id = read_id;
      
			for(avalible_index=0;avalible_index<TX_BUFFER_NUM_FOR_ONE_ED;avalible_index++)
			{
			  if(Ed_tx_buffer[i].ed_tx_info[avalible_index].in_use == BUFFER_IN_USE)
        {
				  proc_tx_ed_info(ed_ip, &Ed_tx_buffer[i].ed_tx_info[avalible_index]); 
				}
			}
			
      /* ʹ����������ͷ�����ڵ�ռ�õĻ����� */
      Ed_tx_buffer[i].in_use = BUFFER_AVALIABLE;
    }
  }
	if(i == TX_BUFFER_NUM_FOR_ALL_ED)
	{
		start_emergency_dispatch_in_rtc = 1;
		emergency_dispatch_flag = 1; //�������������֮�󣬿�������ִ��һ�ζ���ǰ���͵����ݽ����ط���
	}
}

/**************************************************************************************************
 * @fn          proc_tx_ed_info
 *
 * @brief       ����ָ���ڵ�ķ��ͻ������е���Ϣ.
 *
 * @param       ip          - �ڵ��IP��ַ
 *              pEd_tx_info - �ڵ�ķ��ͻ�����
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

  /* ������ED��֡�Ƿ���ҪӦ�� */
  if(reply_request_flag == 0x40)  //��ҪӦ��
  {
    /* ����ǰ���Ӧ���־λ.�������,ED����ظ�Ӧ��֡ */
    CLEAN_REPLY_REQUEST_FLAG(pEd_tx_info->inner_msg);
    
    /* ���Ӧ���־λ��Ӧ���¼���У��� */
    pEd_tx_info->inner_msg[inner_msg_len - 2] = gen_crc(pEd_tx_info->inner_msg, (inner_msg_len - 2));
    
    /* ����ED */
    send_result = send_net_pkg(p->id, pEd_tx_info->inner_msg, inner_msg_len);
    
    /* ���ͳɹ�,�����ػظ�Ӧ��֡ */
    if(send_result == 1)
    {
      change_gtway_frm_info_for_reply(pEd_tx_info);
      output_msg_len = bui_gtway_msg_from_gtway_frm_info(pEd_tx_info, output_msg);
      
      Send_gtway_msg_to_uart(output_msg, output_msg_len);
    }
  }
  else  //����ҪӦ��
  {
    /* ֱ�ӷ��� */
    //SMPL_SendOpt(p->id, pEd_tx_info->inner_msg, inner_msg_len, SMPL_TXOPTION_NONE);
    send_net_pkg(p->id, pEd_tx_info->inner_msg, inner_msg_len);
  }

  goto FREE_BUFFER;


BUILD_ALARM_FRM:  
  {
    /* �����澯֡ */
    
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
 * @brief       ��ָ���ڵ�ķ��ͻ������е�֡��дΪ��Ӧ��Ӧ��֡.
 *
 * @param       pEd_tx_info   - �ڵ�ķ��ͻ�����
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

  /* �޸Ŀ�����:Ӧ��֡,����ҪӦ��,ͨ�ŷ�ʽ���� */
  control_code = GET_CONTROL_CODE(pEd_tx_info->inner_msg);
  control_code &= 0x0c;
  control_code |= 0x02;
  SET_CONTROL_CODE(pEd_tx_info->inner_msg, control_code);

  /* �޸�֡�е������򳤶�:Ӧ��֡�����򳤶�Ϊ0 */
  SET_DATA_LEN(pEd_tx_info->inner_msg, 0);
  
  /* ���¼���Ӧ��֡��У��� */
  check_sum = gen_crc(pEd_tx_info->inner_msg, 8);
  pEd_tx_info->inner_msg[8] = check_sum;

  /* �������Ӧ��֡��֡β */
  pEd_tx_info->inner_msg[9] = INNER_FRAME_TAIL;
  
  /* �������û������е�֡���� */
  pEd_tx_info->inner_msg_len = GET_FRAME_LEN(pEd_tx_info->inner_msg);
}


/**************************************************************************************************
 * @fn          bui_gtway_msg_from_gtway_frm_info
 *
 * @brief       ��ָ���ڵ�ķ��ͻ������е�֡����Ϊ��Ӧ������ͨ��֡.
 *
 * @param       pEd_tx_info   - �ڵ�ķ��ͻ�����
 *              pOut_msg      - ���������ͨ��֡
 *
 * @return      ����ͨ��֡����
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
 * @brief       �޸����ڱ�ʶ��ͬ�ڵ㷢�ͻ�������IP��ַ.��ȷʵ�����ڴ˽ڵ����δΪ�˽ڵ���仺����,
 *              ����BUFFER_IP_MODIFY_FAIL.�˺�������������ʹ��0x0000��0xffff��ΪIP��ַ.
 *
 * @param       current_ed_ip   - ��ǰ���ͻ������еĽڵ�IP
 *              modified_ed_ip  - �޸ĺ�Ľڵ�IP
 *
 * @return      BUFFER_SUCCESS        - �޸ĳɹ�
 *              BUFFER_IP_MODIFY_FAIL - �޸�ʧ��,δ�Ի����������κδ���
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
 * @brief       �ͷ�ĳһ�ڵ�ķ��ͻ�����,���������ͷź���Ա������ڵ�ʹ��.
 *
 * @param       ed_ip   - ��ʶҪ�ͷŻ������Ľڵ�IP
 *
 * @return      BUFFER_SUCCESS    - �ͷŻ�����
 *              BUFFER_FREE_FAIL  - �ͷŻ�����ʧ��,δ�Ի����������κδ���
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
 * @brief       �ж��Ƿ���д���鲥֡.
 *
 * @param       ed_ip   - ��ʶҪ�ͷŻ������Ľڵ�IP
 *
 * @return      BUFFER_SUCCESS    - �ͷŻ�����
 *              BUFFER_FREE_FAIL  - �ͷŻ�����ʧ��,δ�Ի����������κδ���
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
 * @brief       ������1������д�뷢�ͻ�����
 *
 * @param       pIn_gtway_pkg - ���յ���68֡��Ϣ
 *              pOut_msg      - ����ķ���֡(��澯֡)
 *              pOut_len      - ����ķ���֡����
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
    case 0x68:  /* ��ڵ�ͨ��:����֡ͷ */
    {
      ed_ip = ((pIn_pkg[2]<<8)|pIn_pkg[3]);
      
      pEd_tx_buffer = search_ip_in_ed_tx_buffer(ed_ip);
      if (pEd_tx_buffer == NULL)
      {
        pEd_tx_buffer = add_new_ed_in_ed_tx_buffer(ed_ip);
        if (pEd_tx_buffer == NULL)
        {
          /* ���нڵ㷢�ͻ������Ѿ������� */
          break;
        }
      }
      
      pEd_tx_info = find_a_avaliable_ed_tx_info(pEd_tx_buffer);
      if (pEd_tx_info == NULL)
      {
        /* �˽ڵ��޿��õĻ����� */
        break;
      }

//      write_a_ed_tx_info(pEd_tx_info, pIn_gtway_pkg);
        write_usart1_data_info(pEd_tx_info,pIn_pkg);
      break;
    }
    default:
    {
      /* �����֡ͷ */
    }
  }
  
  return;
}

/**************************************************************************************************
 * @fn          write_a_ed_tx_info
 *
 * @brief       �����յ��Ĵ���1����д�뻺����
 *
 * @param       pEd_tx_info   - �ڵ�ķ��ͻ�����
 *              pIn_gtway_pkg - 69֡�ṹ
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


