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
} mac_addr_t; //mac��ַ

#define NET_IP_ADDR_SIZE   4
#define NET_AP_IP_ADDR_SIZE   2
#define NET_ED_IP_ADDR_SIZE   2

typedef struct
{
  uint8_t  edAddr[NET_ED_IP_ADDR_SIZE];//�ڵ��ַ
  uint8_t  apAddr[NET_AP_IP_ADDR_SIZE];//��������ַ
} ip_addr_t;

extern uint8_t nwk_resetUnlinkedED(void);// ɾ��sPersistInfo��Ϣ��ֻjoin��û��link����Ϣ��
extern uint8_t nwk_resetlinkedED(void);//ɾ��sPersistInfo��Ϣ��LINK�ڵ����Ϣ��
extern void sysinfo_to_flash(void);//���������д洢�Ľڵ���Ϣ(sPersistInfo�ṹ����Ϣ)д��flash, flashԤ���ռ�Ϊ4K��
extern void sysinfo_from_flash(void);// �������ϵ��λ�󣬴�flash�ж����洢�Ľڵ���Ϣ
extern uint16_t nwk_getAPandTotalEDInfor(uint8_t *);//��ȡ��������ַ�����нڵ��IP��ַ��MAC��ַ��״̬�� �����δ�ŵ���������

extern int8_t  nwk_joinList_add(const mac_addr_t *);//�����Join���豸�б������һ���豸��ַ.
extern int8_t  nwk_joinList_del(const addr_t *);//�����Join���豸�б���ɾ��һ���豸��ַ.
extern void    nwk_joinList_save(void);//��RAM�е����Join���豸�б��ֵ�FLASH.
extern uint8_t nwk_joinList_searchIP(const addr_t *,ip_addr_t *);//ʹ�ø������豸��ַ�����Join���豸�б��в��Ҷ�Ӧ�豸��IP��ַ.

extern linkID_t nwk_getLinkIdByAddress(addr_t *addr);//ͨ���ڵ��MAC��ַ�����LID.
extern addr_t  *nwk_getAddressByLinkId(linkID_t linkid);//ͨ���ڵ��LID���MAC��ַ.

extern uint8_t nwk_getEDState(uint8_t *pInNetED,uint8_t *pOutNetED);//��ȡ�����ڵ��������߽ڵ������ڵ�����
extern uint8_t nwk_getEDbasicInfo(linkID_t linkID,uint8_t * ed_info);//��ýڵ������Ϣ
extern addr_t const *nwk_getMyAddress(void);//��ȡ�豸��ַ
extern uint8_t nwk_setMyAddress(addr_t *addr);//���õ�ַ
extern uint8_t nwk_setMyType(type_t *type);//��������
extern type_t const  *nwk_getMyType(void);//��ȡ�豸����
extern uint8_t nwk_QfindAppFrame(void);//�ж�����������Ƿ�������.���������֡�͹㲥֡�����жϡ�
extern linkID_t nwk_getDuplicateLinkLid(void);// ��ȡ�������������ӵĽڵ��lid
extern void     nwk_clearDuplicateLinkFlag(void);//����������������ӵĽڵ��־
extern uint8_t  nwk_isExistDuplicateLinkED(void);//�����������ӵĽڵ�
extern uint8_t nwk_getFrameReceiveCount(void);//��ȡ�������Ч��֡����

extern int8_t nwk_getRemoterRssi(void);//��ȡң������RSSIֵ.
extern uint8_t nwk_getEDRssi(linkID_t lid,int8_t *rssi);//��ýڵ�RSSI
extern void SWSN_DELAY(uint16_t milliseconds );//��ʱ
extern void SWSN_init(uint8_t (*f)(linkID_t,uint8_t));//SWSN��ʼ��
extern uint8_t SWSN_LinkListen(linkID_t *linkID);//SWSN����
extern uint8_t SWSN_DeleteEDByLid(linkID_t *lid);//ͨ��lidɾ���ڵ�
extern uint8_t SWSN_DeleteEDByMAC(addr_t mac);//ͨ��Macɾ���ڵ�

extern uint8_t recv_user_msg(linkID_t lid, uint8_t *msg, uint8_t *len);//��ָ��ID���û�����
extern uint8_t recv_bcast_msg(uint8_t *msg, uint8_t *len);//�ӹ㲥�˿ڶ��㲥����
extern uint8_t send_user_msg(linkID_t lid, uint8_t *msg, uint8_t len);//��ָ��ID��������
extern uint8_t send_bcast_msg(uint8_t *msg, uint8_t len);//���͹㲥����
extern void SEGGER_RTT_print_sysinfo(void);





#endif














