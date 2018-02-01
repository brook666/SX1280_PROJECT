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

#include "stdint.h"
#include "sx1276-LoRa.h"
#include "stm32_rtc.h"
#include "mrfi.h"
#include "string.h"
#include "mrfi_spi.h"
#include "bsp_external/mrfi_board_defs.h"
#include "stm32f10x_spi.h"
extern mrfiPacket_t mrfiIncomingPacket;

#define LENGTH_FIELD_LEN     1

uint8_t gb_SF;
uint8_t  gb_BW;
uint8_t  CR;	//LR_RegModemConfig1 
  
#define CRC2   0x00  //CRC DISENable

#define datlen 13
//static uint8_t data_len;
extern uint8_t array[11];
uint8_t write1byte_to_sx1278(uint8_t);
uint8_t ReadOneByte(uint8_t);
uint8_t gtmp;
/**********************************************************
**Parameter table define
**********************************************************/
const u16 SX1276FreqTbl[5][3] = {{0x066C, 0x0780, 0x0800},{0x066C, 0x07B0, 0x0806},{0x066C, 0x07D0, 0x080B},{0x066C, 0x07F0, 0x0810},{0x066D, 0x0710, 0x0814}}; //434MHz
//const uint16_t SX1276FreqTbl[3] = {0x0685, 0x073b, 0x0813}; //434MHz @ 26m
//__root const u16 SX1276FreqTbl[3] = {0x0634, 0x0700, 0x0800}; //169MHz @ 26m

const uint16_t SX1276PowerTbl[6] =
{ 
  0x09FF,                   //20dbm  
  0x09FC,                   //17dbm
  0x09F9,                   //14dbm
  0x09F6,                   //11dbm 
	0x09F4,                   //9dbm
	0x09F1,                   //6dbm
};


const uint8_t SX1276LoRaBwTbl[10] =
{// 0        1     2       3      4       5         6      7       8      9
//7.8KHz,10.4KHz,15.6KHz,20.8KHz,31.2KHz,41.7KHz,62.5KHz,125KHz,250KHz,500KHz
  0,1,2,3,4,5,6,7,8,9
};

const uint8_t SX1276SpreadFactorTbl[7] =
{
  6,7,8,9,10,11,12
};


//__root const u8  SX1276Data[datlen] = {"1234567890ABCDEFGHIJK"};
uint8_t  SX1276Data[256];

uint8_t  gb_RxData[254]; //Receive data buffer

void delayms(unsigned int t)
{
	unsigned int i;
	unsigned char j; 
	for(i=0;i<t;i++)
	for(j=0;j<120;j++);
}


/**********************************************************
**Name:     SPICmd8bit
**Function: SPI Write one byte
**Input:    WrPara
**Output:   none
**note:     use for burst mode
**********************************************************/
void SPICmd8bit(u8 WrPara)
{
  u8 bitcnt;  
  GPIO_ResetBits(GPIOB,GPIO_Pin_9);                     //nCS=0;
  GPIO_ResetBits(GPIOB,GPIO_Pin_11);                    //SCK=0;
  
  for(bitcnt=8; bitcnt!=0; bitcnt--)
  {
    GPIO_ResetBits(GPIOB,GPIO_Pin_11);                  //SCK=0;
    if(WrPara&0x80)
     GPIO_SetBits(GPIOB,GPIO_Pin_12);                   //MOSI=1;
    else
     GPIO_ResetBits(GPIOB,GPIO_Pin_12);                 //MOSI=0;
	__ASM("nop");	
	__ASM("nop");	
     GPIO_SetBits(GPIOB,GPIO_Pin_11);                    //SCK=1;
	__ASM("nop");	
    WrPara <<= 1;
  }
  GPIO_ResetBits(GPIOB,GPIO_Pin_11);                    //SCK=0;
  GPIO_SetBits(GPIOB,GPIO_Pin_12);                      //MOSI=1;
}

