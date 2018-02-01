/**************************************************************************************************
  Revised:        $Date: 2007-07-06 11:19:00 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

**************************************************************************************************/

/* ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 *   MRFI (Minimal RF Interface)
 *   Include file for all MRFI services.
 * ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 */

#ifndef MRFI_H
#define MRFI_H


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"
#include "mrfi_defs.h"
#include "smpl_nwk_config.h"  //Added By LEI Di, 2014/9/19
#include "mrfi_config.h"      //Added By LEI Di, 2014/11/10

/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MRFI_NUM_LOGICAL_CHANS           __mrfi_NUM_LOGICAL_CHANS__

#define MRFI_NUM_POWER_SETTINGS          __mrfi_NUM_POWER_SETTINGS__

/* return values for MRFI_Transmit */
#define MRFI_TX_RESULT_SUCCESS        0
#define MRFI_TX_RESULT_FAILED         1

/* transmit type parameter for MRFI_Transmit */
#define MRFI_TX_TYPE_FORCED           0
#define MRFI_TX_TYPE_CCA              1//1

/* Network header size definition */
/* *********************************  NOTE  ****************************************
 * There is a dependency here that really shouldn't exist. A reimplementation
 * is necessary to avoid it.
 *
 * MRFI allocates the frame buffer which means it needs to know at compile time
 * how big the buffer is. This means in must know the NWK header size, the
 * maximum NWK and User application payload sizes and whether Security is enabled.
 * ********************************************************************************
 */
#ifndef SMPL_SECURE
#define  NWK_HDR_SIZE   3
#define  NWK_PAYLOAD    MAX_NWK_PAYLOAD
#else
#define  NWK_HDR_SIZE   6
#define  NWK_PAYLOAD    (MAX_NWK_PAYLOAD+4)
#endif

/* if external code has defined a maximum payload, use that instead of default */
#ifdef MAX_APP_PAYLOAD
#ifndef MAX_NWK_PAYLOAD
#error ERROR: MAX_NWK_PAYLOAD not defined
#endif
#if MAX_APP_PAYLOAD < NWK_PAYLOAD
#define MAX_PAYLOAD  NWK_PAYLOAD
#else
#define MAX_PAYLOAD  MAX_APP_PAYLOAD
#endif
#define MRFI_MAX_PAYLOAD_SIZE  (MAX_PAYLOAD+NWK_HDR_SIZE) /* swsn payload size plus six byte overhead */
#endif


/* frame definitions */
#define MRFI_ADDR_SIZE              __mrfi_ADDR_SIZE__
#ifndef MRFI_MAX_PAYLOAD_SIZE
#define MRFI_MAX_PAYLOAD_SIZE       __mrfi_MAX_PAYLOAD_SIZE__
#endif
#define MRFI_MAX_FRAME_SIZE         (MRFI_MAX_PAYLOAD_SIZE + __mrfi_FRAME_OVERHEAD_SIZE__)
#define MRFI_RX_METRICS_SIZE        __mrfi_RX_METRICS_SIZE__
#define MRFI_RX_METRICS_RSSI_OFS    __mrfi_RX_METRICS_RSSI_OFS__
#define MRFI_RX_METRICS_CRC_LQI_OFS __mrfi_RX_METRICS_CRC_LQI_OFS__

/* Radio States */
#define MRFI_RADIO_STATE_UNKNOWN        0
#define MRFI_RADIO_STATE_RESERVE        1
#define MRFI_RADIO_STATE_STDBY_RC       2
#define MRFI_RADIO_STATE_STDBY_XOSC     3
#define MRFI_RADIO_STATE_FS             4
#define MRFI_RADIO_STATE_RX             5
#define MRFI_RADIO_STATE_TX             6
#define MRFI_RADIO_STATE_OFF            7

/*-------------LORA  Abstraction--------------*/
//#define LED_ON    GPIO_ResetBits(GPIOB,GPIO_Pin_10)
//#define LED_OFF   GPIO_SetBits(GPIOB,GPIO_Pin_10)

#define TX_LED_PORT                     GPIO_Pin_0
#define TX_LED_GROUP                    GPIOC
#define RX_LED_PORT                     GPIO_Pin_2
#define RX_LED_GROUP                    GPIOC


