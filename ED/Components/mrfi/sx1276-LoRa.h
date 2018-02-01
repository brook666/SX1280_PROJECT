/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
#ifndef __SX1276_LORA_H__
#define __SX1276_LORA_H__

#include "stdint.h"
//-----------------------------------------------------------------------------
// ×Ó³ÌÐòÉùÃ÷
//-----------------------------------------------------------------------------
void Sx1276M_GpioInt(void);
void SpiWriteAddressData(unsigned char address, unsigned char data1);
unsigned char SpiReadAddressData(unsigned char u8Addr);
void SX1276LoRaInit( void );

//-----------------------------------------------------------------------------
// define MCU GPIO
//-----------------------------------------------------------------------------
#define RF_RST        PD_ODR_ODR2
#define RF_SCK        PC_ODR_ODR4
#define RF_MISO       PC_IDR_IDR5 //INPUT
#define RF_MOSI       PC_ODR_ODR6 
#define RF_NSEL_PIN   PC_ODR_ODR7
#define SW_CTRL1  		PB_ODR_ODR6
#define SW_CTRL2  		PC_ODR_ODR2



#define RF_RST        	        PD_ODR_ODR2
#define SCK        		PC_ODR_ODR4
#define MISO       		PC_IDR_IDR5 //INPUT
#define MOSI       		PC_ODR_ODR6 
#define nCS   			PC_ODR_ODR7



#define RF_IRQ_DIO0       PD_IDR_IDR3

extern uint8_t gb_SF;
extern uint8_t  gb_BW;
extern uint8_t  CR;	//LR_RegModemConfig1 

extern uint8_t Flg_RxFinish;
void SX1276_Config(uint8_t mode);
uint8_t SPIRead(uint8_t adr);
void SPIBurstRead(uint8_t adr, uint8_t *ptr, uint8_t length);
void SX1276WriteFifo( uint8_t , uint8_t *, uint8_t  );
void SX1276ReadFifo( uint8_t, uint8_t *, uint8_t);
void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size );
void SX1276_LoRaRx_On(uint8_t);
//uint8_t Mrfi_GetRxFifoByteNum(void);
void Mrfi_ReadRxFifo(uint8_t *, uint8_t);
void Mrfi_ReadRxFifoData(uint8_t *pData, uint8_t len,uint8_t);
void SX1276_LoRaEntryRx(uint8_t);
uint8_t SX1276_LoRaRxPacket(void);
void SX1276_LoRaEntryTx(uint8_t *,uint8_t,uint8_t);
void SX1276_LoRaTxPacket(uint8_t *,uint8_t);
void SX1276_LoRaCAD_Init(void);
void SX1276_LoRa_CAD(void);
void SX1276_LoRaClearIrq(void); //Clear irq
void SX1276_Standby(void); //Entry Standby mode  
void delayms(unsigned int t);
void Get_Radio_fre(uint8_t*);
void Get_Radio_Rate(uint8_t *);
void Get_Radio_mode(uint8_t *);
void Get_Radio_Txpower(uint8_t *);
void Set_Radio_Txpower(uint8_t *);
void Sx1278_Reset(void);
/*!
 * SX1276 Internal registers Address
 */
//SX1276 Internal registers Address
#define LR_RegFifo                                  0x0000
// Common settings
#define LR_RegOpMode                                0x0100
#define LR_RegFrMsb                                 0x0600
#define LR_RegFrMid                                 0x0700
#define LR_RegFrLsb                                 0x0800
// Tx settings
#define LR_RegPaConfig                              0x0900
#define LR_RegPaRamp                                0x0A00
#define LR_RegOcp                                   0x0B00
// Rx settings
#define LR_RegLna                                   0x0C00
// LoRa registers
#define LR_RegFifoAddrPtr                           0x0D00
#define LR_RegFifoTxBaseAddr                        0x0E00
#define LR_RegFifoRxBaseAddr                        0x0F00
#define LR_RegFifoRxCurrentaddr                     0x1000
#define LR_RegIrqFlagsMask                          0x1100
#define LR_RegIrqFlags                              0x1200
#define LR_RegRxNbBytes                             0x1300
#define LR_RegRxHeaderCntValueMsb                   0x1400
#define LR_RegRxHeaderCntValueLsb                   0x1500
#define LR_RegRxPacketCntValueMsb                   0x1600
#define LR_RegRxPacketCntValueLsb                   0x1700
#define LR_RegModemStat                             0x1800
#define LR_RegPktSnrValue                           0x1900
#define LR_RegPktRssiValue                          0x1A00
#define LR_RegRssiValue                             0x1B00
#define LR_RegHopChannel                            0x1C00
#define LR_RegModemConfig1                          0x1D00
#define LR_RegModemConfig2                          0x1E00
#define LR_RegSymbTimeoutLsb                        0x1F00
#define LR_RegPreambleMsb                           0x2000
#define LR_RegPreambleLsb                           0x2100
#define LR_RegPayloadLength                         0x2200
#define LR_RegMaxPayloadLength                      0x2300
#define LR_RegHopPeriod                             0x2400
#define LR_RegFifoRxByteAddr                        0x2500
#define LR_RegModemConfig3                         0x2600

// I/O settings
#define REG_LR_DIOMAPPING1                          0x4000
#define REG_LR_DIOMAPPING2                          0x4100
// Version
#define REG_LR_VERSION                              0x4200
// Additional settings
#define REG_LR_PLLHOP                               0x4400
#define REG_LR_TCXO                                 0x4B00
#define REG_LR_PADAC                                0x4D00
#define REG_LR_FORMERTEMP                           0x5B00

#define REG_LR_AGCREF                               0x6100
#define REG_LR_AGCTHRESH1                           0x6200
#define REG_LR_AGCTHRESH2                           0x6300
#define REG_LR_AGCTHRESH3                           0x6400

#endif //__SX1276_LORA_H__
