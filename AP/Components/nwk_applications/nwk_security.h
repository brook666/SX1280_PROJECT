/***************************************************************************
** �ļ���:  nwk_security.h
** ������:  �׵�
** ��  ��:  2016��01��17��
** �޸���:  
** ��  ��:  
** ��  ��:  �����ӽ�����غ���
**          
** ��  ��:  1.0
***************************************************************************/


#ifndef NWK_SECURITY_H
#define NWK_SECURITY_H

/* change the following as Security application is developed */
#define MAX_SEC_APP_FRAME    0

/* prototypes  */
void       nwk_securityInit(void);
fhStatus_t nwk_processSecurity(mrfiPacket_t *);
void       nwk_setSecureFrame(mrfiPacket_t *, uint8_t, uint32_t *);
uint8_t    nwk_getSecureFrame(mrfiPacket_t *, uint8_t, uint32_t *);
#endif



