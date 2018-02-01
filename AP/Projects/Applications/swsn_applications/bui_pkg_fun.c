/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   bui_pkg_fun.c
// Description: User APP.
//              �ڲ�֡�Ĺ������������ͷ�ļ�.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-21
// History:     2014-10-27  Leidi ���Ӻ���Debui_app_pkg().
*****************************************************************************/

#include "bsp.h"
#include "mrfi_spi.h"
#include "stm32_usart2_gtway.h"
#include "bui_pkg_fun.h"
#include "nwk_layer.h"
#include "user_def_fun.h"
#include "stm32_rtc.h"

extern ap_time_t real_time;



/***************************************************************************
 * @fn          bui_app_pkg
 *     
 * @brief       ��֡�ֽ�����ת���ɽṹ�����ݣ����ں�������
 *     
 * @data        2015��08��07��
 *     
 * @param       msg        - ֡�ֽ�����
 *              app_in_pkg - ת����Ľṹ������
 *     
 * @return      void
 ***************************************************************************
 */ 
void bui_app_pkg(uint8_t *msg,app_pkg_t *app_in_pkg)
{
  uint8_t j = 0;

  /*����֡��ʼ��1*/
  app_in_pkg->frm_head  =*msg;//
  /*����֡��ַa0-A1*/
  app_in_pkg->ctr_code  =*(msg+1);
  app_in_pkg->ed_addr   =*(msg+2);
  app_in_pkg->ed_addr   = (app_in_pkg->ed_addr<<8) |  *(msg+3);
  app_in_pkg->ap_addr   =*(msg+4);
  app_in_pkg->ap_addr   =(app_in_pkg->ap_addr<<8) | *(msg+5);
  app_in_pkg->frm_cnt   = *(msg+6);
 
  /*����֡�����򳤶�*/
  app_in_pkg->data_len  =*(msg+7);
 
  /*����֡������*/
  for( j=0;j<app_in_pkg->data_len;j++)
    app_in_pkg->msg[j]  =*(msg+8+j);

  /*����֡У����CS*/
  app_in_pkg->cs=*(msg+8+app_in_pkg->data_len);
 
  /*����֡������*/
  app_in_pkg->frm_end =*(msg+9+app_in_pkg->data_len);

}

/**************************************************************************************************
 * @fn          Debui_app_pkg
 *
 * @brief       ���ڲ�֡�ṹ������������ֽ�����.�ڴ˹����п��Ըı��������.
 *
 * @param       pApp_in_pkg   - �ڲ�֡�ṹ��
 *              new_ctr_code  - �µĿ�����
 *              pOut_msg      - ������ֽ�����
 *
 * @return      ������ֽ����鳤��
 **************************************************************************************************
 */
uint8_t Debui_app_pkg(const app_pkg_t *pApp_in_pkg, uint8_t new_ctr_code, uint8_t *pOut_msg)
{
  uint8_t data_len = pApp_in_pkg->data_len;
  uint8_t i;
  
  pOut_msg[0] = pApp_in_pkg->frm_head;
  pOut_msg[1] = new_ctr_code;
  pOut_msg[2] = pApp_in_pkg->ed_addr >> 8;
  pOut_msg[3] = pApp_in_pkg->ed_addr;
  pOut_msg[4] = pApp_in_pkg->ap_addr >> 8;
  pOut_msg[5] = pApp_in_pkg->ap_addr ;
  pOut_msg[6] = pApp_in_pkg->frm_cnt;
  pOut_msg[7] = data_len;
  for(i = 0; i< data_len; i++)
  {
    pOut_msg[8 + i] = pApp_in_pkg->msg[i];
  }
  pOut_msg[8 + data_len] = gen_crc(pOut_msg, 8 + data_len);
  pOut_msg[9 + data_len] = pApp_in_pkg->frm_end;
  
  return (10 + data_len);
}

/***************************************************************************
 * @fn          bui_77_app_pkg
 *     
 * @brief       ����77֡��77֡�����򳤶�Ϊ�����ֽڡ�
 *     
 * @data        2015��08��07��
 *     
 * @param       msg        - ֡�ֽ�����
 *              app_in_pkg - ת����Ľṹ������      
 *     
 * @return      void
 ***************************************************************************
 */ 
