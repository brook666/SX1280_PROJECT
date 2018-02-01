/***************************************************************************
** �ļ���:  nwk_mgmt.h
** ������:  �׵�
** ��  ��:  2016��01��17��
** �޸���:  
** ��  ��:  
** ��  ��:  �洢/ת����ѯ�ڵ���Ϣ�Ļ�������
**          
** ��  ��:  1.0
***************************************************************************/


#ifndef NWK_MGMT_H
#define NWK_MGMT_H

/* MGMT frame application requests */
#define  MGMT_REQ_POLL        0x01

/* change the following as protocol developed */
#define MAX_MGMT_APP_FRAME    7

/* application payload offsets */
/*    both */
#define MB_APP_INFO_OS           0
#define MB_TID_OS                1

/*    Poll frame */
#define M_POLL_PORT_OS          2
#define M_POLL_ADDR_OS          3

/* change the following as protocol developed */
#define MAX_MGMT_APP_FRAME    7

/* frame sizes */
#define MGMT_POLL_FRAME_SIZE  7

/* prototypes */
void         nwk_mgmtInit(void);
fhStatus_t   nwk_processMgmt(mrfiPacket_t *);
smplStatus_t nwk_poll(uint8_t, uint8_t *);
void         nwk_resetSFMarker(uint8_t);

#endif


