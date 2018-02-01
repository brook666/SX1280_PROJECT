/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   mrfi_si4438_api.c
// Description: MRFI (Minimal RF Interface)
//              Radios: Si4438
//              射频芯片的API.
// Author:      Leidi
// Version:     1.0
// Date:        2015-8-03
// History:     2014-8-03   Leidi   初始版本建立.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mrfi_si4438_api.h"
#include "mrfi_spi.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
 /***************************************************************************
 * @fn          Mrfi_ResetSi4438
 *     
 * @brief       复位SI4438
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void Mrfi_ResetSi4438(void)
{
  uint16_t wDelay;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  // Put radio in shutdown, wait then release 
  GPIO_SetBits(GPIOB, GPIO_Pin_0);
  //! @todo this needs to be a better delay function.
  for (wDelay = 0; wDelay < 330; wDelay++);
  GPIO_ResetBits(GPIOB, GPIO_Pin_0);
  for (wDelay = 0; wDelay < RADIO_CONFIGURATION_DATA_RADIO_DELAY_CNT_AFTER_RESET; wDelay++);
  delay_ms(6);
}

/**************************************************************************************************
 * @fn          Mrfi_ResetRxFifo
 *
 * @brief       Flush the receive FIFO of any residual data.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_ResetRxFifo(void)
{
  uint8_t Pro2Cmd[2] = {CMD_FIFO_INFO, 0x02}; // Use FIFO INFO command to Reset Rx FIFO

  Mrfi_SpiSendCmdGetResp(2, Pro2Cmd, 0, NULL);
}

/**************************************************************************************************
 * @fn          Mrfi_ResetTxFifo
 *
 * @brief       Flush the transmit FIFO of any residual data.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_ResetTxFifo(void)
{
  uint8_t Pro2Cmd[2] = {CMD_FIFO_INFO, 0x01}; // Use FIFO INFO command to Reset Tx FIFO
  
  Mrfi_SpiSendCmdGetResp(2, Pro2Cmd, 0, NULL);
}

/**************************************************************************************************
 * @fn          Mrfi_GetRxFifoCount
 *
 * @brief       Returns the number of received bytes currently stored in the RX FIFO. 
 *
 * @param       none
 *
 * @return      RX_FIFO_COUNT
 **************************************************************************************************
 */
uint8_t Mrfi_GetRxFifoCount(void)
{
  uint8_t Pro2Cmd[1] = {CMD_FIFO_INFO};
  uint8_t fifoInfo[2];

  Mrfi_SpiSendCmdGetResp(1, Pro2Cmd, 2, fifoInfo);
  
  return fifoInfo[0]; //RX_FIFO_COUNT
}

/**************************************************************************************************
 * @fn          Mrfi_GetTxFifoSpace
 *
 * @brief       Returns the number of empty bytes (i.e., space) currently available in the TX FIFO. 
 *
 * @param       none
 *
 * @return      TX_FIFO_SPACE
 **************************************************************************************************
 */
uint8_t Mrfi_GetTxFifoSpace(void)
{
  uint8_t Pro2Cmd[1] = {CMD_FIFO_INFO};
  uint8_t fifoInfo[2];

  Mrfi_SpiSendCmdGetResp(1, Pro2Cmd, 2, fifoInfo);
  
  return fifoInfo[1]; //TX_FIFO_SPACE
}

/**************************************************************************************************
 * @fn          Mrfi_ReadRxFifo
 *
 * @brief       Read data from radio receive FIFO.
 *
 * @param       pData - pointer for storing read data
 * @param       len   - length of data in bytes
 *
 * @return      none
 **************************************************************************************************
 */
//void Mrfi_ReadRxFifo(uint8_t *pData, uint8_t len)
//{
//  Mrfi_SpiReadData(CMD_RX_FIFO_READ, len, pData);
//}