#define LORA_RX_ENABLE	GPIO_SetBits(GPIOB,GPIO_Pin_9);\
	                      GPIO_ResetBits(GPIOB,GPIO_Pin_11);
#define LORA_TX_ENABLE	GPIO_ResetBits(GPIOB,GPIO_Pin_9);\
	                      GPIO_SetBits(GPIOB,GPIO_Pin_11);
												
#define RESET_IO_PORT                   GPIO_Pin_6         
#define RESET_IO_GROUP                  GPIOB

#define LORA_DIO0_PORT                  GPIO_Pin_10
#define LORA_DIO0_GROUP                 GPIOC
/* Platform constant used to calculate worst-case for an application
 * acknowledgment delay. Used in the NWK_REPLY_DELAY() macro.
 *

                                      processing time on peer
                                      |   round trip
                                      |   |      max number of replays
                                      |   |      |             number of backoff opportunities
                                      |   |      |             |         average number of backoffs
                                      |   |      |             |         |                                    */
#define   PLATFORM_FACTOR_CONSTANT   (2 + 2*(MAX_HOPS*(MRFI_CCA_RETRIES*(8*MRFI_BACKOFF_PERIOD_USECS)/1000)))


/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MRFI_GET_PAYLOAD_LEN(p)         __mrfi_GET_PAYLOAD_LEN__(p)
#define MRFI_SET_PAYLOAD_LEN(p,x)       __mrfi_SET_PAYLOAD_LEN__(p,x)

#define MRFI_P_DST_ADDR(p)              __mrfi_P_DST_ADDR__(p)
#define MRFI_P_SRC_ADDR(p)              __mrfi_P_SRC_ADDR__(p)
#define MRFI_P_PAYLOAD(p)               __mrfi_P_PAYLOAD__(p)

/* ------------------------------------------------------------------------------------------------
 *                                          Typdefs
 * ------------------------------------------------------------------------------------------------
 */
typedef struct
{
  uint8_t frame[MRFI_MAX_FRAME_SIZE];
  uint8_t rxMetrics[MRFI_RX_METRICS_SIZE];
} mrfiPacket_t;

typedef struct
{
  uint8_t  Ed_Addr[4];
  uint8_t  Tx_Time_info[2];
	uint8_t  Rx_Time_info[2];
} Data_Frm_Time;	
/* ------------------------------------------------------------------------------------------------
 *                                         Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void    Lora_Init(void);
void    MRFI_Init(void);
uint8_t MRFI_Transmit(mrfiPacket_t *, uint8_t);

uint8_t Lora_Transmit(mrfiPacket_t *, uint8_t);
void Lora_Receive(void);

void    MRFI_Receive(mrfiPacket_t *);
void    MRFI_RxCompleteISR(void); /* populated by code using MRFI */
uint8_t MRFI_GetRadioState(void);
uint8_t MRFI_GetPhInterruptState(void);
void    MRFI_RxOn(void);
void    MRFI_Ready(void);
int8_t  MRFI_Rssi(void);
void    MRFI_SetLogicalChannel(uint8_t);
uint8_t  MRFI_GetLogicalChannel(void);
uint8_t MRFI_SetRxAddrFilter(uint8_t *);
void    MRFI_EnableRxAddrFilter(void);
void    MRFI_DisableRxAddrFilter(void);
void    MRFI_Sleep(void);
void    MRFI_WakeUp(void);
uint8_t MRFI_RandomByte(void);
void    MRFI_DelayMs(uint16_t);
void    MRFI_ReplyDelay(void);
void    MRFI_PostKillSem(void);
void    MRFI_SetRFPwr(uint8_t);
uint8_t MRFI_getFrameTransmissionCount(void);
uint8_t MRFI_getFrameReceiveCount(void);
void MRFI_SetChannelNum(uint8_t );
uint8_t MRFI_GetChannelNum(void);


/* ------------------------------------------------------------------------------------------------
 *                                       Global Constants
 * ------------------------------------------------------------------------------------------------
 */
extern const uint8_t mrfiBroadcastAddr[];
extern const uint8_t mrfiRemoterAddr[];

typedef struct
{
  uint8_t pre;
  uint8_t cur;
} time_sync_cnt_t;



/**************************************************************************************************
 */
#endif