/**********************************************************
**Name:     SPIRead8bit
**Function: SPI Read one byte
**Input:    None
**Output:   result byte
**Note:     use for burst mode
**********************************************************/

void Sx1278_Reset(void)
{
	GPIO_SetBits(RESET_IO_GROUP,RESET_IO_PORT);         //SX1278  RFSET=1
}

u8 SPIRead8bit(void)
{
 u8 RdPara = 0;
 u8 bitcnt;
 
  GPIO_ResetBits(GPIOB,GPIO_Pin_9);                     //nCS=0;
  GPIO_SetBits(GPIOB,GPIO_Pin_12);                      //MOSI=1;    Read one byte data from FIFO, MOSI hold to High
  for(bitcnt=8; bitcnt!=0; bitcnt--)
  {
  GPIO_ResetBits(GPIOB,GPIO_Pin_11);    //SCK=0;
	__ASM("nop");
	__ASM("nop");	
    RdPara <<= 1;
    GPIO_SetBits(GPIOB,GPIO_Pin_11);    //SCK=1;
    if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8))
      RdPara |= 0x01;
    else
      RdPara |= 0x00;
  }
  GPIO_ResetBits(GPIOB,GPIO_Pin_11);    //SCK=0;
  return(RdPara);
}

/**********************************************************
**Name:     SPIRead
**Function: SPI Read CMD
**Input:    adr -> address for read
**Output:   None
**********************************************************/
//u8 SPIRead(u8 adr)
//{
//  u8 tmp; 
//  SPICmd8bit(adr);                                         //Send address first
//  tmp = SPIRead8bit();  
//  GPIO_SetBits(GPIOB,GPIO_Pin_9);                          //nCS=1;
//  return(tmp);
//}

u8 SPIRead(u8 adr)
{
  uint8_t regValue;
  
  regValue=ReadOneByte(adr);
  
  return regValue;
}

uint8_t ReadOneByte(uint8_t Readaddr)
{
	uint8_t ReadData;
  MRFI_SPI_DRIVE_CSN_LOW();
  ReadData=write1byte_to_sx1278(Readaddr&0x7F);
	ReadData=write1byte_to_sx1278(0);
	MRFI_SPI_DRIVE_CSN_HIGH(); 
	return ReadData;
}



void SPIWrite(uint16_t WrPara)
{
	uint8_t i=2;
  uint8_t write_data[2];
	write_data[0]=(WrPara>>8);
	write_data[0] |=0x80;
	write_data[1]=WrPara;
  MRFI_SPI_DRIVE_CSN_LOW();
	for(i=0;i<2;i++)
	{
    write1byte_to_sx1278(write_data[i]);
	}
	MRFI_SPI_DRIVE_CSN_HIGH(); 
} 

uint8_t write1byte_to_sx1278(uint8_t writeByte)
{
//  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  
   /* 通过 SPI2发送一字节数据 */
  SPI_I2S_SendData(SPI1, writeByte);		
 
   /* 当SPI接收缓冲器为空时等待 */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}


void SPIBurstRead(u8 adr, u8 *ptr, u8 length)
{
  if(length<1)                                            //length must more than one
    return;
  else
  {
    SX1276ReadFifo(adr, ptr, length );
  }
}

void SX1276ReadFifo( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS=0
    MRFI_SPI_DRIVE_CSN_LOW();

    write1byte_to_sx1278(addr&0x7F);

    for( i = 0; i < size; i++ )
    {
        buffer[i] = write1byte_to_sx1278(0x00);
    }

    //NSS = 1;
    MRFI_SPI_DRIVE_CSN_HIGH();
}



void BurstWrite(u8 adr, u8 *ptr, u8 length)
{ 

  if(length<=1)                                            //length must more than one
    return;
  else  
  {   
   SX1276WriteFifo( adr, ptr, length );
  }
}