/**************************************************************************************************
 * @fn          Mrfi_WriteTxFifo
 *
 * @brief       Write data to radio transmit FIFO.
 *
 * @param       pData - pointer for storing write data
 * @param       len   - length of data in bytes
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_WriteTxFifo(uint8_t *pData ,uint8_t len)
{
  Mrfi_SpiWriteData(CMD_TX_FIFO_WRITE, len, pData);
}

/**************************************************************************************************
 * @fn          Mrfi_StartRx
 *
 * @brief       Sends START_RX command to the radio.
 *
 * @param       channel - Channel number.
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_StartRx(uint8_t channel)
{
  uint8_t Pro2Cmd[8];

  Pro2Cmd[0] = CMD_START_RX;  // Use start Rx command
  Pro2Cmd[1] = channel;       // Set channel number
  Pro2Cmd[2] = 0x00;          // Start RX condition: Start Rx immediately
  Pro2Cmd[3] = 0x00;          // Packet fields used, do not enter packet length here
  Pro2Cmd[4] = 0x00;          // Packet fields used, do not enter packet length here
  Pro2Cmd[5] = 0x08;          // No change if Rx timeout
  Pro2Cmd[6] = 0x03;          // Ready state after Rx valid state
  Pro2Cmd[7] = 0x08;          // Ready state after Rx invalid state
  
  Mrfi_SpiSendCmdGetResp(8, Pro2Cmd, 0, NULL);
}

/**************************************************************************************************
 * @fn          Mrfi_StartTx
 *
 * @brief       Sends START_TX command to the radio.
 *
 * @param       channel - Channel number.
 * @param       len     - Payload length (exclude the PH generated CRC).
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_StartTx(uint8_t channel, uint8_t len)
{
  uint8_t Pro2Cmd[6];

  Pro2Cmd[0] = CMD_START_TX;  // Use Tx Start command
  Pro2Cmd[1] = channel;       // Set channel number
  Pro2Cmd[2] = 0x30;          // Start TX condition: Ready state after Tx, start Tx immediately
  Pro2Cmd[3] = 0x00;          // Packet fields used, do not enter packet length here
  Pro2Cmd[4] = len;          // Packet fields used, do not enter packet length here
  Pro2Cmd[5] = 0x00;          // TX delay
  
  Mrfi_SpiSendCmdGetResp(6, Pro2Cmd, 0, NULL);
}

/**************************************************************************************************
 * @fn          Mrfi_ChangeToReadyState
 *
 * @brief       Issue a change state command to the radio. Switch the chip to READY state. 
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_ChangeToReadyState(void)
{
  uint8_t Pro2Cmd[2] = {CMD_CHANGE_STATE, 3}; // Use change state command to Go to READY state
  
  Mrfi_SpiSendCmdGetResp(2, Pro2Cmd, 0, NULL);
}

/**************************************************************************************************
 * @fn          Mrfi_ClearAllInterruptFlags
 *
 * @brief       Clear all Interrupt status/pending flags. Does NOT read back interrupt flags. 
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_ClearAllInterruptFlags(void)
{
  // Sending the GET_INT_STATUS command with 
  // NO input parameters results in clearing
  // all of the PENDING interrupts.
  uint8_t Pro2Cmd[4];
  
  Pro2Cmd[0] = CMD_GET_INT_STATUS;
  Pro2Cmd[1] = 0;
  Pro2Cmd[2] = 0;
  Pro2Cmd[3] = 0;   
  
  Mrfi_SpiSendCmdGetResp(4, Pro2Cmd, 0, NULL);
}

/**************************************************************************************************
 * @fn          Mrfi_ClearAllPacketHandlerFlags
 *
 * @brief       Clear all Packet Handler status flags. Does NOT read back interrupt flags
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_ClearAllPacketHandlerFlags(void)
{
  // Sending the GET_PH_STATUS command with 
  // NO input parameters results in clearing
  // all of the PENDING interrupts.
  uint8_t Pro2Cmd[2] ;
  
  Pro2Cmd[0] = CMD_GET_PH_STATUS;
  Pro2Cmd[1] = 0;  
  
  Mrfi_SpiSendCmdGetResp(2, Pro2Cmd, 0, NULL);
}

/**************************************************************************************************
 * @fn          Mrfi_SetPaPower
 *
 * @brief       Configuration of PA output power level. 
 *
 * @param       powerLevel - PA output power level.
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_SetPaPower(uint8_t powerLevel)
{
  uint8_t Pro2Cmd[4];

  Pro2Cmd[0] = 0x08;        // Si4463/64: higher maximum power but with larger step size. 
  Pro2Cmd[1] = powerLevel;  // PA output power level
  Pro2Cmd[2] = 0x00;        // PA Bias and duty cycle of the TX clock source
  Pro2Cmd[3] = 0x3d;        // PA ramping parameters. 
  
  Mrfi_SetProperty(PROP_PA_GROUP, 4, PROP_PA_MODE, Pro2Cmd);
}

/**************************************************************************************************
 * @fn          Mrfi_GetLatchedRSSI
 *
 * @brief       Get latched RSSI value as defined in MODEM_RSSI_CONTROL:LATCH. 
 *              注意:要使用此功能,必须将FRR_CTL_A_MODE设置为LATCHED_RSSI(10). 
 *
 * @param       none
 *
 * @return      LATCH_RSSI
 **************************************************************************************************
 */
