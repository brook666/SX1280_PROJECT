/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_status_fun.c
// Description: User APP.
//              SWSN网络状态处理.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-27
// History:     2014-10-27  Leidi 修改函数modify_ip_addr()的实现逻辑.
//              2014-10-29  Leidi 修改函数delete_ed_connect()的实现逻辑.
//              2015-01-22  Leidi 修改函数get_join_link_token():调整字节序.
//                                修改函数get_ed_info():调整MAC的字节序.
//                                修改函数get_ap_mac():改为汇报4字节MAC.
//                                删除对全局变量的引用:sLinkToken,sJoinToken.
//              2015-05-13  Leidi 增加函数原型set_join_ed().
//              2015-05-14  Leidi 删除节点时，也删除许可Join的设备列表中对应的设备.
*****************************************************************************/

#include <string.h>
#include "nwk_layer.h"
#include "bsp.h"
#include "mrfi_spi.h"
#include "net_status_fun.h"
#include "ap_inner_fun.h"
#include "68_frm_proc.h"
#include "net_list_proc.h"

extern volatile uint8_t bsp_reset_ap_config ;
uint8_t reset_system_flag = 0;
extern  uint8_t net_ed_num;
extern int32_t remoter_press_num ;
/***************************************************************************
 * @fn          net_status_proc
 *     
 * @brief       组网状态处理入口，根据DI2确定，DI2 = 0x02。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度     
 *     
 * @return      APP_SUCCESS - 成功
 *              APP_ERR     - 失败
 ***************************************************************************
 */ 
app_status_t net_status_proc(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  
  app_net_status_t  status_rc= APP_NET_STATUS_SUCCESS;
  uint8_t  msg_id_1 = in_pkg->msg[2];    
  uint8_t  alarm_info[4];
  
  switch(msg_id_1)
  {
    case 0x01:  status_rc = reset_system(in_pkg, out_pkg, len);//系统复位
                break; 
    case 0x03:  status_rc = get_join_link_token(in_pkg, out_pkg, len);//读取加入令牌和连接令牌,
                break;
    case 0x10:  status_rc = get_net_info(in_pkg, out_pkg, len);//查询网络当前节点数
                break;
    case 0x11:  status_rc = get_ed_info(in_pkg, out_pkg, len);// 查询集中器中记录的节点信息
                break;
    case 0x13:  status_rc = set_join_ed(in_pkg, out_pkg, len);
                break;
    case 0x20:  status_rc = get_ap_mac(in_pkg, out_pkg, len);//读取集中器MAC地
                break;
    case 0x40:  status_rc = reset_ap(in_pkg, out_pkg, len);//恢复集中器到出厂设置
                break;
    case 0x90:  status_rc = get_ed_mac_and_ip(in_pkg, out_pkg, len);//查询目标节点的MAC和IP
                break;
    case 0x91:  status_rc = modify_ip_addr(in_pkg, out_pkg, len);//修改目标节点IP地址
                break;
    case 0x92:  status_rc = delete_ed_connect(in_pkg, out_pkg, len);// 删除目标节点
                break;
    case 0xa0:  status_rc = read_temoter_press_num(in_pkg, out_pkg, len);
                break;
    default:    status_rc = no_status_fun(in_pkg, out_pkg, len);
  }
       
  if(status_rc ==APP_NET_STATUS_SUCCESS)
  {  
    return APP_SUCCESS;
  }
  else if(status_rc == APP_THIS_ED_IS_NOT_FOUND)
  {
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x02;
    alarm_info[2] = 0x02;
    alarm_info[3] = 0x05;
    *len =bui_pkg_alarm(in_pkg, alarm_info, 4, out_pkg); 
    
    return APP_ERR;      
  }
  else if((status_rc == APP_MODIFY_IP_ADDR_FAIL)||(status_rc == APP_IP_CONFLICT))
  {
    return APP_ERR;     
  }
  else
  {
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x02;
    alarm_info[2] = 0x02;
    alarm_info[3] = 0x08;
    *len =bui_pkg_alarm(in_pkg, alarm_info, 4, out_pkg);  
    
    return APP_ERR;
  }
}

/**************************************************************************************************
 * @fn          no_status_fun
 *
 * @brief       未使用的组网状态设置函数.
 *
 * @param       in_frame  - 输入的帧结构体
 *              out_msg   - 输出的字节数组
 *              out_len   - 输出的字节数组长度
 *
 * @return      NO_STATUS_FUN_FAIL     - 处理组网状态设置函数失败
 **************************************************************************************************
 */
app_net_status_t no_status_fun(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = NO_STATUS_FUN_FAIL;
   
  return rc;
}

