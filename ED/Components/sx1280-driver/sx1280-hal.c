/*
  ______                              _
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2016 Semtech

Description: Handling of the node configuration protocol

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Gregory Cristian and Matthieu Verdy
*/
#include "sx1280-hal.h"

#define IRQ_HIGH_PRIORITY  0

uint8_t SpiInOut(uint8_t writeByte);
extern void EXIT_PC10_Config(void);


const struct Radio_s Radio =
{
    SX1280Init,
    SX1280HalReset,
    SX1280GetStatus,
    SX1280HalWriteCommand,
    SX1280HalReadCommand,
    SX1280HalWriteRegisters,
    SX1280HalWriteRegister,
    SX1280HalReadRegisters,
    SX1280HalReadRegister,
    SX1280HalWriteBuffer,
    SX1280HalReadBuffer,
    SX1280HalGetDioStatus,
    SX1280GetFirmwareVersion,
    SX1280SetRegulatorMode,
    SX1280SetStandby,
    SX1280SetPacketType,
    SX1280SetModulationParams,
    SX1280SetPacketParams,
    SX1280SetRfFrequency,
    SX1280SetBufferBaseAddresses,
    SX1280SetTxParams,
    SX1280SetDioIrqParams,
    SX1280SetSyncWord,
    SX1280SetRx,
    SX1280GetPayload,
    SX1280SendPayload,
    SX1280SetRangingRole,
    SX1280SetPollingMode,
    SX1280SetInterruptMode,
    SX1280SetRegistersDefault,
    SX1280GetOpMode,
    SX1280SetSleep,
    SX1280SetFs,
    SX1280SetTx,
    SX1280SetRxDutyCycle,
    SX1280SetCad,
    SX1280SetTxContinuousWave,
    SX1280SetTxContinuousPreamble,
    SX1280GetPacketType,
    SX1280SetCadParams,
    SX1280GetRxBufferStatus,
    SX1280GetPacketStatus,
    SX1280GetRssiInst,
    SX1280GetIrqStatus,
    SX1280ClearIrqStatus,
    SX1280Calibrate,
    SX1280SetSaveContext,
    SX1280SetAutoTx,
    SX1280SetAutoFS,
    SX1280SetLongPreamble,
    SX1280SetPayload,
    SX1280SetSyncWordErrorTolerance,
    SX1280SetCrcSeed,
    SX1280SetCrcPolynomial,
    SX1280SetWhiteningSeed,
    SX1280SetRangingIdLength,
    SX1280SetDeviceRangingAddress,
    SX1280SetRangingRequestAddress,
    SX1280GetRangingResult,
    SX1280SetRangingCalibration,
    SX1280RangingClearFilterResult,
    SX1280RangingSetFilterNumSamples,
    SX1280GetFrequencyError,
};

/*!
 * Radio driver structure initialization
 */


/*!
 * \brief Used to block execution waiting for low state on radio busy pin.
 *        Essentially used in SPI communications
 */
void SX1280HalWaitOnBusy( void )
{
    while( GPIO_ReadInputDataBit( RADIO_BUSY_PORT, RADIO_BUSY_PIN ) == 1 );
}

void SX1280HalInit( DioIrqHandler **irqHandlers )
{
    SX1280HalReset( );
//    SX1280HalIoIrqInit( irqHandlers );
}

void SX1280HalIoIrqInit( DioIrqHandler **irqHandlers )
{
   // GpioSetIrq( RADIO_DIOx_PORT, RADIO_DIOx_PIN, IRQ_HIGH_PRIORITY, irqHandlers[0] );
	  EXIT_PC10_Config();
}

void SX1280HalReset( void )
{
    delay_ms( 20 );
    GPIO_ResetBits( RADIO_nRESET_PORT, RADIO_nRESET_PIN);
    delay_ms( 50 );
    GPIO_SetBits( RADIO_nRESET_PORT, RADIO_nRESET_PIN);
    delay_ms( 20 );
}