void SX1276WriteFifo( uint8_t addr, uint8_t *ptr, uint8_t size )
{
    uint8_t i;
    //NSS=0
    MRFI_SPI_DRIVE_CSN_LOW();
    write1byte_to_sx1278(addr | 0x80);
    for( i = 0; i < size; i++ )
    {
        write1byte_to_sx1278(ptr[i]);
    }
    //NSS = 1;
    MRFI_SPI_DRIVE_CSN_HIGH();
}


/**********************************************************
**Name:     SX1276_Standby
**Function: Entry standby mode
**Input:    None
**Output:   None
**********************************************************/
void SX1276_Standby(void)
{
	
  SPIWrite(LR_RegOpMode+0x01+0x08);                              //Standby
	
}

/**********************************************************
**Name:     SX1276_Sleep
**Function: Entry sleep mode
**Input:    None
**Output:   None
**********************************************************/
void SX1276_Sleep(void)
{
  SPIWrite(LR_RegOpMode+0x00+0x08);                              //Sleep
}

/*********************************************************/
//LoRa mode
/*********************************************************/
/**********************************************************
**Name:     SX1276_EntryLoRa
**Function: Set RFM69 entry LoRa(LongRange) mode
**Input:    None
**Output:   None
**********************************************************/
void SX1276_EntryLoRa(void)
{
  SPIWrite(LR_RegOpMode+0x80+0x08);
}

/**********************************************************
**Name:     SX1276_LoRaClearIrq
**Function: Clear all irq
**Input:    None
**Output:   None
**********************************************************/
void SX1276_LoRaClearIrq(void)
{
  SPIWrite(LR_RegIrqFlags+0xFF);
}

/**********************************************************
**Name:     SX1276_Config
**Function: SX1276 base config
**Input:    mode
**Output:   None
**********************************************************/
void SX1276_Config(uint8_t mode)
{
  u8 i; 
    
  GPIO_ResetBits(RESET_IO_GROUP,RESET_IO_PORT);                        //RFSET=0 
  for(i=150;i!=0;i--)                                      //Delay
    __ASM("nop");
  
  GPIO_SetBits(RESET_IO_GROUP,RESET_IO_PORT);                          //RFSET=1    复位
  
  for(i=100;i!=0;i--)                                      //Delay
    __ASM("nop");  
    
  SX1276_Sleep();                                           //Change modem mode Must in Sleep mode 
  for(i=100;i!=0;i--)                                      //Delay
    __ASM("nop");  

   SPIWrite(REG_LR_TCXO+0x09);                              //USE TCXO

	 i=SPIRead((u8)(REG_LR_TCXO>>8));
	
    SX1276_EntryLoRa();  
    //SPIWrite(0x5904);   //?? Change digital regulator form 1.6V to 1.47V: see errata note
    
    for(i=0;i<3;i++)                                       //setting frequency parameter
    {
      SPIWrite(SX1276FreqTbl[0][i]);  
    }

    //setting base parameter 
    SPIWrite(SX1276PowerTbl[0]);             //Setting output power parameter  
    
    SPIWrite(LR_RegOcp+0x0B);                              //RegOcp,Close Ocp
    SPIWrite(LR_RegLna+0x23);                              //RegLNA,High & LNA Enable

	  i=SPIRead((u8)(LR_RegOcp>>8));
    
    if(SX1276SpreadFactorTbl[gb_SF]==6)           //SFactor=6
    {
      u8 tmp;
      SPIWrite(LR_RegModemConfig1+(SX1276LoRaBwTbl[gb_BW]<<4)+(CR<<1)+0x01);//Implicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)
      SPIWrite(LR_RegModemConfig2+(SX1276SpreadFactorTbl[gb_SF]<<4)+(CRC2<<2)+0x03);
      
      tmp = SPIRead(0x31);
      tmp &= 0xF8;
      tmp |= 0x05;
      SPIWrite(0x3100+tmp);
      SPIWrite(0x3700+0x0C);
    } 
    else
    {
      SPIWrite(LR_RegModemConfig1+(SX1276LoRaBwTbl[gb_BW]<<4)+(CR<<1)+0x00);//Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)
      SPIWrite(LR_RegModemConfig2+(SX1276SpreadFactorTbl[gb_SF]<<4)+(CRC2<<2)+0x03);  //SFactor &  LNA gain set by the internal AGC loop 
      SPIWrite(LR_RegModemConfig3+0x00);//LowDataRateOptimize en
    }
    SPIWrite(LR_RegSymbTimeoutLsb+0xFF);                   //RegSymbTimeoutLsb Timeout = 0x3FF(Max) 
    
    SPIWrite(LR_RegPreambleMsb + 0);                       //RegPreambleMsb 
    SPIWrite(LR_RegPreambleLsb + 6);                      //RegPreambleLsb 8+4=12byte Preamble  16+4=20
    
//		SPIWrite(LR_RegFrMsb + 0x6c); 
//		SPIWrite(LR_RegFrMid + 0x88); 
//		SPIWrite(LR_RegFrLsb + 0x01); 
		
//    SPIWrite(REG_LR_DIOMAPPING2+0x01);                     //RegDioMapping2 DIO5=00, DIO4=01
    SX1276_Standby();                                         //Entry standby mode
}

