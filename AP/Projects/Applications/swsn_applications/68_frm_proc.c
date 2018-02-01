/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   68_frm_proc.c
// Description: User APP.
//              SWSN����֡(0x68֡)��������Դ�ļ�.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-21
// History:     2014-10-21  Leidi ��дԭ��68֡�����ȫ��������֧�ֽڵ㷢�ͻ���������.
//              2014-10-31  Leidi ��Ӻ���send_bcast_frm(),���ڷ��͹㲥֡������Ӧ��֡.��68֡�����������ô˺���.
//                                ʹ�ú궨��MISSES_IN_A_ROW��ʾ��ED���ͷǹ㲥֡������Դ���.
//                                ��д����send_bcast_pkg(),ʹ�ù㲥LID(SMPL_LINKID_USER_UUD),����ͷ�ļ�nwk.h.
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
#define MISSES_IN_A_ROW 1        //��ED���ͷǹ㲥֡������Դ���
#define MISSES_BCAST_IN_A_ROW 3  //��ED���͹㲥֡������Դ���

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
 * @brief       �ԴӴ������������ͨ��֡(0x69֡)�е�SWSN����֡(0x68֡)����֡���ͽ��з���.
 *
 * @param       pIn_gtway_pkg - ���������ͨ��֡(0x69֡)�ṹ��
 *              pOut_msg      - �����Ӧ��֡�ֽ�����
 *              pOut_len      - ������ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t frame_type = GET_FRAME_TYPE(pIn_gtway_pkg->msg);//֡����
  
  if(frame_type == 0x00)//����֡
  {
    proc_68_data_frm(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
  else if(frame_type == 0x01)//����֡
  {
    proc_68_command_frm(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
  else if(frame_type == 0x02) //Ӧ��֡
  {
    proc_68_ack_frm(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
  else if(frame_type == 0x03) //�澯֡
  {
    proc_68_alarm_frm(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
  return;
}


/**************************************************************************************************
 * @fn          proc_68_data_frm
 *
 * @brief       ���������������ͨ��֡(0x69֡)�е�SWSN����֡(0x68֡)�еġ�����֡��.
 *
 * @param       pIn_gtway_pkg - ���������ͨ��֡(0x69֡)�ṹ��
 *              pOut_msg      - �����Ӧ��֡�ֽ�����
 *              pOut_len      - ������ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_data_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t   comm_type = GET_COMM_TYPE(pIn_gtway_pkg->msg);//ͨ������
  uint16_t  ed_ip     = GET_ED_IP(pIn_gtway_pkg->msg);//��ȡ�ڵ��ַ

  if (comm_type == 0x00 || comm_type == 0x04) //͸����͸��
  {
    if (ed_ip == 0x0000 || ed_ip == 0xffff) //�㲥��ַ
    {
      send_bcast_frm(pIn_gtway_pkg->msg, pIn_gtway_pkg->data_len, pOut_msg, pOut_len);
    }
    else  //������ַ
    {
      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    }
  }
  else if (comm_type == 0x08) //���ʼ�����
  {
    /* code */
  }
  else if (comm_type == 0x0C) //�������ʲ���
  {
    Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
  }
}

/**************************************************************************************************
 * @fn          proc_68_command_frm
 *
 * @brief       ���������������ͨ��֡(0x69֡)�е�SWSN����֡(0x68֡)�еġ�����֡��.
 *
 * @param       pIn_gtway_pkg - ���������ͨ��֡(0x69֡)�ṹ��
 *              pOut_msg      - �����Ӧ��֡�ֽ�����
 *              pOut_len      - ������ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_command_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t   comm_type       = GET_COMM_TYPE(pIn_gtway_pkg->msg);//��ȡ��������
  uint8_t   command_id_fun  = GET_COMMANA_ID_DOMAIN(pIn_gtway_pkg->msg);//�����ʶ
  uint16_t  ed_ip           = GET_ED_IP(pIn_gtway_pkg->msg);//�ڵ��ַ
  app_pkg_t in_pkg;

  if (comm_type == 0x00 || comm_type == 0x04) //͸����͸��
  {
    if (ed_ip == 0x0000 || ed_ip == 0xffff) //�㲥��ַ
    {
      send_bcast_frm(pIn_gtway_pkg->msg, pIn_gtway_pkg->data_len, pOut_msg, pOut_len);
    }
    else  //������ַ
    {
      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    }
  }
  else if (comm_type == 0x08) //���ʼ�����
  {
    bui_app_pkg((uint8_t *)pIn_gtway_pkg->msg, &in_pkg);

    if (command_id_fun == 0x01) //�����������֡
    {
      net_para_proc(&in_pkg, pOut_msg, pOut_len);
    }
    else if (command_id_fun == 0x02)  //����״̬����֡
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
 * @brief       ���������������ͨ��֡(0x69֡)�е�SWSN����֡(0x68֡)�еġ�Ӧ��֡��.
 *
 * @param       pIn_gtway_pkg - ���������ͨ��֡(0x69֡)�ṹ��
 *              pOut_msg      - �����Ӧ��֡�ֽ�����
 *              pOut_len      - ������ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_ack_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t   comm_type = GET_COMM_TYPE(pIn_gtway_pkg->msg);
  uint16_t  ed_ip     = GET_ED_IP(pIn_gtway_pkg->msg);

  if (comm_type == 0x00 || comm_type == 0x04) //͸����͸��
  {
    if (ed_ip == 0x0000 || ed_ip == 0xffff) //�㲥��ַ
    {
      send_bcast_frm(pIn_gtway_pkg->msg, pIn_gtway_pkg->data_len, pOut_msg, pOut_len);
    }
    else  //������ַ
    {
      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    }
  }
  else if (comm_type == 0x08) //���ʼ�����
  {
    /* code */
  }
  else if (comm_type == 0x0C) //�û���¼���
  {
    /* code */
  }
}

/**************************************************************************************************
 * @fn          proc_68_alarm_frm
 *
 * @brief       ���������������ͨ��֡(0x69֡)�е�SWSN����֡(0x68֡)�еġ��澯֡��.
 *
 * @param       pIn_gtway_pkg - ���������ͨ��֡(0x69֡)�ṹ��
 *              pOut_msg      - �����Ӧ��֡�ֽ�����
 *              pOut_len      - ������ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_alarm_frm(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t   comm_type     = GET_COMM_TYPE(pIn_gtway_pkg->msg);
  uint16_t  ed_ip         = GET_ED_IP(pIn_gtway_pkg->msg);


  if (comm_type == 0x00 || comm_type == 0x04) //͸����͸��
  {
    if (ed_ip == 0x0000 || ed_ip == 0xffff) //�㲥��ַ
    {
      send_bcast_frm(pIn_gtway_pkg->msg, pIn_gtway_pkg->data_len, pOut_msg, pOut_len);
    }
    else  //������ַ
    {
      Write_tx_ed_buffer(pIn_gtway_pkg, pOut_msg, pOut_len);
    } 
  }
  else if (comm_type == 0x08) //���ʼ�����
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
 * @brief       ����SWSN�㲥֡,��������ҪӦ���־λ����Ӧ��֡.
 *
 * @param       pIn_msg       - �����SWSN�㲥֡�ֽ�����
 *              in_msg_len    - ������ֽ����鳤��
 *              pOut_msg      - �����Ӧ��֡�ֽ�����
 *              pOut_msg_len  - ������ֽ����鳤��
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
  
  //�������豸(�ֻ��ͻ��ˡ���������)Ҫ��Ĺ㲥���ݣ���������
  for(bcast_send_index = 0;bcast_send_index<2;bcast_send_index++)
  {
    /* ֱ�ӷ���.����ǰ�������Ӧ���־λ.ED����Թ㲥֡�ظ�Ӧ��֡ */
    send_result = send_bcast_pkg((uint8_t *)pIn_msg, in_msg_len);
    SWSN_DELAY(20);
  }
  
  /* ��ҪӦ���ҷ��ͳɹ�,�����ػظ�Ӧ��֡ */
  if ((reply_request_flag == 0x40) && (send_result == 1))
  {
    bui_app_pkg((uint8_t *)pIn_msg, &in_pkg);
    *pOut_msg_len = bui_pkg_ack(&in_pkg, pOut_msg);
  }
}


/**************************************************************************************************
 * @fn          send_net_pkg
 *
 * @brief       ��ָ�������ӱ�ʶ��(LID)��ED��������.����Դ�����MISSES_IN_A_ROW����.
 *
 * @param       lid     - ���ӱ�ʶ��(LID)
 *              string  - ������ֽ�����
 *              tx_len  - �ֽ����鳤��
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
 * @brief       ��ED���͹㲥����.
 *
 * @param       string  - ������ֽ�����
 *              len     - �ֽ����鳤��
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
  
  //����Ϊֻ����һ�Σ����ʧ�ܣ���������Ķ�������ѡ����
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
 * @brief       ���㷢����ɺ�ĵȴ�ʱ��
 *
 * @param       payload_len     - ������֡����
 *
 * @return      �ȴ�ʱ��
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



