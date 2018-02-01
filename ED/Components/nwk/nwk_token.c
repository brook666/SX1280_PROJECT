/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   addr_control_fun.c
// Description: User APP.
//              Join Token��Link Token����.
// Author:      Zengjia
// Version:     1.0
// Date:        2015-9-24
// History:     2015-9-24   Leidi   ��ʼ�汾
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "nwk_token.h"
#include "nwk_api.h"
#include "nwk_globals.h"
#include "nwk.h"
#include <string.h>


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
/* FLASH�б������Ϣλ��(ע�ⵥƬ���ͺ�) */
#define JOIN_TOKEN_PTR       0x0801a006   //Join Token(4�ֽ�)
#define LINK_TOKEN_PTR       0x0801a00a //Link Token(4�ֽ�)


/**************************************************************************************************
 * @fn          Load_JoinToken
 *
 * @brief       ����FLASH�е�JoinToken.���FLASH�еĲ���������,���޲���.�����ڸ�������ʼ��ǰ����.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Load_JoinToken(void)
{
  if(JoinTokenInFlashIsValid())//FLASH�еı�����ַ��Ч
  {
    UseJoinTokenInFlash();//ʹ��FLASH�еı�����ַ
  }
}

/**************************************************************************************************
 * @fn          JoinTokenInFlashIsValid
 *
 * @brief       �ж�FLASH�е�JoinToken�Ƿ���Ч.
 *
 * @param       none
 *
 * @return      0 - ��Ч����ַ������
 *              1 - ��Ч��ַ
 **************************************************************************************************
 */
uint8_t JoinTokenInFlashIsValid(void)
{
	uint32_t joinToken = 0;;
  /* ������ַ�ĵ�һ�ֽڲ���Ϊ0x00��0xFF */
  if((*(uint8_t *)JOIN_TOKEN_PTR == 0x00) || *(uint8_t *)JOIN_TOKEN_PTR == 0xFF)
  {
     return 0;
  }

  return memcmp((const uint32_t *)JOIN_TOKEN_PTR, &joinToken, sizeof(int));//FLASH�е�JoinToken��Ϊ0����Ч
}

/**************************************************************************************************
 * @fn          UseJoinTokenInFlash
 *
 * @brief       ����FLASH�е�JoinToken.
 *
 * @param       none
 *
 * @return      SMPL_SUCCESS - �����ɹ�
 **************************************************************************************************
 */
smplStatus_t UseJoinTokenInFlash(void)
{
  /* �˲���������SMPL_Init()ǰִ�вŻ�ɹ� */
  ioctlToken_t token ;
  token.tokenType = TT_JOIN;
  
//�����Ѵ�˷�ʽ�洢��STM8���ڴ����ҲΪ���ģʽ  
  memcpy(&token.token,(void *)JOIN_TOKEN_PTR,4);

  return SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_SET,(void *)&token);
}

/**************************************************************************************************
 * @fn          WriteMacAddressToFlash
 *
 * @brief       ��FLASH��д��JoinToken.
 *
 * @param       ָ��Ҫд��ı�����ַ��ָ��
 *
 * @return      none
 **************************************************************************************************
 */
void WriteJoinTokenToFlash(const uint32_t joinToken)
{
  uint32_t token;
	//token = htonl(joinToken);
  FLASH_WriteByte(JOIN_TOKEN_PTR,(uint8_t*)&token,sizeof(uint32_t));
}

/**************************************************************************************************
 * @fn          CleanJoinTokenInFlash
 *
 * @brief       ���FLASH�е�JoinToken.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void CleanJoinTokenInFlash(void)
{
  uint32_t token = {0};
    
  FLASH_WriteByte(JOIN_TOKEN_PTR,(uint8_t *)(&token),sizeof(uint32_t));
}



/**************************************************************************************************
 * @fn          Load_LinkToken
 *
 * @brief       ����FLASH�е�LinkToken.���FLASH�еĲ���������,���޲���.�����ڸ�������ʼ��ǰ����.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Load_LinkToken(void)
{
  if(LinkTokenInFlashIsValid())//FLASH�еı�����ַ��Ч
  {
    UseLinkTokenInFlash();//ʹ��FLASH�еı�����ַ
  }
}

/**************************************************************************************************
 * @fn          LinkTokenInFlashIsValid
 *
 * @brief       �ж�FLASH�е�LinkToken�Ƿ���Ч.
 *
 * @param       none
 *
 * @return      0 - ��Ч����ַ������
 *              1 - ��Ч��ַ
 **************************************************************************************************
 */
uint8_t LinkTokenInFlashIsValid(void)
{
	uint32_t linkToken = 0;
  /* ������ַ�ĵ�һ�ֽڲ���Ϊ0x00��0xFF */
  if((*(uint8_t *)LINK_TOKEN_PTR == 0x00) || *(uint8_t *)LINK_TOKEN_PTR == 0xFF)
  {
     return 0;
  }

  return memcmp((const uint32_t *)LINK_TOKEN_PTR, &linkToken, sizeof(int));//FLASH�е�JoinToken��Ϊ0����Ч
}

/**************************************************************************************************
 * @fn          UseLinkTokenInFlash
 *
 * @brief       ����FLASH�е�LinkToken.
 *
 * @param       none
 *
 * @return      SMPL_SUCCESS - �����ɹ�
 **************************************************************************************************
 */
smplStatus_t UseLinkTokenInFlash(void)
{
  /* �˲���������SMPL_Init()ǰִ�вŻ�ɹ� */
  ioctlToken_t token ;
  token.tokenType = TT_LINK;
  
//������С�˷�ʽ��ʵ����STM8���ڴ����Ϊ���ģʽ  
  memcpy((void*)&token.token.linkToken,(void *)LINK_TOKEN_PTR,sizeof(uint32_t));
  //token.token.linkToken = swsn_ntohl(token.token.linkToken);
  
  return SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_SET,(void *)&token);
}

/**************************************************************************************************
 * @fn          WriteLinkTokenToFlash
 *
 * @brief       ��FLASH��д��LinkToken.
 *
 * @param       ָ��Ҫд��ı�����ַ��ָ��
 *
 * @return      none
 **************************************************************************************************
 */
void WriteLinkTokenToFlash(const uint32_t linkToken)
{
  uint32_t token;
	//token = htonl(linkToken);
  FLASH_WriteByte(LINK_TOKEN_PTR,(uint8_t*)&token,sizeof(uint32_t));
}

/**************************************************************************************************
 * @fn          CleanLinkTokenInFlash
 *
 * @brief       ���FLASH�е�LinkToken.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void CleanLinkTokenInFlash(void)
{
  uint32_t token = {0};
    
  FLASH_WriteByte(LINK_TOKEN_PTR,(uint8_t *)(&token),sizeof(uint32_t));
}

/**************************************************************************************************
 */
