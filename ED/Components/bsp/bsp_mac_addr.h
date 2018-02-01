/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   addr_control_fun.h
// Description: User APP.
//              �豸��ַ����ӿ�.
// Author:      Leidi
// Version:     3.0
// Date:        2012-9-30
// History:     2014-9-30   Leidi ��AP��ַ��д,IP��ַ��д,����MAC��ַ��д�ĺ���
//                                ԭ����net_list_proc.h���뱾�ļ�. �ڵ�˲���ʹ
//                                ��net_list_proc.h�ļ�.
//              2015-06-16  Leidi ʹ��ö�ٶ���ģ����Ϣ.
//                                ������ȡ��ģ����Ϣ�ĺ���ԭ��GetMyModuleInfo().
//              2015-07-29  Leidi �޸ĺ�����,����Load_IpAddress(),Get_IpAddress()��Load_MacAddress().
//              2015-09-06  Leidi ɾ���йش���AP��ַ��IP��ַ�Ĵ���.
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
/* ����MAC��ַ��д */
void Load_MacAddress(void);
uint8_t MacAddressInFlashIsValid(void);
uint8_t UseMacAddressInFlash(void);
void WriteMacAddressToFlash(const uint8_t *);
void CleanMacAddressInFlash(void);
void WriteModuleInfoToFlash(const uint8_t *myModuleInfo);
void CleanModuleInfoInFlash(void);

/* ��ģ����Ϣ */
uint8_t GetMyModuleInfo(uint8_t item);
uint8_t *bsp_getMacAddr(void);
#endif