/**********************************************************
**Name:     SX1276_LoRaEntryRx
**Function: Entry Rx mode
**Input:    None
**Output:   None
**********************************************************/
void SX1276_LoRaEntryRx(uint8_t mrfiChannelNum)
{
  uint8_t addr; 
  uint8_t i;      
//  SX1276_Config(0);                                         //setting base parameter
	
	SX1276_Standby();
	
	for(i=0;i<3;i++)                                       //setting frequency parameter
  {
    SPIWrite(SX1276FreqTbl[mrfiChannelNum][i]);  
  }
	
	SPIWrite(LR_RegFifoRxBaseAddr+0x02); 
  SPIWrite(LR_RegFifoTxBaseAddr+0xFD); 
	
  SPIWrite(REG_LR_DIOMAPPING2+0x01);                     //RegDioMapping2 DIO5=00, DIO4=01
	
  SPIWrite(0x4D00+0x84);                                   //Normal and Rx
  SPIWrite(LR_RegHopPeriod+0xFF);                          //RegHopPeriod NO FHSS
  SPIWrite(REG_LR_DIOMAPPING1+0x01);                       //DIO0=00, DIO1=00, DIO2=00, DIO3=01  DIO0=00--RXDONE
      
  SPIWrite(LR_RegIrqFlagsMask+0x3F);                       //Open RxDone interrupt & Timeout
  SX1276_LoRaClearIrq();   
  
  SPIWrite(LR_RegPayloadLength+datlen);                       //RegPayloadLength  21byte(this register must difine when the data long of one byte in SF is 6)
    
  addr = SPIRead((u8)(LR_RegFifoRxBaseAddr>>8));           //Read RxBaseAddr
  SPIWrite(LR_RegFifoAddrPtr+addr);                        //RxBaseAddr -> FiFoAddrPtr　 
  SPIWrite(LR_RegOpMode+0x0D);                        //Continuous Rx Mode

//  delayms(10);
	__ASM("nop"); 
	__ASM("nop"); 
	__ASM("nop"); 
}

/**********************************************************
**Name:     SX1276_LoRaRxWaitStable
**Function: Determine whether the state of stable Rx 查询RX 状态
**Input:    none
**Output:   none
**********************************************************/
u8 SX1276_LoRaRxWaitStable(void)
{ 
	uint8_t tmp;
	tmp=SPIRead((u8)(LR_RegModemStat>>8));
	return tmp;
}


/**********************************************************
**Name:     Mrfi_ReadRxFifoData
**Function: Receive data in LoRa mode
**Input:    None
**Output:   None
**********************************************************/
void Mrfi_ReadRxFifoData(uint8_t *pData, uint8_t len,uint8_t length_ofs)
{
		uint8_t addr;

		addr = SPIRead((u8)(LR_RegFifoRxCurrentaddr>>8));//last packet addr 数据包的最后地址(数据的尾地址)
		SPIWrite(LR_RegFifoAddrPtr+addr+length_ofs);//RxBaseAddr -> FiFoAddrPtr   
	
	  SPIBurstRead(0x00,pData, len);                   //读取FIFO数据到全局变量mrfiIncomingPacket

		SX1276_LoRaClearIrq();	
}