/***************************************************************************
 * @fn          reset_system
 *     
 * @brief       读取加入令牌和连接令牌,DI1 = 0x03，DI0 = 0。前4字节是加入令牌
 *              (如FE 08 01 05) ，后4字节是连接令牌(如05 04 03 02) 。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS        - 成功
 *              APP_RESET_SYSTEM_FAIL         - 失败
 ***************************************************************************
 */ 
app_net_status_t reset_system(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_RESET_SYSTEM_FAIL;
  uint8_t  msg_id_0 = in_pkg->msg[3];
  
#ifdef DEBUG
  put_string_uart1("reset system!!!");
#endif
  
  if(msg_id_0 == 1)
  {

   *len = bui_rep_pkg_command(in_pkg, NULL, 0, out_pkg); 
    reset_system_flag = 1;
    rc= APP_NET_STATUS_SUCCESS;
  } 
  else
  {
  }
  
  return rc;
}
/***************************************************************************
 * @fn          get_join_link_token
 *     
 * @brief       读取加入令牌和连接令牌,DI1 = 0x03，DI0 = 0。前4字节是加入令牌
 *              (如FE 08 01 05) ，后4字节是连接令牌(如05 04 03 02) 。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS        - 成功
 *              APP_GET_JOIN_LINK_TOKEN_FAIL  - 失败
 ***************************************************************************
 */ 
app_net_status_t get_join_link_token(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_GET_JOIN_LINK_TOKEN_FAIL;
  uint8_t  msg_id_0 = in_pkg->msg[3];
  uint8_t  join_link_token[8];
  uint32_t join_token = swsn_htonl(DEFAULT_JOIN_TOKEN);
  uint32_t link_token = swsn_htonl(DEFAULT_LINK_TOKEN);
  
#ifdef DEBUG
  put_string_uart1("enter get_join_link_token!!!");
#endif
  
  if(msg_id_0 == 0)
  {
    memcpy(&join_link_token[0], &join_token, sizeof(join_token));//加入令牌(如FE 08 01 05)
    memcpy(&join_link_token[4], &link_token, sizeof(link_token));//连接令牌(如05 04 03 02)

   *len = bui_rep_pkg_command(in_pkg, join_link_token, 8, out_pkg); 
    
    rc= APP_NET_STATUS_SUCCESS;
  } 
  else
  {
  }
  
  return rc;
}


/***************************************************************************
 * @fn          get_net_info (SWSN配置工具_1_0_0_40软件仍在使用,误删)
 *     
 * @brief       查询网络当前节点数,DI1 = 0x10，DI0 = 0。字节0：本网络所支持
 *              的最大节点数；字节1表已添加节点总数；字节2表已添加，在网节点
 *              数；字节3：不在网节点数。仅对集中器有效。       
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS - 成功
 *              APP_GET_NET_INFO_FAIL  - 失败
 ***************************************************************************
 */ 
app_net_status_t get_net_info(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_status_t  rc= APP_GET_NET_INFO_FAIL;
  uint8_t  msg_id_0 = in_pkg->msg[3];
  uint8_t  net_info[4];
  
  if(msg_id_0==0)
  {
    net_info[0] =  NUM_CONNECTIONS;
    net_info[1] =  nwk_getEDState(&net_info[2],&net_info[3]);

    *len = bui_rep_pkg_command(in_pkg,net_info,4,out_pkg);
    rc= APP_NET_STATUS_SUCCESS; 
  }
  else
  {
  }  
  return rc; 
}

/***************************************************************************
 * @fn          get_ed_info(现在不用)
 *     
 * @brief       查询集中器中记录的节点信息,DI1 = 0x11，DI0 = 0。对于请求帧：
 *              用户数据1个字节为目标节点序号。对于回复帧：包含9字节：字节0:
 *              节点号；字节1:节点状态(0-节点未链接,1-节点加入网络,2-节点已
 *              链接)；字节2:节点HOP数；字节3～6:MAC (如FE 08 03 01)；字节
 *              7～8:IP(如03 01)；字节9:RSSI；字节10:LQI。仅对集中器有效。
 *              MAC地址为高位在前，低位在后。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS   - 成功
 *              APP_GET_ED_INFO_FAIL     - 失败
 *              APP_THIS_ED_IS_NOT_FOUND - 失败，节点不存在
 ***************************************************************************
 */ 
