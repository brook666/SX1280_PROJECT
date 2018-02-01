#ifndef _MAIN_H_
#define _MAIN_H_

#ifdef _MAIN_C_
#define _MAIN_C_
#else
#define _MAIN_C_		extern
#endif
/**********************************************************************/


#define cTx_Statue			0X10 //
#define cSx1276RxStatue   0x8D
#define CRX		0X01 //0--TX   1--RX  这里选择是发射  还是  接收
#define S_BW10        	PA_IDR_IDR1 //INPUT
#define S_BW62        	PA_IDR_IDR2 //INPUT
#define S_BW125        PB_IDR_IDR3 //INPUT
#define S_FSK        	PB_IDR_IDR7 //INPUT
#define S_Tx_Rx        PC_IDR_IDR1 //INPUT
#define Sx1276_RxMode        		0
#define Sx1276_TxMode      		1

//-----------------------------------------------------------------------------
// 变量声明
//-----------------------------------------------------------------------------
uint8 D_value[5];
uint8 Tx_Rx_Choice_Bak;
uint8 Tx_phase;
uint8 Contbak;
uint8 Sx1276Statue;
uint8 SX1276_GetStatue_Cout;
uint8 ContBak=0xff;
uint8 Sx1276VerNO ;
uint8 TxTimeCout;
uint8 Trg;
uint8 Cont;
uint8 gz8_RXbuffer[85];
uint8 KeyScanCnt;	
uint8 Chargcout;
uint8 Tx_TimeCout;
uint8 Rx_phase;
uint8 RxPakCout;
uint8 PER_phase;
UU16  PacketNuNow;
UU88  PacketNuStart;
uint8 PER;
uint8 RxPacketCout;
uint8 PER_phase;
UU16  PacketNuNow;
//UU16  PacketNuStart;
UU88  PacketNuStart;
uint8 PER;
uint8 RxPacketCout;
uint8 Flg_RxFinish;
extern u8  gb_RxData[254]; //Receive data buffer
extern u8  gb_TxData[254];

#if 0
__root const u8  cPerStart[] = {"开始PER统计..."};
__root const u8  PERDON[] = {"PER DON..."};
__root const u8  GET[] = {"CRC_OK的包数:"};
__root const u8  Lost[] = {"   丢包数:"};
__root const u8  Toatl[] = {"  总共发包数:"};
__root const u8  cPer[] = {"  PER="};
#endif

__root const u8  cPerStart[] = {"--------StartCout PER--------\x0D\x0A"};
__root const u8  cPERDON[] = {"PER DON..."};
__root const u8  cGET[] = {"Rec_Packet:"};
__root const u8  cLost[] =  {"    Lost_Packet:"};
__root const u8  cToatl[] = {"    Toatl_Packet:"};
__root const u8  cPer[] =  {"    PER="};


 union 
  {
    unsigned char Flg;
    struct
      {unsigned char  Flg_S_BW10:1,
                      Flg_S_BW62:1,
                      Flg_S_BW125:1,
                      Flg_S_FSK :1,
                      Flg_S_Tx_Rx:1, //LSB
                      Flg_SX1276_RXorTX:1,
                      Flg_RFSleep:1,
                      Flg_RFDown:1;
       };
  };
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
void RF_GpioInt();
void InitRF_Tx(void);
void RFTransmit(void);
void RFIdle(void);

#endif
