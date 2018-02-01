/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   77_frm_proc.c
// Description: User APP.
//              用户与网关通信帧(0x77帧)处理.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-28
// History:     2014-10-28  Leidi 修改函数get_ap_ed_mac_ip(),增加节点状态字节并可选
//                                择获取节点信息时是否检测节点在网.
*****************************************************************************/

#include "77_frm_proc.h"
#include "bui_pkg_fun.h"
#include "nwk_layer.h"
#include "firmware_info_proc.h"

extern struct ip_mac_id *head_list;


/***************************************************************************
 * @fn          proc_77_frm
 *     
 * @brief       77帧数据处理
 *     
 * @data        2015年08月05日
 *     
 * @param       pInMsg  - 输入数据
 *              out_pkg - 输出数据
 *              len     - 数据包长度。函数入口处为输入数据的长度，
 *                        出口处为输出数据的长度
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
    proc_77_data_frm(pInMsg,out_pkg,len);//暂时没用
  }
  else if(frame_type == 0x01)
  {
    proc_77_command_frm(pInMsg,out_pkg,len);//命令帧
  }
  else if(frame_type == 0x02)
  {
    proc_77_ack_frm(pInMsg,out_pkg,len);//暂时没用
  }
  else if(frame_type == 0x03)
  {
    proc_77_alarm_frm(pInMsg,out_pkg,len);//暂时没用
  }
  else
  {
  }
}


/***************************************************************************
 * @fn          proc_77_data_frm
 *     
 * @brief       77帧数据帧处理
 *     
 * @data        2015年08月05日
 *     
 * @param       pInMsg  - 输入数据
 *              out_pkg - 输出数据
 *              len     - 数据包长度。函数入口处为输入数据的长度，
 *                        出口处为输出数据的长度    
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
 * @brief       77帧命令帧处理
 *     
 * @data        2015年08月05日
 *     
 * @param       pInMsg  - 输入数据
 *              out_pkg - 输出数据
 *              len     - 数据包长度。函数入口处为输入数据的长度，
 *                        出口处为输出数据的长度    
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
 * @brief       77帧应答帧处理
 *     
 * @data        2015年08月05日
 *     
 * @param       pInMsg  - 输入数据
 *              out_pkg - 输出数据
 *              len     - 数据包长度。函数入口处为输入数据的长度，
 *                        出口处为输出数据的长度    
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
 * @brief       77帧告警帧处理
 *     
 * @data        2015年08月05日
 *     
 * @param       pInMsg  - 输入数据
 *              out_pkg - 输出数据
 *              len     - 数据包长度。函数入口处为输入数据的长度，
 *                        出口处为输出数据的长度    
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
 * @brief       查询集中器地址、节点地址和节点状态。前两个字节是集中器IP地址，
 *              第三个字节是节点总数，后面是每7个一个节点信息，前4个字节是节点
 *              mac地址，之后2个字节是节点IP地址，最后1个字节是表示节点状态。
 *              节点状态字节含义如下:
 *                  Bit T7  T6  T5  T4  T3  T2  T1       T0
 *                  0   -   -   -   -   -   -  不在网    IP无冲突
 *                  1   -   -   -   -   -   -  在网      IP冲突
 *              注: （1）IP有冲突的节点不可访问，应尝试修改其IP。
 *                  （2）当不检测节点在网状态时，T1位无效。
 *                  （3）返回的地址均为低位在前，高位在后。例如返回的IP地址
 *                       信息为 5C 01，则实际IP地址为0x015C，返回的MAC地址
 *                       信息5C 01 01 FE，则实际MAC地址为0xFE01015C。
 *     
 * @data        2015年08月05日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度
 *     
 * @return      
 ***************************************************************************
 */ 
void get_ap_ed_mac_ip(app_77_pkg_t *in_pkg,uint8_t *out_pkg,uint16_t *len)
{
  uint16_t msg_len = 0;
  uint8_t msg_temp[NUM_CONNECTIONS * 7] ={0}; //每个节点信息包含7字节
  
  msg_len = nwk_getAPandTotalEDInfor(msg_temp);
  
  *len = bui_rep_77_command(in_pkg,msg_temp,msg_len,out_pkg);   
        
}

/***************************************************************************
 * @fn          get_gtway_type_mac_77
 *     
 * @brief       获取网关类型和网关地址。
 *     
 * @data        2015年08月05日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据。前两个字节为网关类型，后四个字节为网关地址。
 *              len     - 输出数据长度
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



