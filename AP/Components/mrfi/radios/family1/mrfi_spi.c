/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   mrfi_spi.c
// Description: MRFI (Minimal RF Interface)
//              Radios: Si4438, Si4463
//              SPI interface code.
// Author:      Leidi
// Version:     1.0
// Date:        2015-8-03
// History:     2014-8-03   Leidi   ��ʼ�汾����.
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
 * @brief       ͨ��SPI�շ�1�ֽ�.������ֱ����Ӳ�����.
 *
 * @param       txData - Ҫ���͵��ֽ�
 *
 * @return      �յ����ֽ�
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
			MRFI_Init();  //������Ƶģ��
			MRFI_RxOn(); 
			break;
		}
	}
	
  return MRFI_SPI_READ_BYTE();
}


/***************************************************************************
 * @fn          mrfi_SpiWriteByte
 *     
 * @brief       д1 byte����
 *     
 * @data        2015��08��11��
 *     
 * @param       byteToWrite - ��������
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
 * @brief       ��1�ֽ�����
 *     
 * @data        2015��08��11��
 *     
 * @param       void
 *     
 * @return      ����������
 ***************************************************************************
 */ 
static uint8_t mrfi_SpiReadByte(void)
{
  return bsp_SpiMasterTransmitByte(0xFF);
}

/***************************************************************************
 * @fn          mrfi_SpiWriteBytes
 *     
 * @brief       д����ֽ�
 *     
 * @data        2015��08��11��
 *     
 * @param       bDataInLength - ����
 *              pbDataIn      - ָ�����ݵ�ָ��
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
 * @brief       ������ֽڵ�����
 *     
 * @data        2015��08��11��
 *     
 * @param       bDataOutLength - ����
 *              pbDataOut      - ָ�򷵻����ݵ�ָ��
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
 * @brief       �����������ƵоƬ
 *     
 * @data        2015��08��11��
 *     
 * @param       bCmdLength - �����
 *              pbCmdData  - ָ�������ָ��
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
 * @brief       ��ȡоƬ��Ӧ����
 *     
 * @data        2015��08��11��
 *     
 * @param       bRespLength - ��Ӧ���ݳ���
 *              pbRespData  - ָ����Ӧ���ݵ�ָ��
 *     
 * @return      CTSֵ
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
 * @brief       �ȴ�CTS
 *     
 * @data        2015��08��11��
 *     
 * @param       void
 *     
 * @return      CTSֵ
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
 * @brief       ���������ȡ��Ӧֵ
 *     
 * @data        2015��08��11��
 *     
 * @param       bCmdLength  - �����
 *              pbCmdData   - ָ�������ָ��
 *              bRespLength - ��Ӧ���ݳ���
 *              pbRespData  - ָ����Ӧ���ݵ�ָ��
 *     
 * @return      CTSֵ
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
 * @brief       ����һ���ֽڵ�������ȴ�CTS���ֽڶ�����
 *     
 * @data        2015��08��11��
 *     
 * @param       cmd       - ����
 *              byteCount - �������ݵĳ���
 *              pData     - ָ��������ݵ�ָ��
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
   /* �� SPI���ͻ������ǿ�ʱ�ȴ� */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  
   /* ͨ�� SPI2����һ�ֽ����� */
  SPI_I2S_SendData(SPI1, dat);		
 
   /* ��SPI���ջ�����Ϊ��ʱ�ȴ� */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}



/***************************************************************************
 * @fn          Mrfi_SpiWriteData
 *     
 * @brief       ����1�ֽڵ�������ȴ�CTS���ֽ�д����
 *     
 * @data        2015��08��11��
 *     
 * @param       cmd       - ����
 *              byteCount - �ȴ�д���ֽ���
 *              pData     - ָ���д�����ݵ�ָ��
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


