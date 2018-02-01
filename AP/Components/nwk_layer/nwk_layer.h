#ifndef _NWK_LAYER_H
#define _NWK_LAYER_H

#include <stdint.h>
#include "smpl_config.h"

typedef uint8_t linkID_t;
#define SMPL_LINKID_USER_UUD    ((linkID_t) ~0)

typedef struct
{
  uint8_t  addr[MAC_ADDR_SIZE]; //MAC_ADDR_SIZE=4 
} addr_t;

typedef struct
{
  uint8_t  type[MAC_TYPE_SIZE];//MAC_TYPE_SIZE=2
} type_t;

typedef struct
{  
  addr_t  macAddr;  
  type_t  macType; 
} mac_addr_t; //mac地址

#define NET_IP_ADDR_SIZE   4
#define NET_AP_IP_ADDR_SIZE   2
#define NET_ED_IP_ADDR_SIZE   2

typedef struct
{
  uint8_t  edAddr[NET_ED_IP_ADDR_SIZE];//节点地址
  uint8_t  apAddr[NET_AP_IP_ADDR_SIZE];//集中器地址
} ip_addr_t;

extern uint8_t nwk_resetUnlinkedED(void);// 删除sPersistInfo信息中只join而没有link的信息。
extern uint8_t nwk_resetlinkedED(void);//删除sPersistInfo信息中LINK节点的信息。
extern void sysinfo_to_flash(void);//将集中器中存储的节点信息(sPersistInfo结构体信息)写入flash, flash预留空间为4K。
extern void sysinfo_from_flash(void);// 集中器上电或复位后，从flash中读出存储的节点信息
extern uint16_t nwk_getAPandTotalEDInfor(uint8_t *);//获取集中器地址、所有节点的IP地址、MAC地址及状态， 并依次存放到数组里面

extern int8_t  nwk_joinList_add(const mac_addr_t *);//向许可Join的设备列表中添加一个设备地址.
extern int8_t  nwk_joinList_del(const addr_t *);//从许可Join的设备列表中删除一个设备地址.
extern void    nwk_joinList_save(void);//将RAM中的许可Join的设备列表保持到FLASH.
extern uint8_t nwk_joinList_searchIP(const addr_t *,ip_addr_t *);//使用给定的设备地址在许可Join的设备列表中查找对应设备的IP地址.

extern linkID_t nwk_getLinkIdByAddress(addr_t *addr);//通过节点的MAC地址获得其LID.
extern addr_t  *nwk_getAddressByLinkId(linkID_t linkid);//通过节点的LID获得MAC地址.

extern uint8_t nwk_getEDState(uint8_t *pInNetED,uint8_t *pOutNetED);//获取在网节点数、离线节点数及节点总数
extern uint8_t nwk_getEDbasicInfo(linkID_t linkID,uint8_t * ed_info);//获得节点基本信息
extern addr_t const *nwk_getMyAddress(void);//获取设备地址
extern uint8_t nwk_setMyAddress(addr_t *addr);//设置地址
extern uint8_t nwk_setMyType(type_t *type);//设置类型
extern type_t const  *nwk_getMyType(void);//获取设备类型
extern uint8_t nwk_QfindAppFrame(void);//判断输入队列中是否有数据.不对网络层帧和广播帧进行判断。
extern linkID_t nwk_getDuplicateLinkLid(void);// 获取重新与网关连接的节点的lid
extern void     nwk_clearDuplicateLinkFlag(void);//清除重新与网管连接的节点标志
extern uint8_t  nwk_isExistDuplicateLinkED(void);//存在重新连接的节点
extern uint8_t nwk_getFrameReceiveCount(void);//获取网络层有效的帧计数

extern int8_t nwk_getRemoterRssi(void);//获取遥控器的RSSI值.
extern uint8_t nwk_getEDRssi(linkID_t lid,int8_t *rssi);//获得节点RSSI
extern void SWSN_DELAY(uint16_t milliseconds );//延时
extern void SWSN_init(uint8_t (*f)(linkID_t,uint8_t));//SWSN初始化
extern uint8_t SWSN_LinkListen(linkID_t *linkID);//SWSN监听
extern uint8_t SWSN_DeleteEDByLid(linkID_t *lid);//通过lid删除节点
extern uint8_t SWSN_DeleteEDByMAC(addr_t mac);//通过Mac删除节点

extern uint8_t recv_user_msg(linkID_t lid, uint8_t *msg, uint8_t *len);//从指定ID读用户数据
extern uint8_t recv_bcast_msg(uint8_t *msg, uint8_t *len);//从广播端口读广播数据
extern uint8_t send_user_msg(linkID_t lid, uint8_t *msg, uint8_t len);//从指定ID发送数据
extern uint8_t send_bcast_msg(uint8_t *msg, uint8_t len);//发送广播数据
extern void SEGGER_RTT_print_sysinfo(void);





#endif