void bui_77_app_pkg(uint8_t *msg,app_77_pkg_t *app_in_pkg)
{
	uint16_t msgIndex = 0;
  //app_pkg_t app_in_pkg;
  /*����֡��ʼ��1*/
  app_in_pkg->frm_head  =*msg;//
  /*����֡��ַa0-A1*/
  app_in_pkg->ctr_code  =*(msg+1);
  app_in_pkg->ed_addr   =*(msg+2);
  app_in_pkg->ed_addr   = (app_in_pkg->ed_addr<<8) |  *(msg+3);
  app_in_pkg->ap_addr   =*(msg+4);
  app_in_pkg->ap_addr   =(app_in_pkg->ap_addr <<8) | *(msg+5);
  app_in_pkg->frm_cnt   = *(msg+6);
 
  /*����֡�����򳤶�*/
  app_in_pkg->data_len  =((*(msg+7)&0xffff)<<8) | *(msg+8);
 
  /*����֡������*/
  for( msgIndex=0;msgIndex<app_in_pkg->data_len;msgIndex++)
    app_in_pkg->msg[msgIndex]  =*(msg+9+msgIndex);
  /*����֡У����CS*/
  app_in_pkg->cs=*(msg+9+app_in_pkg->data_len);
 
  /*����֡������*/
  app_in_pkg->frm_end =*(msg+10+app_in_pkg->data_len);

}

/**************************************************************************************
 * @fn          bui_rep_pkg_command
 *
 * @brief       Build the ask frame which is sended to Ap after ED have writen registers.
 *
 * input parameters
 * @param   in_pkg        - the app commend frame from Ap.
 *          out_pkg       - the ask frame builed in ED 
 *          reply_para    - the value of register to read
 *          len           - the number of registers to read
 *
 * output parameters
 *
 * @return   16+para_len           - the length of out_pkg
 */
uint8_t bui_rep_pkg_command(app_pkg_t *in_pkg,uint8_t *reply_para,uint8_t para_len,uint8_t *out_pkg)
{

  uint8_t ctr_code;
  uint8_t j,k;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  *out_pkg = 0x68;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  *(out_pkg+7) = para_len+4;
   for(j=0;j<4;j++)
    *(out_pkg+8+j) = in_pkg->msg[j];
   for(k=0;k<para_len;k++)
     *(out_pkg+12+k) = *(reply_para+k);
   
  *(out_pkg+12+para_len)=gen_crc(out_pkg,12+para_len);//CSУ����
  *(out_pkg+13+para_len)=0x16;
  
  return para_len+14;
}


/**************************************************************************************
 * @fn          bui_rep_pkg_command
 *
 * @brief       Build the ask frame which is sended to Ap after ED have writen registers.
 *
 * input parameters
 * @param   in_pkg        - the app commend frame from Ap.
 *          out_pkg       - the ask frame builed in ED 
 *          reply_para    - the value of register to read
 *          len           - the number of registers to read
 *
 * output parameters
 *
 * @return   16+para_len           - the length of out_pkg
 */
uint8_t bui_rssi_rep_pkg_command(app_pkg_t *in_pkg,uint8_t *reply_para,uint8_t para_len,uint8_t *out_pkg)
{

  uint8_t ctr_code;
  uint8_t j,k;
  ctr_code= in_pkg->ctr_code;
  ctr_code |= BIT7;
  ctr_code &= ~BIT6;
  *out_pkg = 0x68;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  *(out_pkg+7) = para_len+4;
   for(j=0;j<4;j++)
    *(out_pkg+8+j) = in_pkg->msg[j];
   for(k=0;k<para_len;k++)
     *(out_pkg+12+k) = *(reply_para+k);
   
  *(out_pkg+12+para_len)=gen_crc(out_pkg,12+para_len);//CSУ����
  *(out_pkg+13+para_len)=0x16;
  
  return para_len+14;
}

/***************************************************************************
 * @fn          bui_rep_77_command
 *     
 * @brief       ����77֡������ظ�֡
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg     - �����77֡
 *              reply_para - �ظ�����
 *              para_len   - �ظ����ݳ���
 *              out_pkg    - ���ص��ֽ�����
 *     
 * @return      �ظ�֡����
 ***************************************************************************
 */ 
uint16_t bui_rep_77_command(app_77_pkg_t *in_pkg,uint8_t *reply_para,uint16_t para_len,uint8_t *out_pkg)
{

  uint8_t ctr_code;
  uint8_t idIndex = 0;
	uint16_t replyIndex = 0;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  *out_pkg = 0x77;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  
  *(out_pkg+7) = ((para_len+4)&0xffff)>>8;
  *(out_pkg+8) = para_len+4;
  
   for(idIndex=0;idIndex<4;idIndex++)
    *(out_pkg+9+idIndex) = in_pkg->msg[idIndex];
  
   for(replyIndex=0; replyIndex<para_len; replyIndex++)
     *(out_pkg+13+replyIndex) = *(reply_para+replyIndex);
   
  *(out_pkg+13+para_len)=gen_crc(out_pkg,13+para_len);//CSУ����
  *(out_pkg+14+para_len)=0x16;
  
  return para_len+15;
}