app_net_status_t get_ed_info(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_GET_ED_INFO_FAIL;
  uint8_t   msg_id_0= in_pkg->msg[3];
  linkID_t  ed_lid = in_pkg->msg[4];
  uint8_t   ed_info[11]; 
  struct ip_mac_id *p = NULL;
  
  if(msg_id_0 == 0)
  {
    ed_info[0] = ed_lid;
    
    /* 通过LID查找节点信息 */
    if(nwk_getEDbasicInfo(ed_lid,&ed_info[1]) && psearch_id(ed_lid, &p))
    {
      ed_info[9] = ed_info[7];//RSSI
      ed_info[10] = ed_info[8];//LQI
      
      /* IP,汇报给上位机时交换顺序(如03 01)，因为帧结构中顺序如此 */
      ed_info[7] = p->ip[1];
      ed_info[8] = p->ip[0];
      
      rc = APP_NET_STATUS_SUCCESS;
      *len = bui_rep_pkg_command(in_pkg, ed_info, 11, out_pkg);
    }
    else
    {
      rc = APP_THIS_ED_IS_NOT_FOUND;
    }
  }
  else
  {
  }  
  return rc;
}
/***************************************************************************
 * @fn          get_ap_mac(SWSN配置工具_1_0_0_40软件仍在使用,误删)
 *     
 * @brief       读取集中器MAC地址，DI1 = 0x20，DI0 = 0。返回4字节，表示集中器
 *              MAC地址，形如FE 08 09 19。MAC地址为高位在前，低位在后。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS - 成功
 *              APP_GET_AP_MAC_FAIL    - 失败
 ***************************************************************************
 */ 
app_net_status_t get_ap_mac(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_GET_AP_MAC_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];
  uint8_t ap_mac[4];
  addr_t const *myromaddr;
  uint8_t i;
  
  
  if(msg_id_0==0)
  {
    /* 获取本机的MAC(如19 09 08 FE) */
    myromaddr = nwk_getMyAddress();
    
    /* AP的MAC地址以逆序汇报(如FE 08 09 19) */
    for(i = 0; i < 4; i++)
    {
      ap_mac[i] = myromaddr->addr[3-i];
    }
    
    rc= APP_NET_STATUS_SUCCESS;
    *len = bui_rep_pkg_command(in_pkg, ap_mac, 4, out_pkg);
  }
  else
  {
  }
  return rc;
}
/***************************************************************************
 * @fn          set_join_ed(每添加一个节点，需要使用该函数)
 *     
 * @brief       扫描二维码，手动添加授权节点，DI1 = 0x13，DI0 = 0。发送时
 *              ABCD代表节点MAC地址，返回时A代码已添加的节点数，若返回值为
 *              255，则表示添加失败。只对集中器有效。MAC地址为低位在前，
 *              高位在后。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS - 成功
 *              APP_SET_JOIN_ED_FAIL   - 失败
 ***************************************************************************
 */ 
app_net_status_t set_join_ed(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t rc = APP_SET_JOIN_ED_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];
  mac_addr_t *joinAddr = NULL;
  uint8_t tmp[6];
  uint8_t i = 0;
  int8_t result = -1;
  
  if(msg_id_0 == 1)
  {
    if(in_pkg->data_len == 0x0a)
    {
      joinAddr = (mac_addr_t *)&in_pkg->msg[4];      
    }
    else if(in_pkg->data_len == 0x08)
    {
      for(i=0;i<4;i++)
      {
        tmp[i] = in_pkg->msg[4+i] ;
      }
      tmp[4] = in_pkg->msg[6];
      tmp[5] = in_pkg->msg[7];
      joinAddr = (mac_addr_t *)tmp;  
    }
    else
    {
    }
    
    if(-1 != (result = nwk_joinList_add(joinAddr)))
    {
      nwk_joinList_save();
    }
    memset(tmp,0x0,sizeof(tmp));
		tmp[0]=result;
		for(i=0;i<4;i++)
		{
			tmp[i+1] = in_pkg->msg[4+i];
		}
    rc = APP_NET_STATUS_SUCCESS;
    *len = bui_rep_pkg_command(in_pkg, tmp, 5, out_pkg);
  }
  else
  {
  }
  return rc;
}

/***************************************************************************
 * @fn          reset_ap
 *     
 * @brief       恢复集中器到出厂设置，DI1 = 0x40，DI0 = 1。集中器中保存的
 *              节点信息全部清除，集中器会重启。对节点无效。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS - 成功
 *              APP_RESET_AP_FAIL      - 失败
 ***************************************************************************
 */ 
app_net_status_t reset_ap(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_status_t  rc= APP_RESET_AP_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];    
  
  if(msg_id_0 == 0x01)
  {    
    bsp_reset_ap_config =0x01;
    rc= APP_NET_STATUS_SUCCESS; 
    *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);    
  }
  else
  {
  } 
  return rc;  
}

