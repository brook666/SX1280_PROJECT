/***************************************************************************
** 文件名:  nwk_api.h
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  网络层基本的API函数
**          
** 版  本:  1.0
***************************************************************************/

#ifndef NWK_API_H
#define NWK_API_H

#include "nwk_types.h"
#include "nwk_frame.h"
#include "nwk_QMgmt.h"

/* Tx options (bit map) */
#define  SMPL_TXOPTION_NONE       ((txOpt_t)0x00)
#define  SMPL_TXOPTION_ACKREQ     ((txOpt_t)0x01)

/* 设置与获取射频参数 */
#define  SMPL_GET_RADIO_PROPERTY   Mrfi_GetProperty
#define  SMPL_SET_RADIO_PROPERTY   Mrfi_SetProperty

/* 设置与获取信道号 */
#define  SMPL_SET_LOGICAL_CHANNEL  MRFI_SetChannelNum
#define  SMPL_GET_LOGICAL_CHANNEL  MRFI_GetChannelNum

/* 获取射频芯片的基本信息 */
#define  SMPL_GET_RADIO_PART_INFO  Mrfi_GetPartInfo

/*判断队列中是否有应用层数据帧*/
#define  SMPL_QFIND_APP_FRAME      nwk_QfindAppFrame

smplStatus_t SMPL_Init(uint8_t (*)(linkID_t,uint8_t));
smplStatus_t SMPL_Link(linkID_t *);
smplStatus_t SMPL_LinkListen(linkID_t *);
smplStatus_t SMPL_Send(linkID_t lid, uint8_t *msg, uint8_t len);
smplStatus_t SMPL_SendOpt(linkID_t lid, uint8_t *msg, uint8_t len, txOpt_t);
smplStatus_t SMPL_Receive(linkID_t lid, uint8_t *msg, uint8_t *len);
smplStatus_t SMPL_Ioctl(ioctlObject_t, ioctlAction_t, void *);
#ifdef EXTENDED_API
smplStatus_t SMPL_Ping(linkID_t);
smplStatus_t SMPL_Unlink(linkID_t);
smplStatus_t SMPL_Commission(addr_t *, uint8_t, uint8_t, linkID_t *);
#endif  /* EXTENDED_API */
int8_t nwk_getRemoterRssi(void);
uint8_t nwk_getEDRssi(linkID_t lid,int8_t *rssi);
uint8_t recv_user_msg(linkID_t lid, uint8_t *msg, uint8_t *len);
uint8_t recv_bcast_msg(uint8_t *msg, uint8_t *len);
uint8_t send_user_msg(linkID_t lid, uint8_t *msg, uint8_t len);
uint8_t send_bcast_msg(uint8_t *msg, uint8_t len);

void SWSN_DELAY(uint16_t milliseconds );
void SWSN_init(uint8_t (*f)(linkID_t,uint8_t));
void sysinfo_to_flash(void);
void sysinfo_from_flash(void);
uint8_t SWSN_LinkListen(linkID_t *linkID);
uint8_t SWSN_DeleteEDByLid(linkID_t *lid);
uint8_t SWSN_DeleteEDByMAC(addr_t mac);
void refresh_info_in_sysinfo(void);
void SEGGER_RTT_print_sysinfo(void);

#endif
