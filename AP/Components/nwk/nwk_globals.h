/***************************************************************************
** �ļ���:  nwk_globals.h
** ������:  �׵�
** ��  ��:  2016��01��17��
** �޸���:  
** ��  ��:  
** ��  ��:  �����ȫ�ֱ���
**          
** ��  ��:  1.0
***************************************************************************/

#ifndef NWK_GLOBALS_H
#define NWK_GLOBALS_H

#include "nwk_types.h"

/* Prototypes */
void           nwk_globalsInit(void);
addr_t const  *nwk_getMyAddress(void);
uint8_t        nwk_setMyAddress(addr_t *addr);
uint8_t        nwk_setMyType(type_t *type);
type_t const  *nwk_getMyType(void);
void           nwk_setAPAddress(addr_t *addr);
addr_t const  *nwk_getAPAddress(void);
addr_t const  *nwk_getBCastAddress(void);
addr_t const  *nwk_getRemoterAddress(void);
uint8_t const *nwk_getFWVersion(void);
uint8_t        nwk_getProtocolVersion(void);
ip_addr_t const *nwk_getMyIPAddress(void);
uint8_t nwk_setMyIPAddress(ip_addr_t *IPAddr);

#endif