/**********************************************************
**Name:     SX1276_LoRaEntryTx
**Function: Entry Tx mode
**Input:    None
**Output:   None
**********************************************************/
void SX1276_LoRaEntryTx(uint8_t * pPacket,uint8_t data_len,uint8_t mrfiChannelNum )
{
		uint8_t addr;
	  uint8_t i;
//		SX1276_Config(0);                                         //setting base parameter
    //data_len=(*pPacket)+LENGTH_FIELD_LEN;
	  SX1276_Standby(); 
	
    for(i=0;i<3;i++)                                       //setting frequency parameter
    {
      SPIWrite(SX1276FreqTbl[mrfiChannelNum][i]);  
    }	
	
		SPIWrite(LR_RegFifoRxBaseAddr+0xFD); 
    SPIWrite(LR_RegFifoTxBaseAddr+0x02); 
		
	  SPIWrite(REG_LR_DIOMAPPING2+0x01);                     //RegDioMapping2 DIO5=00, DIO4=01
	
		SPIWrite(0x4D00+0x87);                                   //Tx for 20dBm
		SPIWrite(LR_RegHopPeriod);                               //RegHopPeriod NO FHSS
		SPIWrite(REG_LR_DIOMAPPING1+0x41);                       //DIO0=01, DIO1=00, DIO2=00, DIO3=01

		SX1276_LoRaClearIrq();
		SPIWrite(LR_RegIrqFlagsMask+0xF7);            	//Open TxDone interrupt
		SPIWrite(LR_RegPayloadLength+ data_len);                       //RegPayloadLength  21byte

		addr = SPIRead((u8)(LR_RegFifoTxBaseAddr>>8));           //RegFiFoTxBaseAddr
		SPIWrite(LR_RegFifoAddrPtr+addr);                        //RegFifoAddrPtr
}
uint16_t TxPacketCout=1;
/**********************************************************
**Name:     SX1276_LoRaTxPacket
**Function: Send data in LoRa mode
**Input:    None
**Output:   1- Send over
**********************************************************/
void SX1276_LoRaTxPacket(uint8_t * pPacket, uint8_t data_len)
{ 
	  //uint8_t addr;
		uint8_t tmp=2;
    //uint8_t data_len=*pPacket;	
    //uint8_t data;
	
	  //SPIWrite(LR_RegPayloadLength+ data_len); 
		//addr = SPIRead((uint8_t)(LR_RegFifoTxBaseAddr>>8));           //RegFiFoTxBaseAddr
		//SPIWrite(LR_RegFifoAddrPtr+addr);                        //RegFifoAddrPtr
	
//		delayms(10);
	  __ASM("nop"); 
	  __ASM("nop"); 
	  __ASM("nop"); 
	
		//SX1276Data[0]=(TxPacketCout>>8);
		//SX1276Data[1]=(uint8_t)(TxPacketCout);
		TxPacketCout++;
		
		for(tmp=0;tmp<data_len;tmp++){
			//array_data=tmp-1;
			SX1276Data[tmp]=* pPacket;
			pPacket=pPacket+1;
    }
                
		BurstWrite(0x00, (uint8_t *)SX1276Data, data_len);
		SPIWrite(LR_RegOpMode+0x03+0x08);                    //Tx Mode       
}