void SX1280HalClearInstructionRam( void )
{
    // Clearing the instruction RAM is writing 0x00s on every bytes of the
    // instruction RAM
	  uint16_t address;
    SX1280HalWaitOnBusy( );

    GPIO_ResetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);
    SpiInOut( RADIO_WRITE_REGISTER );                 // Send write register opcode
    SpiInOut( ( IRAM_START_ADDRESS >> 8 ) & 0x00FF ); // Send MSB of the first byte address
    SpiInOut( IRAM_START_ADDRESS & 0x00FF );          // Send LSB of the first byte address

    for(address= IRAM_START_ADDRESS; address < ( IRAM_START_ADDRESS + IRAM_SIZE ); address++ )
    {
        SpiInOut( 0x00 );
    }
    GPIO_SetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    SX1280HalWaitOnBusy( );
}

void SX120HalWakeup( void )
{
    __disable_irq( );

    GPIO_ResetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    SpiInOut( RADIO_GET_STATUS );
    SpiInOut( 0x00 );

    GPIO_SetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    // Wait for chip to be ready.
    SX1280HalWaitOnBusy( );

    __enable_irq( );
}

void SX1280HalWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
	  uint16_t i;
    SX1280HalWaitOnBusy( );

    GPIO_ResetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);
    
    SpiInOut( ( uint8_t )command );
    
    for(  i= 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }

    GPIO_SetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    if( command != RADIO_SET_SLEEP )
    {
        SX1280HalWaitOnBusy( );
    }
}

void SX1280HalReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
	  uint16_t i; 
    SX1280HalWaitOnBusy( );

    GPIO_ResetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);
    
    SpiInOut( ( uint8_t )command );
    SpiInOut( 0x00 );
    for( i= 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }

    GPIO_SetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    SX1280HalWaitOnBusy( );
}

void SX1280HalWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
	  uint16_t i;
    SX1280HalWaitOnBusy( );

    GPIO_ResetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);
    
    SpiInOut( RADIO_WRITE_REGISTER );
    SpiInOut( ( address & 0xFF00 ) >> 8 );
    SpiInOut( address & 0x00FF );
    
    for(i= 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }

    GPIO_SetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    SX1280HalWaitOnBusy( );
}

void SX1280HalWriteRegister( uint16_t address, uint8_t value )
{
    SX1280HalWriteRegisters( address, &value, 1 );
}

void SX1280HalReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
	  uint16_t i;
    SX1280HalWaitOnBusy( );

    GPIO_ResetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    SpiInOut( RADIO_READ_REGISTER );
    SpiInOut( ( address & 0xFF00 ) >> 8 );
    SpiInOut( address & 0x00FF );
    SpiInOut( 0 );
    for( i= 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }
    GPIO_SetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    SX1280HalWaitOnBusy( );
}

uint8_t SX1280HalReadRegister( uint16_t address )
{
    uint8_t data;

    SX1280HalReadRegisters( address, &data, 1 );

    return data;
}

void SX1280HalWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
	  uint16_t  i;
    SX1280HalWaitOnBusy( );

    GPIO_ResetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);
    
    SpiInOut( RADIO_WRITE_BUFFER );
    SpiInOut( offset );
    for( i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }
    GPIO_SetBits( RADIO_NSS_PORT, RADIO_NSS_PIN );

    SX1280HalWaitOnBusy( );
}

void SX1280HalReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
	  uint16_t i;
    SX1280HalWaitOnBusy( );

    GPIO_ResetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    SpiInOut( RADIO_READ_BUFFER );
    SpiInOut( offset );
    SpiInOut( 0 );
    for( i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }
    GPIO_SetBits( RADIO_NSS_PORT, RADIO_NSS_PIN);

    SX1280HalWaitOnBusy( );
}

uint8_t SX1280HalGetDioStatus( void )
{
    return ( GPIO_ReadInputDataBit( RADIO_DIOx_PORT, RADIO_DIOx_PIN ) << 1 ) | ( GPIO_ReadInputDataBit( RADIO_BUSY_PORT, RADIO_BUSY_PIN ) << 0 );
}


uint8_t SpiInOut(uint8_t writeByte)
{
  uint8_t return_data;
   /* 通过 SPI1发送一字节数据 */
  SPI_I2S_SendData(SPI1, writeByte);		
 
   /* 当SPI接收缓冲器为空时等待 */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return_data=SPI_I2S_ReceiveData(SPI1);
	return return_data;
}