/***************************************************************************
 * @fn          get_ed_mac_and_ip
 *     
 * @brief       查询目标节点的MAC和IP，DI1 = 0x90，DI0 = 0。对于请求帧：用户
 *              数据1个字节（X）为目标节点序号。对于回复帧：前4个字节(ABCD)为
 *              节点MAC地址，后2个字节（XY）为节点IP地址。例如： 
 *              5C 01 01 FE + 5C 01。此查询对节点和集中器均有效。发送目标为
 *              节点时，目标节点序号可为任意值。节点序列号的合法范围是
 *              1~NUM_CONNECTIONS(网络容量)。返回信息会低位在前，高位在后。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS      - 成功
 *              APP_GET_ED_MAC_AND_IP_FAIL  - 失败
 ***************************************************************************
 */ 
app_net_status_t get_ed_mac_and_ip (app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_status_t  rc= APP_GET_ED_MAC_AND_IP_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3]; 
  uint8_t mac_and_ip[6];
  uint8_t i=0;
  
  if(msg_id_0 == 0x00)
  {
    struct ip_mac_id *p=NULL;
    
    if(psearch_id(in_pkg->msg[4], &p))//检测数据域：是否为链表中的LID
    {
      for(i=0;i<4;i++)
      {
        mac_and_ip[i] = p->mac[i];
      }
      for(i=0;i<2;i++)
      {
        mac_and_ip[i+4] = p->ip[i];
      }
      rc= APP_NET_STATUS_SUCCESS;
      *len = bui_rep_pkg_command(in_pkg,mac_and_ip,6,out_pkg); 
    }
    else
    {
    }    
  }
  else
  {
  }
  return rc;  
}

/***************************************************************************
 * @fn          modify_ip_addr
 *     
 * @brief       修改目标节点IP地址，DI1 = 0x91，DI0 = 1。前4个字节(ABCD)为目标
 *              节点MAC地址，后2个字节（XY）为要修改的IP地址，例如： 
 *              5C 01 01 FE + 22 11。只需发送给集中器，集中器确认信息正确后会发
 *              送给节点。若修改成功，应该收到2条返回帧，分别来自集中器与节点。
 *              IP地址分别保存于节点与集中器的FLASH中。MAC地址和IP地址均是低位
 *              在前，高位在后。注意，0x68帧节点地址域中的值实际上是节点IP(反转)
 *              地址，使用此帧修改目标节点IP地址之后，发送到节点的0x68帧的节点地
 *              址域都必须使用新的IP地址，否则无法与节点通信。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS   - 成功
 *              APP_MODIFY_IP_ADDR_FAIL  - 失败
 *              APP_IP_CONFLICT          - 新IP与已有IP冲突
 ***************************************************************************
 */ 
app_net_status_t modify_ip_addr(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t  rc= APP_MODIFY_IP_ADDR_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];    
  uint8_t alarm_info[10];
  
  if(msg_id_0 == 0x01)
  {
    addr_t mac_addr;
    uint16_t new_ip_addr;
    linkID_t lid;
    struct ip_mac_id *p = NULL;
    uint8_t send_pkg[MAX_APP_PAYLOAD];
    uint8_t send_pkg_len;
    uint8_t send_result;
    
    /* 从数据域中获取节点的MAC地址和新的IP */
    memcpy(mac_addr.addr, &in_pkg->msg[4], MAC_ADDR_SIZE);
    new_ip_addr = in_pkg->msg[8] | (in_pkg->msg[9]<<8);
    
    /* 获取节点MAC地址对应的LID */
    lid = nwk_getLinkIdByAddress(&mac_addr);
    
    /* 节点MAC地址存在已添加的节点列表中 */
    if(lid != 0)
    {
      /* 查找新的IP是否与已有的IP冲突 */
      if(psearch_ip(&new_ip_addr, &p))
      {
        /* 新的IP与已有的IP冲突 */
        rc = APP_IP_CONFLICT;
      }
      else
      {
        /* 构造向节点发送的命令帧,同时修盖其命令码 */
        send_pkg_len = Debui_app_pkg(in_pkg, 0x05, send_pkg);
        
        /* 向节点发送修改IP的命令帧 */
        send_result = send_net_pkg(lid, send_pkg, send_pkg_len);
        
        /* 向节点发送成功 */
        if(send_result == 1)
        {
          /* 修改节点信息链表中的IP地址 */
          Modify_IP_by_MAC(&mac_addr, new_ip_addr);
          
          rc= APP_NET_STATUS_SUCCESS;
        }
        
      }
    }
    
    if(rc!= APP_NET_STATUS_SUCCESS)
    { 
      alarm_info[0] = 0x00;
      alarm_info[1] = 0x02;      
      alarm_info[2] = 0x02;
      alarm_info[3] = 0x14;
      *len =bui_pkg_alarm(in_pkg, alarm_info, 4, out_pkg);  
    }
    else
    {   
      *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);    
    }   
  }
  return rc;  
}

