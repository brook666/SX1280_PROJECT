/***************************************************************************
** �ļ���:  nwk_ioctl.h
** ������:  �׵�
** ��  ��:  2016��01��17��
** �޸���:  
** ��  ��:  
** ��  ��:  ������IO����
**          
** ��  ��:  1.0
***************************************************************************/



#ifndef NWK_IOCTL_H
#define NWK_IOCTL_H

#include "nwk_types.h"

/* prototypes */
smplStatus_t nwk_rawSend(ioctlRawSend_t *);
smplStatus_t nwk_rawReceive(ioctlRawReceive_t *);
smplStatus_t nwk_radioControl(ioctlAction_t, void *);
smplStatus_t nwk_deviceAddress(ioctlAction_t, addr_t *); 
smplStatus_t nwk_deviceType(ioctlAction_t action, type_t *type);
smplStatus_t nwk_deviceIPAddress(ioctlAction_t action, ip_addr_t *addr);
smplStatus_t nwk_connectionControl(ioctlAction_t, void *);
#ifdef ACCESS_POINT
smplStatus_t nwk_joinContext(ioctlAction_t);
#endif

#endif


