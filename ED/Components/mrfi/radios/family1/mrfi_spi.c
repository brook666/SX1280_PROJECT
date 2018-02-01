/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   mrfi_spi.c
// Description: MRFI (Minimal RF Interface)
//              Radios: Si4438, Si4463
//              SPI interface code.
// Author:      Leidi
// Version:     1.0
// Date:        2015-8-03
// History:     2014-8-03   Leidi   初始版本建立.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp_external/mrfi_board_defs.h"
#include "mrfi_spi.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                            Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MRFI_SPI_TURN_CHIP_SELECT_ON()        MRFI_SPI_DRIVE_CSN_LOW()
#define MRFI_SPI_TURN_CHIP_SELECT_OFF()       MRFI_SPI_DRIVE_CSN_HIGH()
#define MRFI_SPI_CHIP_SELECT_IS_OFF()         MRFI_SPI_CSN_IS_HIGH()

#define MRFI_SPI_DEBUG
#ifdef MRFI_SPI_DEBUG
#define MRFI_SPI_ASSERT(x)      BSP_ASSERT(x)
#else
#define MRFI_SPI_ASSERT(x)
#endif


/* ------------------------------------------------------------------------------------------------
 *                                       Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static uint8_t bsp_SpiMasterTransmitByte(uint8_t );
static void mrfi_SpiWriteByte(uint8_t );
static uint8_t mrfi_SpiReadByte(void);
static void mrfi_SpiWriteBytes(uint8_t , uint8_t *);
static void mrfi_SpiReadBytes(uint8_t , uint8_t *);
u8 SPI_NRF_RW(u8 dat);
/**************************************************************************************************
 * @fn          mrfiSpiInit
 *
 * @brief       Initialize SPI.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void mrfiSpiInit(void)
{
  /* configure all SPI related pins */
  MRFI_SPI_CONFIG_CSN_PIN_AS_OUTPUT();
//  MRFI_SPI_CONFIG_SCLK_PIN_AS_OUTPUT();
//  MRFI_SPI_CONFIG_SI_PIN_AS_OUTPUT();
//  MRFI_SPI_CONFIG_SO_PIN_AS_INPUT();

  /* set CSn to default high level */
  MRFI_SPI_DRIVE_CSN_HIGH();
  
  /* initialize the SPI registers */
  MRFI_SPI_INIT(); 
} 

/**************************************************************************************************
 * @fn          bsp_SpiMasterTransmitByte
 *
 * @brief       通过SPI收发1字节.本函数直接与硬件相关.
 *
 * @param       txData - 要发送的字节
 *
 * @return      收到的字节
 **************************************************************************************************
 */
static uint8_t bsp_SpiMasterTransmitByte(uint8_t txData)
{
	uint16_t i=0;
	SPI1->DR =txData;
//  MRFI_SPI_WRITE_BYTE(txData);
  MRFI_SPI_WAIT_DONE()
	{
		i++;
		if(i>10)
		{
			MRFI_Init();  //重启射频模块
			MRFI_RxOn(); 
			break;
		}
	}
	
  return MRFI_SPI_READ_BYTE();
}


/***************************************************************************
 * @fn          mrfi_SpiWriteByte
 *     
 * @brief       写1 byte数据
 *     
 * @data        2015年08月11日
 *     
 * @param       byteToWrite - 输入数据
 *     
 * @return      void
 ***************************************************************************
 */ 
static void mrfi_SpiWriteByte(uint8_t byteToWrite)
{
  bsp_SpiMasterTransmitByte(byteToWrite);
}

/***************************************************************************
 * @fn          mrfi_SpiReadByte
 *     
 * @brief       读1字节数据
 *     
 * @data        2015年08月11日
 *     
 * @param       void
 *     
 * @return      读到的数据
 ***************************************************************************
 */ 
static uint8_t mrfi_SpiReadByte(void)
{
  return bsp_SpiMasterTransmitByte(0xFF);
}

