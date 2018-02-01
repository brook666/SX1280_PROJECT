/***************************************************************************
** �ļ���:  nwk_QMgmt.h
** ������:  �׵�
** ��  ��:  2016��01��17��
** �޸���:  
** ��  ��:  
** ��  ��:  �����֡���й���
**          
** ��  ��:  1.0
***************************************************************************/

#ifndef NWK_QMGMT_H
#define NWK_QMGMT_H

#include "nwk_types.h"
#include "nwk_frame.h"

#define  INQ   1
#define  OUTQ  2

#define  USAGE_NORMAL  1
#define  USAGE_FWD     2

/* prototypes */
void              nwk_QInit(void);
frameInfo_t *nwk_QfindSlot(uint8_t);
void         nwk_QadjustOrder(uint8_t, uint8_t);
frameInfo_t *nwk_QfindOldest(uint8_t, rcvContext_t *, uint8_t);
frameInfo_t *nwk_getQ(uint8_t);
uint8_t nwk_QfindAppFrame(void);

#endif  /* NWK_QMGMT_H */