/***************************************************************************
 * @fn          bui_rep_88_command
 *     
 * @brief       ����88֡������ظ�֡
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg     - �����88֡
 *              reply_para - �ظ�����
 *              para_len   - �ظ����ݳ���
 *              out_pkg    - ���ص��ֽ�����
 *     
 * @return      �ظ�֡����
 ***************************************************************************
 */ 
uint8_t bui_rep_88_command(app_pkg_t *in_pkg,uint8_t *reply_para,uint8_t para_len,uint8_t *out_pkg)
{

  uint8_t ctr_code;
  uint8_t j,k;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  *out_pkg = 0x88;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  *(out_pkg+7) = para_len+4;
   for(j=0;j<4;j++)
    *(out_pkg+8+j) = in_pkg->msg[j];
   for(k=0;k<para_len;k++)
     *(out_pkg+12+k) = *(reply_para+k);
   
  *(out_pkg+12+para_len)=gen_crc(out_pkg,12+para_len);//CSУ����
  *(out_pkg+13+para_len)=0x16;
  
  return para_len+14;
}


/***************************************************************************
 * @fn          bui_pkg_ack
 *     
 * @brief       ����68��88֡��Ӧ��֡
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg     - �����68��88֡
 *              out_pkg    - ���ص��ֽ�����
 *     
 * @return      �ظ�֡����
 ***************************************************************************
 */ 
uint8_t bui_pkg_ack(app_pkg_t *in_pkg,uint8_t *out_pkg)
{
  uint8_t ctr_code;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  ctr_code &= ~BIT0;
  ctr_code |= BIT1;
  *out_pkg = in_pkg->frm_head;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  *(out_pkg+7) = 0;
    
   
  *(out_pkg+8)=gen_crc(out_pkg,8);//CSУ����
  *(out_pkg+9)=0x16; 
  return 10;
}

/***************************************************************************
 * @fn          bui_77_pkg_ack
 *     
 * @brief       ����77֡��Ӧ��֡
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg     - �����77֡
 *              out_pkg    - ���ص��ֽ�����
 *     
 * @return      �ظ�֡����
 ***************************************************************************
 */ 
uint8_t bui_77_pkg_ack(app_77_pkg_t *in_pkg,uint8_t *out_pkg)
{
  uint8_t ctr_code;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  ctr_code &= ~BIT0;
  ctr_code |= BIT1;
  *out_pkg = in_pkg->frm_head;
  *(out_pkg+1) = ctr_code; 
  *(out_pkg+2) = in_pkg->ed_addr >>8;
  *(out_pkg+3) = in_pkg->ed_addr;
  *(out_pkg+4) = in_pkg->ap_addr >>8;
  *(out_pkg+5) = in_pkg->ap_addr; 
  *(out_pkg+6) = in_pkg->frm_cnt;  
  *(out_pkg+7) = 0;
  *(out_pkg+8) = 0;  
   
  *(out_pkg+9)=gen_crc(out_pkg,9);//CSУ����
  *(out_pkg+10)=0x16; 
  return 11;
}

/***************************************************************************
 * @fn          bui_pkg_alarm
 *     
 * @brief       ����68֡�ĸ澯֡
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg     - �����88֡
 *              err_info   - �澯����
 *              err_len    - �澯��Դ
 *              out_pkg    - ���ص��ֽ�����
 *     
 * @return      �澯֡����
 ***************************************************************************
 */ 
uint8_t bui_pkg_alarm(app_pkg_t *in_pkg,uint8_t *err_info,uint8_t err_len,uint8_t *out_pkg)
{
  uint8_t ctr_code;
  uint8_t i;
  uint8_t k;
  static uint8_t alarm_frm_cnt=0;
  ctr_code= in_pkg->ctr_code;
  ctr_code &= ~BIT7;
  ctr_code &= ~BIT6;
  ctr_code |= BIT0;
  ctr_code |= BIT1;
  *out_pkg = in_pkg->frm_head;
  if(in_pkg != NULL)
  {
    *(out_pkg+1) = ctr_code;
    *(out_pkg+2) = in_pkg->ed_addr >>8;
    *(out_pkg+3) = in_pkg->ed_addr;
    *(out_pkg+4) = in_pkg->ap_addr >>8;
    *(out_pkg+5) = in_pkg->ap_addr; 
    *(out_pkg+6) = in_pkg->frm_cnt;  
    *(out_pkg+7) = err_len;
    for(k=0;k<err_len;k++)
      *(out_pkg+8+k) = *(err_info+k);             
    *(out_pkg+8+err_len)=gen_crc(out_pkg,8+err_len);//CSУ����
    *(out_pkg+9+err_len)=0x16;
    return 10+err_len;  
  }
  else
  { 
    alarm_frm_cnt ++;
        
   *(out_pkg+1) = 0x03;
    for(i=0;i<2;i++)
      *(out_pkg+i+2) = 0xee;
    *(out_pkg+4) = 0xff;
    *(out_pkg+5) = 0xff; 
    *(out_pkg+6) = alarm_frm_cnt;   
    //*(out_pkg+7) = 1;
    *(out_pkg+7) = err_len;
    for(k=0;k<err_len;k++)
      *(out_pkg+8+k) = *(err_info+k);             
    *(out_pkg+8+err_len)=gen_crc(out_pkg,8+err_len);//CSУ����
    *(out_pkg+9+err_len)=0x16;
    return 10+err_len;  
  } 
}

