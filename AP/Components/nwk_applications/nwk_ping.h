/***************************************************************************
** �ļ���:  nwk_ping.h
** ������:  �׵�
** ��  ��:  2016��01��17��
** �޸���:  
** ��  ��:  
** ��  ��:  �����ping������غ���
**          
** ��  ��:  1.0
***************************************************************************/


#ifndef NWK_PING_H
#define NWK_PING_H

/* change the following as protocol developed */
#define MAX_PING_APP_FRAME    2

/* application payload offsets */
/*    both */
#define PB_REQ_OS     0
#define PB_TID_OS     1


/* ping requests */
#define PING_REQ_PING       1

/* prototypes */
fhStatus_t   nwk_processPing(mrfiPacket_t *);
void         nwk_pingInit(void);
smplStatus_t nwk_ping(linkID_t);


#endif



