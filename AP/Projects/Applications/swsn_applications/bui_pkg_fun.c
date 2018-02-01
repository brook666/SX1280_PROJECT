/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   bui_pkg_fun.c
// Description: User APP.
//              内部帧的构建与解析处理头文件.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-21
// History:     2014-10-27  Leidi 增加函数Debui_app_pkg().
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
 * @brief       将帧字节数组转换成结构体数据，便于后续处理
 *     
 * @data        2015年08月07日
 *     
 * @param       msg        - 帧字节数组
 *              app_in_pkg - 转换后的结构体数据
 *     
 * @return      void
 ***************************************************************************
 */ 
void bui_app_pkg(uint8_t *msg,app_pkg_t *app_in_pkg)
{
  uint8_t j = 0;

  /*构造帧起始符1*/
  app_in_pkg->frm_head  =*msg;//
  /*构造帧地址a0-A1*/
  app_in_pkg->ctr_code  =*(msg+1);
  app_in_pkg->ed_addr   =*(msg+2);
  app_in_pkg->ed_addr   = (app_in_pkg->ed_addr<<8) |  *(msg+3);
  app_in_pkg->ap_addr   =*(msg+4);
  app_in_pkg->ap_addr   =(app_in_pkg->ap_addr<<8) | *(msg+5);
  app_in_pkg->frm_cnt   = *(msg+6);
 
  /*构造帧数据域长度*/
  app_in_pkg->data_len  =*(msg+7);
 
  /*构造帧数据域*/
  for( j=0;j<app_in_pkg->data_len;j++)
    app_in_pkg->msg[j]  =*(msg+8+j);

  /*构造帧校验码CS*/
  app_in_pkg->cs=*(msg+8+app_in_pkg->data_len);
 
  /*构造帧结束符*/
  app_in_pkg->frm_end =*(msg+9+app_in_pkg->data_len);

}

/**************************************************************************************************
 * @fn          Debui_app_pkg
 *
 * @brief       将内部帧结构体的内容生成字节数组.在此过程中可以改变其控制码.
 *
 * @param       pApp_in_pkg   - 内部帧结构体
 *              new_ctr_code  - 新的控制码
 *              pOut_msg      - 输出的字节数组
 *
 * @return      输出的字节数组长度
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
 * @brief       构建77帧。77帧数据域长度为两个字节。
 *     
 * @data        2015年08月07日
 *     
 * @param       msg        - 帧字节数组
 *              app_in_pkg - 转换后的结构体数据      
 *     
 * @return      void
 ***************************************************************************
 */ 
void bui_77_app_pkg(uint8_t *msg,app_77_pkg_t *app_in_pkg)
{
	uint16_t msgIndex = 0;
  //app_pkg_t app_in_pkg;
  /*构造帧起始符1*/
  app_in_pkg->frm_head  =*msg;//
  /*构造帧地址a0-A1*/
  app_in_pkg->ctr_code  =*(msg+1);
  app_in_pkg->ed_addr   =*(msg+2);
  app_in_pkg->ed_addr   = (app_in_pkg->ed_addr<<8) |  *(msg+3);
  app_in_pkg->ap_addr   =*(msg+4);
  app_in_pkg->ap_addr   =(app_in_pkg->ap_addr <<8) | *(msg+5);
  app_in_pkg->frm_cnt   = *(msg+6);
 
  /*构造帧数据域长度*/
  app_in_pkg->data_len  =((*(msg+7)&0xffff)<<8) | *(msg+8);
 
  /*构造帧数据域*/
  for( msgIndex=0;msgIndex<app_in_pkg->data_len;msgIndex++)
    app_in_pkg->msg[msgIndex]  =*(msg+9+msgIndex);
  /*构造帧校验码CS*/
  app_in_pkg->cs=*(msg+9+app_in_pkg->data_len);
 
  /*构造帧结束符*/
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
   
  *(out_pkg+12+para_len)=gen_crc(out_pkg,12+para_len);//CS校验码
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
   
  *(out_pkg+12+para_len)=gen_crc(out_pkg,12+para_len);//CS校验码
  *(out_pkg+13+para_len)=0x16;
  
  return para_len+14;
}

/***************************************************************************
 * @fn          bui_rep_77_command
 *     
 * @brief       构建77帧的命令回复帧
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg     - 输入的77帧
 *              reply_para - 回复内容
 *              para_len   - 回复内容长度
 *              out_pkg    - 返回的字节数组
 *     
 * @return      回复帧长度
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
   
  *(out_pkg+13+para_len)=gen_crc(out_pkg,13+para_len);//CS校验码
  *(out_pkg+14+para_len)=0x16;
  
  return para_len+15;
}

/***************************************************************************
 * @fn          bui_rep_88_command
 *     
 * @brief       构建88帧的命令回复帧
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg     - 输入的88帧
 *              reply_para - 回复内容
 *              para_len   - 回复内容长度
 *              out_pkg    - 返回的字节数组
 *     
 * @return      回复帧长度
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
   
  *(out_pkg+12+para_len)=gen_crc(out_pkg,12+para_len);//CS校验码
  *(out_pkg+13+para_len)=0x16;
  
  return para_len+14;
}


/***************************************************************************
 * @fn          bui_pkg_ack
 *     
 * @brief       构建68、88帧的应答帧
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg     - 输入的68、88帧
 *              out_pkg    - 返回的字节数组
 *     
 * @return      回复帧长度
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
    
   
  *(out_pkg+8)=gen_crc(out_pkg,8);//CS校验码
  *(out_pkg+9)=0x16; 
  return 10;
}

/***************************************************************************
 * @fn          bui_77_pkg_ack
 *     
 * @brief       构建77帧的应答帧
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg     - 输入的77帧
 *              out_pkg    - 返回的字节数组
 *     
 * @return      回复帧长度
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
   
  *(out_pkg+9)=gen_crc(out_pkg,9);//CS校验码
  *(out_pkg+10)=0x16; 
  return 11;
}

/***************************************************************************
 * @fn          bui_pkg_alarm
 *     
 * @brief       构建68帧的告警帧
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg     - 输入的88帧
 *              err_info   - 告警内容
 *              err_len    - 告警来源
 *              out_pkg    - 返回的字节数组
 *     
 * @return      告警帧长度
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
    *(out_pkg+8+err_len)=gen_crc(out_pkg,8+err_len);//CS校验码
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
    *(out_pkg+8+err_len)=gen_crc(out_pkg,8+err_len);//CS校验码
    *(out_pkg+9+err_len)=0x16;
    return 10+err_len;  
  } 
}

/***************************************************************************
 * @fn          bui_check_link_pkg
 *     
 * @brief       构建检测节点是否在线帧
 *     
 * @data        2015年08月07日
 *     
 * @param       ed_id   - 节点ID号
 *              ed_addr - 节点地址
 *              out_pkg - 构建完成后的输出帧
 *     
 * @return      帧长度
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
   
  *(out_pkg+14)=gen_crc(out_pkg,14);//CS校验码
  *(out_pkg+15)=0x16;
  
  return 16;
}


/***************************************************************************
 * @fn          bui_time_sync_pkg
 *     
 * @brief       构建同步帧，用于对节点进行时间同步
 *     
 * @data        2015年08月07日
 *     
 * @param       out_pkg - 构建完成后的输出帧
 *     
 * @return      帧长度
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
 * @brief       产生CRC校验码
 *     
 * @data        2015年08月07日
 *     
 * @param       frm  - 输入数据
 *              len  - 输入数据长度
 *     
 * @return      校验码(一个字节)
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