/***************************************************************************
 * @fn          bui_check_link_pkg
 *     
 * @brief       �������ڵ��Ƿ�����֡
 *     
 * @data        2015��08��07��
 *     
 * @param       ed_id   - �ڵ�ID��
 *              ed_addr - �ڵ��ַ
 *              out_pkg - ������ɺ�����֡
 *     
 * @return      ֡����
 ***************************************************************************
 */ 
uint8_t bui_check_link_pkg(uint8_t ed_id, uint8_t *ed_addr,uint8_t *out_pkg)
{
  uint8_t ctr_code; 
  uint8_t i=0;
  addr_t const *  myromaddr = nwk_getMyAddress();
  ctr_code= 0x05;
  *out_pkg = 0x68;
  *(out_pkg+1) = ctr_code;
  for( i=0;i<2;i++)
    *(out_pkg+i+2) = *(ed_addr+i);
  *(out_pkg+4) = myromaddr->addr[1];
  *(out_pkg+5) = myromaddr->addr[0];
  *(out_pkg+6) = 0x00;
  *(out_pkg+7) = 6;
  
  *(out_pkg+8) = 0x00;
  *(out_pkg+9) = 0x01;
  *(out_pkg+10) = 0x05;
  *(out_pkg+11) = 0x00;
  
  *(out_pkg+12) = ed_id;
  *(out_pkg+13) = 0x00;
   
  *(out_pkg+14)=gen_crc(out_pkg,14);//CSУ����
  *(out_pkg+15)=0x16;
  
  return 16;
}


/***************************************************************************
 * @fn          bui_time_sync_pkg
 *     
 * @brief       ����ͬ��֡�����ڶԽڵ����ʱ��ͬ��
 *     
 * @data        2015��08��07��
 *     
 * @param       out_pkg - ������ɺ�����֡
 *     
 * @return      ֡����
 ***************************************************************************
 */ 
uint8_t bui_time_sync_pkg(uint8_t *out_pkg)
{
  uint8_t ctr_code;
  static uint8_t time_sync_frm_cnt=0;
  addr_t const *  myromaddr = nwk_getMyAddress();
  
  time_sync_frm_cnt++;
  ctr_code= 0x05;
  *out_pkg = 0x68;
  *(out_pkg+1)  = ctr_code; 
  *(out_pkg+2)  = 0xff;
  *(out_pkg+3)  = 0xff;
  *(out_pkg+4)  = myromaddr->addr[1];
  *(out_pkg+5)  = myromaddr->addr[0]; 
  *(out_pkg+6)  = time_sync_frm_cnt;  
  *(out_pkg+7)  = 0x0d;
  *(out_pkg+8)  = 0x00;
  *(out_pkg+9)  = 0x02;
  *(out_pkg+10) = 0x30;
  *(out_pkg+11) = 0x01;
  *(out_pkg+12) = real_time.year-2000;
  *(out_pkg+13) = real_time.month;
  *(out_pkg+14) = real_time.day;
  *(out_pkg+15) = real_time.hour;
  *(out_pkg+16) = real_time.minute;
  *(out_pkg+17) = real_time.second;
  *(out_pkg+18) = real_time.msec;
  *(out_pkg+19) = set_week(real_time.week);
  *(out_pkg+20) = 1;
  *(out_pkg+21) =gen_crc(out_pkg,21);//CS
  *(out_pkg+22)=0x16;
  
  return 23;
}


/***************************************************************************
 * @fn          gen_crc
 *     
 * @brief       ����CRCУ����
 *     
 * @data        2015��08��07��
 *     
 * @param       frm  - ��������
 *              len  - �������ݳ���
 *     
 * @return      У����(һ���ֽ�)
 ***************************************************************************
 */ 
uint8_t gen_crc(uint8_t *frm, uint16_t len)
{
  uint8_t crc_sum=0;
  uint16_t i;
  for( i =0; i <len; i++)
  {
    crc_sum = *frm + crc_sum;
    frm++;
  }
  return crc_sum;
}

