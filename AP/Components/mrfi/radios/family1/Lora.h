#ifndef _LORA_H_
#define _LORA_H_

#ifdef _LORA_C_
#define _LORA_C_
#else
#define _LORA_C_		extern
#endif
/**********************************************************************/

#include "stdint.h"

#define cTx_Statue			0X10 //
#define cSx1276RxStatue   0x8D
#define CRX		0X01 //0--TX   1--RX  这里选择是发射  还是  接收
#define Sx1276_RxMode        		0
#define Sx1276_TxMode      		1

//-----------------------------------------------------------------------------
// 变量声明
//-----------------------------------------------------------------------------
uint8_t D_value[5];
uint8_t Tx_Rx_Choice_Bak;
uint8_t Tx_phase;
uint8_t Contbak;
uint8_t Sx1276Statue;
uint8_t SX1276_GetStatue_Cout;
uint8_t ContBak=0xff;
uint8_t Sx1276VerNO ;
uint8_t TxTimeCout;
uint8_t Trg;
uint8_t Cont;
uint8_t gz8_RXbuffer[85];
uint8_t KeyScanCnt;	
uint8_t Chargcout;
uint8_t Tx_TimeCout;
uint8_t Rx_phase;
uint8_t RxPakCout;
uint8_t PER_phase;
uint8_t PER;
uint8_t RxPacketCout;
uint8_t PER_phase;

//UU16  PacketNuStart;
uint8_t PER;
uint8_t RxPacketCout;
uint8_t Flg_RxFinish;
extern uint8_t  gb_RxData[254]; //Receive data buffer
extern uint8_t  gb_TxData[254];


typedef enum
{
	Rx_Parameters_Set,		
	Wait_RxFinish		//
}RX_STATE;

typedef enum
{
	PerStart,		
	PerGoOn,
	PerDone//
}PER_PHASE;

typedef enum
{
	SetTx_Parameters,		// 接收模式光标PER_phase
	Write_FIFO_AndTx,		// 设置模式光标
	Wait_Tx_Finish,
	Wait_NextTx
}TX_STATE;


//-----------------------------------------------------------------------------
// 子程序声明
//-----------------------------------------------------------------------------
unsigned short g_CurrentDispPage ,g_TotalDispPage, g_IDH, g_IDL,RF_CH,Page_time_flash;
unsigned char phyRead (unsigned char u8Addr);
void InitRF_Rx(void);
void RF_GpioInt(void);
void InitRF_Tx(void);
void RFTransmit(void);
void RFIdle(void);

#endif