uint8_t Mrfi_GetLatchedRSSI(void)
{
  uint8_t regValue;
  
  Mrfi_SpiReadData(CMD_FAST_RESPONSE_REG_A, 1, &regValue);
  
  return regValue;
}

/**************************************************************************************************
 * @fn          Mrfi_GetCurrentRSSI
 *
 * @brief       Get Current RSSI value reading from the modem. 
 *
 * @param       none
 *
 * @return      CURR_RSSI
 **************************************************************************************************
 */
uint8_t Mrfi_GetCurrentRSSI(void)
{
  //uint8_t SendCmd[2] = {CMD_GET_MODEM_STATUS, 0x00};
  uint8_t ReplyCmd[1];
  ReplyCmd[0]=RSSI_LF_MODE-SPIRead((u8)(LR_RegRssiValue>>8));
  //Mrfi_SpiSendCmdGetResp(2, SendCmd, 3, ReplyCmd);
  
  return ReplyCmd[0]; //CURR_RSSI
}

/**************************************************************************************************
 * @fn          Mrfi_IsRssiExceededThreshold
 *
 * @brief       Check if the current RSSI value has exceeded the threshold value defined in the 
 *              MODEM_RSSI_THRESH property.
 *
 * @param       none
 *
 * @return      0x00 - RSSI value below the threshold
 *              0x01 - RSSI value has exceeded the threshold
 **************************************************************************************************
 */
uint8_t Mrfi_IsRssiExceededThreshold(void)
{
  uint8_t SendCmd[2] = {CMD_GET_MODEM_STATUS, 0x00};
  uint8_t ReplyCmd[2];
  
  Mrfi_SpiSendCmdGetResp(2, SendCmd, 2, ReplyCmd);
  
  return !!(ReplyCmd[1] & 0x08);  //MODEM_STATUS: RSSI
}

/**************************************************************************************************
 * @fn          Mrfi_GetPacketHandlerStatus
 *
 * @brief       Get the interrupt status of the Packet Handler Interrupt. 
*               注意:要使用此功能,必须将FRR_CTL_B_MODE设置为Packet Handler status(0x03). 
 *
 * @param       none
 *
 * @return      PH_STATUS
 **************************************************************************************************
 */
uint8_t Mrfi_GetPacketHandlerStatus(void)
{
  uint8_t regValue;
  
  Mrfi_SpiReadData(CMD_FAST_RESPONSE_REG_B, 1, &regValue);
  
  return regValue;
}

/**************************************************************************************************
 * @fn          Mrfi_GetCurrentState
 *
 * @brief       获取当前射频的工作状态. 
 *               注意:要使用此功能,必须将FRR_CTL_C_MODE设置为CURRENT_STATE(0x09). 
 *
 * @param       none
 *
 * @return      PH_STATUS
 **************************************************************************************************
 */
