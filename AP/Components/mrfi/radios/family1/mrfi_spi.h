/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   mrfi_spi.h
// Description: MRFI (Minimal RF Interface)
//              SPI interface code for Radio FamRadios: Si4438, Si4463
//              SPI interface code.
// Author:      Leidi
// Version:     1.0
// Date:        2015-8-03
// History:     2014-8-03   Leidi   初始版本建立.
*****************************************************************************/

#ifndef MRFI_SPI_H
#define MRFI_SPI_H

/* ------------------------------------------------------------------------------------------------
 *                                         Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                         Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void mrfiSpiInit(void);
void Mrfi_SpiSendCommand(uint8_t , uint8_t *);
uint8_t Mrfi_SpiGetResponse(uint8_t , uint8_t *);
uint8_t Mrfi_SpiWaitforCTS(void);
uint8_t Mrfi_SpiSendCmdGetResp(uint8_t , uint8_t *, uint8_t , uint8_t *);
void Mrfi_SpiReadData(uint8_t , uint8_t , uint8_t* );
void Mrfi_SpiWriteData(uint8_t , uint8_t , uint8_t* );

/**************************************************************************************************
 */
#endif