/**********************************************************
**Name:     SX1276_LoRaCAD
**Function: Entry CAD mode
**Input:    None
**Output:   None
**********************************************************/
void SX1276_LoRaCAD_Init(void)
{
   SPIWrite(LR_RegOpMode+0x01+0x08);                              //Standby
	 SPIWrite(LR_RegIrqFlagsMask+0xFA);            	                //Open CadDone interrupt  and CadDetected
	 SPIWrite(REG_LR_DIOMAPPING1+0x50);                             //DIO0=01, DIO1=01, DIO2=00, DIO3=00       CadDone&CadDetected DIO映射    
	 SPIWrite(REG_LR_DIOMAPPING2+0x01);                             //RegDioMapping2 DIO5=00, DIO4=00
}
/**********************************************************
**Name:     SX1276_LoRa_CAD
**Function: Entry CAD mode
**Input:    None
**Output:   None
**********************************************************/
void SX1276_LoRa_CAD(void)
{
  SPIWrite(LR_RegOpMode+0x07+0x08);                              //CAD MODE
}


///////////////////////////////////////////////// FSK mode //////////////////////////////////////////////////
const u16 SX1276ConfigTbl[16] = 
{ 
  0x0402,                   //RegFdevMsb 	35KHz 
  0x053D,                   //RegFdevLsb
  0x0B0B,                   //RegOcp 	Close Ocp
  //0x0C20,                 //RegLNA 	High & LNA Disable
  0x0C23,                   //RegLNA	High & LNA Enable
  0x1212,                   //RegRxBw		83KHz
  0x1FA0,                   //RegPreambleDet	Enable 2Byte 
  //0x1F20,                 //RegPreambleDet	Disable 
  0x2500,                   //RegPreambleMsb  
  0x2606,                   //RegPreambleLsb	6Byte Preamble
  0x2792,                   //RegSyncConfig	Sync 2+1=3bytes
  0x2800+0xAA,              //SyncWord = aa2dd4
  0x2900+0x2D,              //
  0x2A00+0xD4,              //
  0x3000,                   //RegPacketConfig1  Disable CRC，NRZ
  0x3140,                   //RegPacketConfig2  Packet Mode
  0x3215,                   //RegPayloadLength  21bytes Fixed
  0x3595,                   //RegFiFoThresh		21bytes                        
};

const u16 SX1276FSKRateTbl[4][2] = 
{
  {0x0268, 0x032B},         //BR=1.2Kbps
  {0x0234, 0x0315},         //BR=2.4Kbps
  {0x021A, 0x030B},         //BR=4.8Kbps
  {0x020D, 0x0305},         //BR=9.6Kbps
};

const u16 SX1276RxTable[4] = 
{       
  0x090F,                   //RFIO Pin
  0x400C,                   //DIO0 Mapping for IRQ / DIO2 for RxData
  0x4100,                   //
  0x4D84,                   //Normal and Rx   
};
                              
const u16 SX1276TxTable[3] = 
{
  0x4000,                   //DIO0 Mapping for IRQ / DIO2 for RxData
  0x4100,                   //          
  0x4D87,                   //20dBm Tx
};
/**********************************************************
**Name:     SX1276_Config
**Function: SX1276 base config
**Input:    mode
**Output:   None
**********************************************************/
void SX1276_FskConfig()
{
  u8 i; 

 GPIO_ResetBits(RESET_IO_GROUP,RESET_IO_PORT);      //RFSET=0
  for(i=100;i!=0;i--)                                      //Delay
   __ASM("nop"); 
  
  GPIO_SetBits(RESET_IO_GROUP,RESET_IO_PORT);      //RFSET=1
  
  for(i=250;i!=0;i--)                                      //Delay
   __ASM("nop"); 
    
  SX1276_Sleep();                                           //Change modem mode Must in Sleep mode 
  for(i=250;i!=0;i--)                                      //Delay
    __ASM("nop"); 
  
    for(i=0;i<3;i++)                                       //setting frequency parameter
    {
      SPIWrite(SX1276FreqTbl[0][i]);  
    }
	
    SPIWrite(SX1276PowerTbl[0]);             //Setting output power parameter
    
    for(i=0;i<16;i++)                                      //setting base parameter
      SPIWrite(SX1276ConfigTbl[i]);

  SX1276_Standby();                                         //Entry standby mode
}

