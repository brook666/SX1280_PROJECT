/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   mrfi_si4438_api.h
// Description: MRFI (Minimal RF Interface)
//              Radios: Si4438
//              射频芯片的API.
// Author:      Leidi
// Version:     1.0
// Date:        2015-8-03
// History:     2014-8-03   Leidi   初始版本建立.
*****************************************************************************/
#ifndef MRFI_SI4438_API_H
#define MRFI_SI4438_API_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mrfi_si4438_defs.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                         Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void Mrfi_ResetSi4438(void);
void Mrfi_ResetRxFifo(void);
void Mrfi_ResetTxFifo(void);
uint8_t Mrfi_GetRxFifoCount(void);
uint8_t Mrfi_GetTxFifoSpace(void);
//void Mrfi_ReadRxFifo(uint8_t *, uint8_t );
void Mrfi_WriteTxFifo(uint8_t * ,uint8_t );
void Mrfi_StartRx(uint8_t );
void Mrfi_StartTx(uint8_t ,uint8_t );
void Mrfi_ChangeToReadyState(void);
void Mrfi_ClearAllInterruptFlags(void);
void Mrfi_ClearAllPacketHandlerFlags(void);
void Mrfi_SetPaPower(uint8_t powerLevel);
uint8_t Mrfi_GetLatchedRSSI(void);
uint8_t Mrfi_GetCurrentRSSI(void);
uint8_t Mrfi_IsRssiExceededThreshold(void);
uint8_t Mrfi_GetPacketHandlerStatus(void);
uint8_t Mrfi_GetCurrentState(void);
void Mrfi_GetPartInfo(uint8_t *);
void Mrfi_SetProperty(uint8_t , uint8_t , uint8_t , uint8_t *);
void Mrfi_GetProperty(uint8_t , uint8_t , uint8_t , uint8_t *);
uint8_t Mrfi_ConfigurationInit(const uint8_t* );

#endif
/**************************************************************************************************
 */