//uint8_t Mrfi_GetCurrentState(void)
//{
//  uint8_t regValue;
//  
//  regValue=(SPIRead((u8)(0x0100>>8)))&(0x07);
//  
//  return regValue;
//}
/**************************************************************************************************
 * @fn          Mrfi_GetPartInfo
 *
 * @brief       Reports basic information about the device. 
 *
 * @param       Part Number, Part Version, ROM ID, etc. 
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_GetPartInfo(uint8_t *pPartInfo)
{
  uint8_t Pro2Cmd[1];
  
  Pro2Cmd[0] = CMD_PART_INFO;

  Mrfi_SpiSendCmdGetResp(1, Pro2Cmd, 8, pPartInfo);
}

/**************************************************************************************************
 * @fn          Mrfi_SetProperty
 *
 * @brief       Send SET_PROPERTY command to the radio.
 *
 * @param       group       Property group.
 * @param       numPorps    Number of property to be set. The properties must be in ascending order
 *                          in their sub-property aspect. Max. 12 properties can be set in one command.
 * @param       startProp   Start sub-property address.
 * @param       pPropsWrite Pointed to the value to be seted.
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_SetProperty(uint8_t group, uint8_t numPorps, uint8_t startProp, uint8_t *pPropsWrite)
{
  uint8_t Pro2Cmd[12];
  uint8_t cmdIndex;
  
  Pro2Cmd[0] = CMD_SET_PROPERTY;
  Pro2Cmd[1] = group;
  Pro2Cmd[2] = numPorps;
  Pro2Cmd[3] = startProp;
  
  cmdIndex = 4;
  while(numPorps--)
  {
    Pro2Cmd[cmdIndex] = pPropsWrite[cmdIndex - 4];
    cmdIndex++;
  }
  
  Mrfi_SpiSendCommand(cmdIndex, Pro2Cmd); 
}

/**************************************************************************************************
 * @fn          Mrfi_GetProperty
 *
 * @brief       Get property values from the radio.
 *
 * @param       group       Property group number.
 * @param       numPorps    Number of properties to be read.
 * @param       startProp   Starting sub-property number.
 * @param       pPropsRead  Pointed to the array to read to.
 *
 * @return      none
 **************************************************************************************************
 */
void Mrfi_GetProperty(uint8_t group, uint8_t numPorps, uint8_t startProp, uint8_t *pPropsRead)
{
  uint8_t Pro2Cmd[4];
  
  Pro2Cmd[0] = CMD_GET_PROPERTY;
  Pro2Cmd[1] = group;
  Pro2Cmd[2] = numPorps;
  Pro2Cmd[3] = startProp;
  
  Mrfi_SpiSendCmdGetResp(4, Pro2Cmd, numPorps, pPropsRead);
}

/**************************************************************************************************
 * @fn          Mrfi_ConfigurationInit
 *
 * @brief       This function is used to load all properties and commands with a list of NULL 
 *              terminated commands. Before this function @si446x_reset should be called.
 *
 * @param       pSetPropCmd - Pointed to commands structure in the array
 *
 * @return      SI446X_SUCCESS        - Success
 *              SI446X_CTS_TIMEOUT    - Timeout occured
 *              SI446X_COMMAND_ERROR  - Number of command bytes exceeds maximal allowable length
 **************************************************************************************************
 */
uint8_t Mrfi_ConfigurationInit(const uint8_t* pSetPropCmd)
{
  uint8_t col, numOfBytes;
  uint8_t Pro2Cmd[16];
  
  /* While cycle as far as the pointer points to a command */
  while (*pSetPropCmd != 0x00)
  {
    /* Commands structure in the array:
     * --------------------------------
     * LEN | <LEN length of data>
     */

    numOfBytes = *pSetPropCmd++;

    if (numOfBytes > 16u)
    {
      /* Number of command bytes exceeds maximal allowable length */
      return SI446X_COMMAND_ERROR;
    }

    for (col = 0u; col < numOfBytes; col++)
    {
      Pro2Cmd[col] = *pSetPropCmd;
      pSetPropCmd++;
    }

    if (Mrfi_SpiSendCmdGetResp(numOfBytes, Pro2Cmd, 0, 0) != 0xFF)
    {
      /* Timeout occured */
      return SI446X_CTS_TIMEOUT;
    }
  }

  return SI446X_SUCCESS;
}

/**************************************************************************************************
 */


