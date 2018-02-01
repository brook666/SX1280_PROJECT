/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   stm32_flash.h
// Description: BSP.
//              STM32 FLASH��д��װ.
// Author:      Leidi
// Version:     1.0
// Date:        2014-11-22
// History:     2014-11-22  Leidi ��ʼ�汾����.
//              2014-12-02  Leidi ��������ԭ��FLASH_EraseOnePage().
*****************************************************************************/

#ifndef __STM32_FLASH_H
#define __STM32_FLASH_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "stm32f10x.h"


typedef struct 
{
	uint16_t STATE;//0xFFFF�������� 0x0001���Ը���Ӧ�ó��� 0x0002��ʾ�ָ���������
	uint32_t LENGTH;
}BootLoaderState;


/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void FLASH_WriteByte(uint32_t addr , uint8_t *pByte , uint16_t byte_num);
void FLASH_ReadByte(uint32_t addr, uint8_t *pByte, uint16_t byte_num);
void FLASH_EraseOnePage(uint32_t addr);
void Read_BootLoader_State(void);
void Set_BootLoader_State(void);


#endif
/**************************************************************************************************
 */
