/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   addr_control_fun.c
// Description: User APP.
//              Join Token和Link Token管理.
// Author:      Zengjia
// Version:     1.0
// Date:        2015-9-24
// History:     2015-9-24   Leidi   初始版本
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "token_control_fun.h"
#include "nwk_api.h"
#include "nwk_globals.h"
#include "nwk.h"
#include <string.h>


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
/* FLASH中保存的信息位置(注意单片机型号) */
#define JOIN_TOKEN_PTR       0x0801d80c   //Join Token(4字节)
#define LINK_TOKEN_PTR       0x0801d810   //Link Token(4字节)


/**************************************************************************************************
 * @fn          Load_JoinToken
 *
 * @brief       加载FLASH中的JoinToken.如果FLASH中的参数不可用,则无操作.必须在个域网初始化前运行.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Load_JoinToken(void)
{
  if(JoinTokenInFlashIsValid())//FLASH中的本机地址有效
  {
    UseJoinTokenInFlash();//使用FLASH中的本机地址
  }
}

/**************************************************************************************************
 * @fn          JoinTokenInFlashIsValid
 *
 * @brief       判断FLASH中的JoinToken是否有效.
 *
 * @param       none
 *
 * @return      0 - 无效，地址不存在
 *              1 - 有效地址
 **************************************************************************************************
 */
uint8_t JoinTokenInFlashIsValid(void)
{
  uint32_t joinToken = 0;
  /* 本机地址的第一字节不能为0x00或0xFF */
  if((*(uint8_t *)JOIN_TOKEN_PTR == 0x00) || *(uint8_t *)JOIN_TOKEN_PTR == 0xFF)
  {
     return 0;
  }

  return memcmp((const uint32_t *)JOIN_TOKEN_PTR, &joinToken, sizeof(int));//FLASH中的JoinToken若为0则无效
}

/**************************************************************************************************
 * @fn          UseJoinTokenInFlash
 *
 * @brief       加载FLASH中的JoinToken.
 *
 * @param       none
 *
 * @return      SMPL_SUCCESS - 操作成功
 **************************************************************************************************
 */
smplStatus_t UseJoinTokenInFlash(void)
{
  /* 此操作必须在SMPL_Init()前执行才会成功 */
  ioctlToken_t token ;
  token.tokenType = TT_JOIN;
  
//数据已大端方式存储，STM8的内存管理也为大端模式  
  memcpy(&token.token,(void *)JOIN_TOKEN_PTR,4);
//  token.token.joinToken = *(uint32_t*)JOIN_TOKEN_PTR;
//  token.token.joinToken = ntohl(token.token.joinToken);
  
  return SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_SET,(void *)&token);
}

/**************************************************************************************************
 * @fn          WriteMacAddressToFlash
 *
 * @brief       在FLASH中写入JoinToken.
 *
 * @param       指向要写入的本机地址的指针
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
 * @brief       清除FLASH中的JoinToken.
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
 * @brief       加载FLASH中的LinkToken.如果FLASH中的参数不可用,则无操作.必须在个域网初始化前运行.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Load_LinkToken(void)
{
  if(LinkTokenInFlashIsValid())//FLASH中的本机地址有效
  {
    UseLinkTokenInFlash();//使用FLASH中的本机地址
  }
}

/**************************************************************************************************
 * @fn          LinkTokenInFlashIsValid
 *
 * @brief       判断FLASH中的LinkToken是否有效.
 *
 * @param       none
 *
 * @return      0 - 无效，地址不存在
 *              1 - 有效地址
 **************************************************************************************************
 */
uint8_t LinkTokenInFlashIsValid(void)
{
  uint32_t linkToken = 0;
  /* 本机地址的第一字节不能为0x00或0xFF */
  if((*(uint8_t *)LINK_TOKEN_PTR == 0x00) || *(uint8_t *)LINK_TOKEN_PTR == 0xFF)
  {
     return 0;
  }

  return memcmp((const uint32_t *)LINK_TOKEN_PTR, &linkToken, sizeof(int));//FLASH中的JoinToken若为0则无效
}

/**************************************************************************************************
 * @fn          UseLinkTokenInFlash
 *
 * @brief       加载FLASH中的LinkToken.
 *
 * @param       none
 *
 * @return      SMPL_SUCCESS - 操作成功
 **************************************************************************************************
 */
smplStatus_t UseLinkTokenInFlash(void)
{
  /* 此操作必须在SMPL_Init()前执行才会成功 */
  ioctlToken_t token ;
  token.tokenType = TT_LINK;
  
//数据已小端方式，实际上STM8的内存管理为大端模式  
  memcpy((void*)&token.token.linkToken,(void *)LINK_TOKEN_PTR,sizeof(uint32_t));
  //token.token.linkToken = ntohl(token.token.linkToken);
  
  return SMPL_Ioctl(IOCTL_OBJ_TOKEN,IOCTL_ACT_SET,(void *)&token);
}

/**************************************************************************************************
 * @fn          WriteLinkTokenToFlash
 *
 * @brief       在FLASH中写入LinkToken.
 *
 * @param       指向要写入的本机地址的指针
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
 * @brief       清除FLASH中的LinkToken.
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
