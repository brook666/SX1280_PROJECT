/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   token_control_fun.h
// Description: User APP.
//              设备地址管理接口.
// Author:      Leidi
// Version:     3.0
// Date:        2015-9-24
// History:     2015-9-24   初始版本

*****************************************************************************/
#ifndef _TOKEN_CONTROL_FUN_H
#define _TOKEN_CONTROL_FUN_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * -------------------------------------------------------------------------------- ----------------
 */
#include "nwk_types.h"
#include "stm32_flash.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Typdefs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
/* */
void Load_JoinToken(void);
void Load_LinkToken(void);
uint8_t JoinTokenInFlashIsValid(void);
uint8_t LinkTokenInFlashIsValid(void);
smplStatus_t UseLinkTokenInFlash(void);
smplStatus_t UseJoinTokenInFlash(void);

void WriteJoinTokenToFlash(const uint32_t );
void WriteLinkTokenToFlash(const uint32_t );

void CleanJoinTokenInFlash(void);
void CleanLinkTokenInFlash(void);


#endif

/**************************************************************************************************
 */