/***************************************************************************
 * @fn          delete_ed_connect
 *     
 * @brief       删除目标节点，DI1 = 0x92，DI0 = 1。4个字节(ABCD)为目标节点MAC
 *              地址，例如5C 01 01 FE。只需发送给集中器，集中器确认信息正确后
 *              会发送给节点并删除节点信息。节点被删除后会断开连接，清除保存的
 *              集中器地址（恢复出厂状态），但不会重启。集中器在链表中查找指定
 *              的节点并删除、断开连接。MAC地址和IP地址均是低位在前，高位在后。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS - 成功
 *              其它                   - 失败
 ***************************************************************************
 */ 
app_net_status_t delete_ed_connect(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t  rc= APP_DELETE_ED_CONNECT_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];    
#ifdef DEBUG
  put_string_uart1("enter delete_ed_connect!!!");
#endif  
  
  if(msg_id_0 == 0x01)
  {
    addr_t mac_addr;
    linkID_t lid;
    uint8_t send_pkg[MAX_APP_PAYLOAD];
    uint8_t send_pkg_len;
    
    /* 从数据域中获取节点的MAC地址 */
    memcpy(mac_addr.addr, &in_pkg->msg[4], MAC_ADDR_SIZE);
    
    /* 获取节点MAC地址对应的LID */
    lid = nwk_getLinkIdByAddress(&mac_addr);

    /* 节点MAC地址存在已添加的节点列表中 */
    if(lid != 0)
    {
      /* 构造向节点发送的命令帧,同时修改其命令码 */
      send_pkg_len = Debui_app_pkg(in_pkg, 0x05, send_pkg);
        
      /* 向节点发送删除节点的命令帧 */
      send_net_pkg(lid, send_pkg, send_pkg_len);

      if(SWSN_deleteInnetED(mac_addr,lid))//删除链表中的节点成功
      {
        rc= APP_NET_STATUS_SUCCESS;
        *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);    
      }
      else
      {
      }   
    }
    else
    {
			//正常情况下，此处调用该函数，返回0;
			//此处调用，只是为了防止链表中存在没有删掉的数据。
			pdel_mac(mac_addr.addr);
			
			SWSN_DeleteEDByMAC(mac_addr);

      /*该节点没有加入网络，但是许可Join的设备列表中存在该节点信息,
        在许可Join的设备列表中删除对应的设备，并保存 */
      if(-1 != nwk_joinList_del(&mac_addr))
      {
        nwk_joinList_save();
      }
      rc= APP_NET_STATUS_SUCCESS;
      *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg); 
    }
  }
  else
  {
  }
  return rc;  
}


/***************************************************************************
 * @fn          delete_ed_connect
 *     
 * @brief       删除目标节点，DI1 = 0x92，DI0 = 1。4个字节(ABCD)为目标节点MAC
 *              地址，例如5C 01 01 FE。只需发送给集中器，集中器确认信息正确后
 *              会发送给节点并删除节点信息。节点被删除后会断开连接，清除保存的
 *              集中器地址（恢复出厂状态），但不会重启。集中器在链表中查找指定
 *              的节点并删除、断开连接。MAC地址和IP地址均是低位在前，高位在后。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度       
 *     
 * @return      APP_NET_STATUS_SUCCESS - 成功
 *              其它                   - 失败
 ***************************************************************************
 */ 
app_net_status_t read_temoter_press_num(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_net_status_t  rc= APP_DELETE_ED_CONNECT_FAIL;
  uint8_t msg_id_0 = in_pkg->msg[3];    

#ifdef DEBUG
  put_string_uart1("enter delete_ed_connect!!!");
#endif  
  
  if(msg_id_0 == 0x00)
  {
    uint8_t tmp[4] ={0};
    tmp[0] = remoter_press_num;
    tmp[1] = (remoter_press_num>>8);
    tmp[2] = (remoter_press_num>>16);
    tmp[3] = (remoter_press_num>>24);
    
    rc= APP_NET_STATUS_SUCCESS;
    *len = bui_rep_pkg_command(in_pkg,tmp,4,out_pkg);    
  }
  else
  {
  }
  return rc;  
}