/***************************************************************************
 * @fn          mrfi_SpiWriteBytes
 *     
 * @brief       写多个字节
 *     
 * @data        2015年08月11日
 *     
 * @param       bDataInLength - 长度
 *              pbDataIn      - 指向数据的指针
 *     
 * @return      void
 ***************************************************************************
 */ 
static void mrfi_SpiWriteBytes(uint8_t bDataInLength, uint8_t *pbDataIn)
{
  uint8_t bCnt;
  
  for (bCnt=0; bCnt<bDataInLength; bCnt++)    // Send input data array via SPI
  {
    mrfi_SpiWriteByte(pbDataIn[bCnt]);
  }
}

/*!
 * Read bytes from the radio chip
 *
 * @param bDataOutLength    Number of bytes read from the radio device
 * @param pbDataOut         Pointer to where to put the bytes
 */
/***************************************************************************
 * @fn          mrfi_SpiReadBytes
 *     
 * @brief       读多个字节的数据
 *     
 * @data        2015年08月11日
 *     
 * @param       bDataOutLength - 长度
 *              pbDataOut      - 指向返回数据的指针
 *     
 * @return      void
 ***************************************************************************
 */ 
static void mrfi_SpiReadBytes(uint8_t bDataOutLength, uint8_t *pbDataOut)
{
  uint8_t bCnt;
  
  for (bCnt=0; bCnt<bDataOutLength; bCnt++)
  {
    pbDataOut[bCnt] = mrfi_SpiReadByte(); 
  }
}

/***************************************************************************
 * @fn          Mrfi_SpiSendCommand
 *     
 * @brief       发送命令给射频芯片
 *     
 * @data        2015年08月11日
 *     
 * @param       bCmdLength - 命令长度
 *              pbCmdData  - 指向命令的指针
 *     
 * @return      void
 ***************************************************************************
 */ 
void Mrfi_SpiSendCommand(uint8_t bCmdLength, uint8_t *pbCmdData)
{
  MRFI_SPI_DRIVE_CSN_LOW();                     // Select radio IC by pulling its nSEL pin low
  mrfi_SpiWriteBytes(bCmdLength, pbCmdData);     // Send data array to the radio IC via SPI
  MRFI_SPI_DRIVE_CSN_HIGH();                    // De-select radio IC by putting its nSEL pin high
}

/*!
 * Gets a command response from the radio chip
 *
 * @param bRespLength        Number of bytes to get from the radio chip
 * @param pbRespData         Pointer to where to put the data
 *
 * @return CTS value
 */
/***************************************************************************
 * @fn          Mrfi_SpiGetResponse
 *     
 * @brief       获取芯片响应数据
 *     
 * @data        2015年08月11日
 *     
 * @param       bRespLength - 响应数据长度
 *              pbRespData  - 指向响应数据的指针
 *     
 * @return      CTS值
 ***************************************************************************
 */ 
uint8_t Mrfi_SpiGetResponse(uint8_t bRespLength, uint8_t *pbRespData)
{
  uint8_t bCtsValue = 0;
  uint16_t bErrCnt = 0;
  
  while (bCtsValue!=0xFF)                   // Wait until radio IC is ready with the data
  {
    MRFI_SPI_DRIVE_CSN_LOW();               // Select radio IC by pulling its nSEL pin low
    mrfi_SpiWriteByte(0x44);                // Read command buffer; send command byte(CMD_CTS_READ = 0x44)
    bCtsValue = mrfi_SpiReadByte();         // Read command buffer; get CTS value
    if (bCtsValue != 0xFF)                  // If CTS is not 0xFF, put NSS high and stay in waiting loop
    {
      MRFI_SPI_DRIVE_CSN_HIGH();
      
      if (++bErrCnt > 5)
      {
        return bCtsValue;                   // Error handling; if wrong CTS reads exceeds a limit
      }
    }
  }
  mrfi_SpiReadBytes(bRespLength, pbRespData);  // CTS value ok, get the response data from the radio IC
  MRFI_SPI_DRIVE_CSN_HIGH();                // De-select radio IC by putting its nSEL pin high
  
  return bCtsValue;
}