/**********************************************************
**Name:     SX1276_FskClearFIFO
**Function: Change to RxMode from StandbyMode, can clear FIFO buffer
**Input:    None
**Output:   None
**********************************************************/
void SX1276_FskClearFIFO(void)
{
  SPIWrite(0x0101);                                        //Standby
  SPIWrite(0x0105+0x08);                                   //entry RxMode
}

/**********************************************************
**Name:     SX1276_FskEntryRx
**Function: Set RFM69 entry FSK Rx_mode
**Input:    None
**Output:   "0" for Error Status
**********************************************************/
void SX1276_FskEntryRx(void)
{
  u8 i;
  
  SX1276_FskConfig();  
  for(i=0;i<2;i++)
    SPIWrite(SX1276FSKRateTbl[0][i]);         //setting rf rate parameter  1.2kbps
  for(i=0;i<4;i++)                                         //Define to Rx mode 
    SPIWrite(SX1276RxTable[i]);    
  SPIWrite(0x0105+0x08);                                   //entry RxMode
  
}


/**********************************************************
**Name:     SX1276_FskRxPacket
**Function: Check for receive one packet
**Input:    none
**Output:   "!0"-----Receive one packet
**          "0"------Nothing for receive
**********************************************************/
u8 SX1276_FskRxPacket(void)
{
  //u8 i; 
    
 /* if(RF_IRQ_DIO0)
  { 
    for(i=0;i<32;i++) 
      gb_RxData[i] = 0x00;  
    
    SPIBurstRead(0x00, gb_RxData, 21);  
    SX1276_FskClearFIFO();
    for(i=0;i<17;i++)
    {
      if(gb_RxData[i]!=SX1276Data[i])
        break;  
    }
    if(i>=17) 
      return(1);                                           //Rx success
    else
      return(0);
  }
  else  */
    return(0);     
}




/**********************************************************
**Name:     SX1276_FskEntryTx
**Function: Set SX1276 entry FSK Tx_mode
**Input:    None
**Output:   "0" for Error Status
**********************************************************/
void SX1276_FskEntryTx(void)
{
  u8 i;
  
  SX1276_FskConfig();
  for(i=0;i<2;i++)
    SPIWrite(SX1276FSKRateTbl[0][i]);         //setting rf rate parameter  
  for(i=0;i<3;i++)                                         //Define to Tx mode 
    SPIWrite(SX1276TxTable[i]);
  SPIWrite(0x0103+0x08);
      
}

/**********************************************************
**Name:     SX1276_FskTxPacket
**Function: Check SX1276 send over & send next packet
**Input:    none
**Output:   TxFlag=1, Send success
**********************************************************/
void SX1276_FskTxPacket(void)
{
		
		BurstWrite(0x00, (u8 *)SX1276Data, 21);     
		SPIWrite(0x0103+0x08);                               //Entry Tx mode

		__ASM("nop");	

	//	while(!RF_IRQ_DIO0) ;                    //Packet send over 发送完成了IRQ 变为H,平时L

		SX1276_Standby();                                     //Entry Standby mode      

  
}



/**********************************************************
**Name:     SX1276_LoRaRx_On
**Function: Entry Rx mode
**Input:    None
**Output:   None
**********************************************************/
void SX1276_LoRaRx_On(uint8_t mrfiChannelNum)
{
  
//  SX1276_Config(0);                                         //setting base parameter
  uint8_t i;
	SX1276_Standby(); 
	
  for(i=0;i<3;i++)                                       //setting frequency parameter
  {
    SPIWrite(SX1276FreqTbl[mrfiChannelNum][i]);  
  }
	
	SPIWrite(REG_LR_DIOMAPPING2+0x01);                     //RegDioMapping2 DIO5=00, DIO4=01                                        //Entry standby mode
	
  SPIWrite(0x4D00+0x84);                                   //Normal and Rx
  SPIWrite(LR_RegHopPeriod+0xFF);                          //RegHopPeriod NO FHSS
  //SPIWrite(REG_LR_DIOMAPPING1+0x01);                       //DIO0=00, DIO1=00, DIO2=00, DIO3=01  DIO0=00--RXDONE
      
  //SPIWrite(LR_RegIrqFlagsMask+0x3F);                       //Open RxDone interrupt & Timeout
  SX1276_LoRaClearIrq();     
  SPIWrite(LR_RegPayloadLength+datlen);                       //RegPayloadLength  21byte(this register must difine when the data long of one byte in SF is 6)
  SPIWrite(LR_RegOpMode+0x0D);                        //Continuous Rx Mode
//  delayms(10);
	__ASM("nop");
	__ASM("nop");
	__ASM("nop");
}


