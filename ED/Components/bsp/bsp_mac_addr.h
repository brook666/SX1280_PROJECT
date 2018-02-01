/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   addr_control_fun.h
// Description: User APP.
//              设备地址管理接口.
// Author:      Leidi
// Version:     3.0
// Date:        2012-9-30
// History:     2014-9-30   Leidi 将AP地址读写,IP地址读写,本机MAC地址读写的函数
//                                原型由net_list_proc.h移入本文件. 节点端不再使
//                                用net_list_proc.h文件.
//              2015-06-16  Leidi 使用枚举定义模块信息.
//                                新增获取本模块信息的函数原型GetMyModuleInfo().
//              2015-07-29  Leidi 修改函数名,增加Load_IpAddress(),Get_IpAddress()和Load_MacAddress().
//              2015-09-06  Leidi 删除有关处理AP地址和IP地址的代码.
*****************************************************************************/
#ifndef _BSP_MAC_ADDR_H
#define _BSP_MAC_ADDR_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
//#include "stm8l15x.h"
#include "stm32_flash.h"


/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
/* 本机MAC地址读写 */
void Load_MacAddress(void);
uint8_t MacAddressInFlashIsValid(void);
uint8_t UseMacAddressInFlash(void);
void WriteMacAddressToFlash(const uint8_t *);
void CleanMacAddressInFlash(void);
void WriteModuleInfoToFlash(const uint8_t *myModuleInfo);
void CleanModuleInfoInFlash(void);

/* 本模块信息 */
uint8_t GetMyModuleInfo(uint8_t item);
uint8_t *bsp_getMacAddr(void);
#endif