/*!
 * Waits for CTS to be high
 *
 * @return CTS value
 */
/***************************************************************************
 * @fn          Mrfi_SpiWaitforCTS
 *     
 * @brief       等待CTS
 *     
 * @data        2015年08月11日
 *     
 * @param       void
 *     
 * @return      CTS值
 ***************************************************************************
 */ 
uint8_t Mrfi_SpiWaitforCTS(void)
{
  return Mrfi_SpiGetResponse(0, NULL);
}

/*!
 * Sends a command to the radio chip and gets a response
 *
 * @param bCmdLength      Number of bytes in the command to send to the radio device
 * @param pbCmdData       Pointer to the command data
 * @param bRespLength     Number of bytes in the response to fetch
 * @param pbRespData      Pointer to where to put the response data
 *
 * @return CTS value
 */
/***************************************************************************
 * @fn          Mrfi_SpiSendCmdGetResp
 *     
 * @brief       发送命令并获取响应值
 *     
 * @data        2015年08月11日
 *     
 * @param       bCmdLength  - 命令长度
 *              pbCmdData   - 指向命令的指针
 *              bRespLength - 响应数据长度
 *              pbRespData  - 指向响应数据的指针
 *     
 * @return      CTS值
 ***************************************************************************
 */ 

uint8_t Mrfi_SpiSendCmdGetResp(uint8_t bCmdLength, uint8_t *pbCmdData, uint8_t bRespLength, uint8_t *pbRespData)
{
  uint8_t bCtsValue = 0;
  Mrfi_SpiSendCommand(bCmdLength,pbCmdData);
  bCtsValue = Mrfi_SpiGetResponse(bRespLength,pbRespData);
//#ifdef IWDG_START 
//     IWDG_ReloadCounter();  
//#endif
  return bCtsValue;
}


/***************************************************************************
 * @fn          Mrfi_SpiReadData
 *     
 * @brief       发送一个字节的命令，不等待CTS，字节读数据
 *     
 * @data        2015年08月11日
 *     
 * @param       cmd       - 命令
 *              byteCount - 读到数据的长度
 *              pData     - 指向读出数据的指针
 *     
 * @return      void
 ***************************************************************************
 */ 
void Mrfi_SpiReadData(uint8_t cmd, uint8_t byteCount, uint8_t* pData)
{
  MRFI_SPI_DRIVE_CSN_LOW();
  mrfi_SpiWriteByte(cmd);
	*pData=SPI_NRF_RW(0xFF);
	
//  mrfi_SpiReadBytes(byteCount, pData);
  MRFI_SPI_DRIVE_CSN_HIGH();
}

u8 SPI_NRF_RW(u8 dat)
{  	
   /* 当 SPI发送缓冲器非空时等待 */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  
   /* 通过 SPI2发送一字节数据 */
  SPI_I2S_SendData(SPI1, dat);		
 
   /* 当SPI接收缓冲器为空时等待 */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}



/***************************************************************************
 * @fn          Mrfi_SpiWriteData
 *     
 * @brief       发送1字节的命令，不等待CTS，字节写数据
 *     
 * @data        2015年08月11日
 *     
 * @param       cmd       - 命令
 *              byteCount - 等待写的字节数
 *              pData     - 指向待写的数据的指针
 *     
 * @return      
 ***************************************************************************
 */ 
void Mrfi_SpiWriteData(uint8_t cmd, uint8_t byteCount, uint8_t* pData)
{
  MRFI_SPI_DRIVE_CSN_LOW();
  mrfi_SpiWriteByte(cmd);
  mrfi_SpiWriteBytes(byteCount, pData);
  MRFI_SPI_DRIVE_CSN_HIGH();
}

/**************************************************************************************************
*/


