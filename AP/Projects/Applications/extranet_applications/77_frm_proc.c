/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   77_frm_proc.c
// Description: User APP.
//              �û�������ͨ��֡(0x77֡)����.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-28
// History:     2014-10-28  Leidi �޸ĺ���get_ap_ed_mac_ip(),���ӽڵ�״̬�ֽڲ���ѡ
//                                ���ȡ�ڵ���Ϣʱ�Ƿ���ڵ�����.
*****************************************************************************/

#include "77_frm_proc.h"
#include "bui_pkg_fun.h"
#include "nwk_layer.h"
#include "firmware_info_proc.h"

extern struct ip_mac_id *head_list;


/***************************************************************************
 * @fn          proc_77_frm
 *     
 * @brief       77֡���ݴ���
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���
 *     
 * @return      
 ***************************************************************************
 */ 
void proc_77_frm(uint8_t *pInMsg,uint8_t *out_pkg,uint16_t *len)
{
  uint8_t frame_type;
  
  frame_type = (*(pInMsg+1)) & 0x03;
  
  if(frame_type == 0x00)
  {
    proc_77_data_frm(pInMsg,out_pkg,len);//��ʱû��
  }
  else if(frame_type == 0x01)
  {
    proc_77_command_frm(pInMsg,out_pkg,len);//����֡
  }
  else if(frame_type == 0x02)
  {
    proc_77_ack_frm(pInMsg,out_pkg,len);//��ʱû��
  }
  else if(frame_type == 0x03)
  {
    proc_77_alarm_frm(pInMsg,out_pkg,len);//��ʱû��
  }
  else
  {
  }
}


/***************************************************************************
 * @fn          proc_77_data_frm
 *     
 * @brief       77֡����֡����
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���    
 *     
 * @return      void
 ***************************************************************************
 */ 
void proc_77_data_frm(uint8_t *pInMsg,uint8_t *out_pkg,uint16_t *len)
{
  
}

/***************************************************************************
 * @fn          proc_77_command_frm
 *     
 * @brief       77֡����֡����
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���    
 *     
 * @return      void
 ***************************************************************************
 */ 
void proc_77_command_frm(uint8_t *pInMsg,uint8_t *out_pkg,uint16_t *len)
{
  uint8_t msg_id2;
  app_77_pkg_t in_pkg; 
  bui_77_app_pkg(pInMsg,&in_pkg);
  msg_id2=in_pkg.msg[2];
  if(msg_id2 == 0x01)   
  {
    get_ap_ed_mac_ip(&in_pkg,out_pkg,len);
  }
  else if(msg_id2 == 0x04)
  {
    get_gtway_type_mac_77(&in_pkg,out_pkg,len);
  }
  else
  {
  }
}


/***************************************************************************
 * @fn          proc_77_ack_frm
 *     
 * @brief       77֡Ӧ��֡����
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���    
 *     
 * @return      void
 ***************************************************************************
 */ 
void proc_77_ack_frm(uint8_t *msg,uint8_t *out_pkg,uint16_t *len)
{
  
}

/***************************************************************************
 * @fn          proc_77_alarm_frm
 *     
 * @brief       77֡�澯֡����
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���    
 *     
 * @return      void
 ***************************************************************************
 */ 
void proc_77_alarm_frm(uint8_t *pInMsg,uint8_t *out_pkg,uint16_t *len)
{
  
}


/***************************************************************************
 * @fn          get_ap_ed_mac_ip
 *     
 * @brief       ��ѯ��������ַ���ڵ��ַ�ͽڵ�״̬��ǰ�����ֽ��Ǽ�����IP��ַ��
 *              �������ֽ��ǽڵ�������������ÿ7��һ���ڵ���Ϣ��ǰ4���ֽ��ǽڵ�
 *              mac��ַ��֮��2���ֽ��ǽڵ�IP��ַ�����1���ֽ��Ǳ�ʾ�ڵ�״̬��
 *              �ڵ�״̬�ֽں�������:
 *                  Bit T7  T6  T5  T4  T3  T2  T1       T0
 *                  0   -   -   -   -   -   -  ������    IP�޳�ͻ
 *                  1   -   -   -   -   -   -  ����      IP��ͻ
 *              ע: ��1��IP�г�ͻ�Ľڵ㲻�ɷ��ʣ�Ӧ�����޸���IP��
 *                  ��2���������ڵ�����״̬ʱ��T1λ��Ч��
 *                  ��3�����صĵ�ַ��Ϊ��λ��ǰ����λ�ں����緵�ص�IP��ַ
 *                       ��ϢΪ 5C 01����ʵ��IP��ַΪ0x015C�����ص�MAC��ַ
 *                       ��Ϣ5C 01 01 FE����ʵ��MAC��ַΪ0xFE01015C��
 *     
 * @data        2015��08��05��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���
 *     
 * @return      
 ***************************************************************************
 */ 
void get_ap_ed_mac_ip(app_77_pkg_t *in_pkg,uint8_t *out_pkg,uint16_t *len)
{
  uint16_t msg_len = 0;
  uint8_t msg_temp[NUM_CONNECTIONS * 7] ={0}; //ÿ���ڵ���Ϣ����7�ֽ�
  
  msg_len = nwk_getAPandTotalEDInfor(msg_temp);
  
  *len = bui_rep_77_command(in_pkg,msg_temp,msg_len,out_pkg);   
        
}

/***************************************************************************
 * @fn          get_gtway_type_mac_77
 *     
 * @brief       ��ȡ�������ͺ����ص�ַ��
 *     
 * @data        2015��08��05��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - �������ݡ�ǰ�����ֽ�Ϊ�������ͣ����ĸ��ֽ�Ϊ���ص�ַ��
 *              len     - ������ݳ���
 *     
 * @return      
 ***************************************************************************
 */ 
void get_gtway_type_mac_77(app_77_pkg_t *in_pkg,uint8_t *out_pkg,uint16_t *len)
{
  uint8_t i;
  uint8_t gtway_type_addr[6];
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  
  for(i=0;i<2;i++)
    gtway_type_addr[i] = pMyGtwayAddr->type[1-i];
  for(i=0;i<4;i++)
    gtway_type_addr[i+2] = pMyGtwayAddr->addr[3-i];
  
  *len = bui_rep_77_command(in_pkg,gtway_type_addr,6,out_pkg);
  
}