//uint8_t Mrfi_GetRxFifoByteNum(void)
//{
//	uint8_t packet_size;
//  if(SX1276SpreadFactorTbl[gb_SF]==6)//When SpreadFactor is six，will used Implicit Header mode(Excluding internal packet length)
//	{
//		packet_size=21;
//	}
//	else
//	{
//		packet_size = SPIRead((u8)(LR_RegRxNbBytes>>8));//Number for received bytes    获取整个数据包的长度，包括是首字节
//	}
//	return packet_size;
//}

void Mrfi_ReadRxFifo(uint8_t *pData, uint8_t len)
{
	uint8_t addr;
  addr = SPIRead((u8)(LR_RegFifoRxCurrentaddr>>8));//last packet addr 数据包的最后地址(数据的尾地址)
	SPIWrite(LR_RegFifoAddrPtr+addr);//RxBaseAddr -> FiFoAddrPtr   

	SPIBurstRead(0x00,pData, len);     //读取FIFO首字节获取整个数据域长度，不包括首字节    SPIBurstRead函数读取字节数必须大于0，否则会直接返回
		
//	SPIWrite(LR_RegFifoAddrPtr+addr);     //RxBaseAddr -> FiFoAddrPtr       重新将读取指针指向数据包起始地址	
}



void Get_Radio_fre(uint8_t* freq_temp)
{
//  *freq_temp=SPIRead((u8)(LR_RegFrLsb>>8));
//	*(freq_temp+1)=SPIRead((u8)(LR_RegFrMid>>8));
//	*(freq_temp+2)=SPIRead((u8)(LR_RegFrMsb>>8));
	*freq_temp=0x00;
	*(freq_temp+1)=0x80;
	*(freq_temp+2)=0x6c;
  *(freq_temp+3)=0x00;
}

void Get_Radio_Rate(uint8_t * rate_temp)
{
	*rate_temp=(SPIRead((u8)(LR_RegModemConfig2>>8))>>4)&0x0F;
	*(rate_temp+1)=(SPIRead((u8)(LR_RegModemConfig1>>8))>>4)&0x0F;
	*(rate_temp+2)=(SPIRead((u8)(LR_RegModemConfig1>>8))>>1)&0x07;
}

void Get_Radio_mode(uint8_t * modformat_temp)
{
	 uint8_t mode;
   mode=SPIRead((u8)(LR_RegOpMode>>8));
	 *modformat_temp=((mode>>7)&(0x01));
}

void Get_Radio_Txpower(uint8_t* txpw_temp)
{
	uint8_t Pa_config;
  Pa_config=SPIRead((u8)(LR_RegPaConfig>>8));
	__ASM("nop");
	__ASM("nop");
	__ASM("nop");
	Pa_config=Pa_config&0x0F;
	*txpw_temp=20-(15-Pa_config);
}

void Set_Radio_Txpower(uint8_t* txpw_temp)
{
  uint8_t Pa_config;
	Pa_config=(15-(20-*txpw_temp))|0xF0;
	
	SPIWrite(LR_RegPaConfig+Pa_config);
	
	__ASM("nop");
	__ASM("nop");
	__ASM("nop");	
	
}



