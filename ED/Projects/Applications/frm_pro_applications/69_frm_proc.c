#include "69_frm_proc.h"
#include "68_frm_proc.h"
#include "string.h"


static uint8_t check_inner_frame_len(const ed_pkg_t *pIn_pkg);
static uint16_t get_inner_frame(const ed_pkg_t *pIn_pkg, uint8_t *pOut_msg);

/**************************************************************************************************
 * @fn          proc_69_data_frm
 *
 * @brief       �������Դ��ڵ���������ͨ��֡(0x69֡),�����������.
 *
 * @param       pIn_pkg - ���������ͨ��֡�ṹ��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_69_data_frm(ed_pkg_t *in_pkg)
{
  uint8_t data_frm[300];
  uint8_t ed_msg[1024];
  uint8_t out_pkg[1024];
  uint8_t data_frm_len;
  uint16_t data_77_frm_len;
  uint16_t ed_msg_len;
  uint8_t alarm_info[10];
  uint8_t inner_frm_head;
      
  /* �ڲ�֡��֡ͷ��֡����� */
  if(!check_inner_frame_len(in_pkg))
  {
    /* �澯��Ϣ:���ڴ�����������������Ч֡ */
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x04;  
    alarm_info[2] = 0x04;
    alarm_info[3] = 0x02;
    
    /* �����澯֡����� */
    ed_msg_len = bui_ed_alarm(NULL, alarm_info, 4, ed_msg);  
    Send_ed_msg_to_uart(ed_msg, ed_msg_len);
    
    return;
  }
  /*����68��Ӧ��*/
  pro_ed_data_ack(in_pkg);
  
  /* ������֡�ṹ���л�ȡ�ڲ�֡ */
  data_77_frm_len = get_inner_frame(in_pkg, data_frm);
  data_frm_len = data_77_frm_len; //77֡����ռ��2�ֽ�,����֡����ռ��1�ֽ�
  
  /* ���������ݴ洢���� */
  memset(out_pkg, 0x0, sizeof(out_pkg));
  
  /* �����ڲ�֡��֡ͷ���з��ദ�� */
  inner_frm_head = data_frm[0];
  
  switch(inner_frm_head)
  {
    case 0x68: proc_68_frm(in_pkg, out_pkg, &data_frm_len);   //�ֻ�(�û���)��ڵ�ͨ��,������㲥
               break;
    default: return;
  }

  /* ���ڲ�֡�Ĵ�������װ���ⲿ֡(0x69֡) */
  inner_frm_head = out_pkg[0];
  switch(inner_frm_head)
  {
    case 0x68: ed_msg_len = bui_outnet_frm(in_pkg, out_pkg, data_frm_len, ed_msg);
               break;
    default: return;
  }
  
  /* �򴮿ڷ�������ͨ��֡ */
  Send_ed_msg_to_uart(ed_msg, ed_msg_len);
	
}

/**************************************************************************************************
 * @fn          check_inner_frame_len
 *
 * @brief       ������Դ��ڵ���������ͨ��֡(0x69֡)�������򳤶��Ƿ���ȷ.
 *
 * @param       pIn_pkg - ���������ͨ��֡�ṹ��
 *
 * @return      1 - �����򳤶���ȷ
 *              0 - �����򳤶ȴ���
 **************************************************************************************************
 */
static uint8_t check_inner_frame_len(const ed_pkg_t *pIn_pkg)
{
  uint8_t frame_head = pIn_pkg->msg[0];
  uint8_t inner_msg_len;
  uint8_t mult_ip_count;
  uint8_t mult_field_len;
  
  switch(frame_head)
  {
//    case 0x66: /* Go down */
    case 0x68: /* Go down */
    case 0x88: inner_msg_len = pIn_pkg->msg[7] + 10;  //�����򳤶�Ϊ1�ֽ�
               break;
    case 0x77: inner_msg_len = (((pIn_pkg->msg[7]&0xffff)<<8) | pIn_pkg->msg[8]) + 11;  //�����򳤶�Ϊ2�ֽ�
               break;
    case 0x67: //�鲥֡���ڲ�֡��IP��ַ��ͳ����ڲ�֡���
    {
      mult_ip_count = pIn_pkg->msg[1];
      if(mult_ip_count > IP_GROUP_MAX_NUM)
      {
        return 0;
      }
      mult_field_len = 2 * mult_ip_count +2;
      inner_msg_len = mult_field_len + pIn_pkg->msg[mult_field_len + 7] + 10;
      break;
    }
    default: return 0;
  }
  
  /* ����ڲ�֡�ĳ����Ƿ�����ⲿ֡������ĳ��� */
  if(pIn_pkg->data_len != inner_msg_len)
  {
    return 0;
  }
  
  return 1;
}

/**************************************************************************************************
 * @fn          get_inner_frame
 *
 * @brief       ������֡�ṹ���л�ȡ�ڲ�֡.
 *
 * @param       pIn_pkg   - ���������ͨ��֡�ṹ��
 *              pOut_msg  - ������ڲ�֡����
 *
 * @return      ������ڲ�֡���鳤��
 **************************************************************************************************
 */
static uint16_t get_inner_frame(const ed_pkg_t *pIn_pkg, uint8_t *pOut_msg)
{
  uint16_t out_msg_len = pIn_pkg->data_len;
  
  memcpy(pOut_msg, pIn_pkg->msg, out_msg_len);

  return out_msg_len;
}
