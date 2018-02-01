/**************************************************************************************************
  Revised:        $Date: 2009-01-13 12:04:00 -0700 (Wed, 13 Jan 2009) $
  Revision:       $Revision: 18768 $

**************************************************************************************************/

/* ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 *   MRFI (Minimal RF Interface)
 *   Radios: Si4438, Si4463
 *   Primary code file for supported radios.
 * ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 */

/*
 * History:     2014-9-24   Leidi 修改调试信息.
 *                                新增加了PA控制的相关宏定义.
 *                                将MRFI_MAX_DELAY_US定义为12,似乎该值可以适用于8MHz和16MHz的情况.
 *                                移除有关过滤无效AP的代码,放在nwk_frame.c中进行.
 *              2014-11-10  Leidi 移除头文件stm8s_uart.h.
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <string.h>
#include "mrfi.h"
#include "bsp.h"
#include "bsp_macros.h"
#include "bsp_external/mrfi_board_defs.h"
#include "mrfi_defs.h"
#include "mrfi_spi.h"
#include "../common/mrfi_f1f2.h"
#include "Lora.h"
#include "sx1276-Lora.h"
#include "bsp_leds.h"
#include "stdlib.h"
#include "radio.h"
#include "radio_config_Sx1280.h"
#include "sx1280.h"
/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */

#if (defined MRFI_SI4438)

 #define MRFI_RSSI_OFFSET    64   /* no units */

 /* Worst case wait period in RX state before RSSI becomes valid.
  * These numbers are from Design Note DN505 with added safety margin.
  */
 #define MRFI_RSSI_VALID_DELAY_US    1000

#elif (defined MRFI_SI4463)

  #define MRFI_RSSI_OFFSET    64   /* no units */

 /* Worst case wait period in RX state before RSSI becomes valid.
  * These numbers are from Design Note DN505 with added safety margin.
  */
  #define MRFI_RSSI_VALID_DELAY_US    1000

#else
  #error "ERROR: RSSI offset value not defined for this radio"
#endif


#define RSSI_HF_MODE                       157
#define RSSI_LF_MODE                       164

#define MRFI_LENGTH_FIELD_OFS               __mrfi_LENGTH_FIELD_OFS__
#define MRFI_LENGTH_FIELD_SIZE              __mrfi_LENGTH_FIELD_SIZE__
#define MRFI_HEADER_SIZE                    __mrfi_HEADER_SIZE__
#define MRFI_FRAME_BODY_OFS                 __mrfi_DST_ADDR_OFS__
#define MRFI_BACKOFF_PERIOD_USECS           __mrfi_BACKOFF_PERIOD_USECS__

#define MRFI_RANDOM_OFFSET                   67
#define MRFI_RANDOM_MULTIPLIER              109
#define MRFI_MIN_SMPL_FRAME_SIZE            (MRFI_HEADER_SIZE + NWK_HDR_SIZE)

/* rx metrics definitions, known as appended "packet status bytes" in datasheet parlance */
#define MRFI_RX_METRICS_CRC_OK_MASK         __mrfi_RX_METRICS_CRC_OK_MASK__
#define MRFI_RX_METRICS_LQI_MASK            __mrfi_RX_METRICS_LQI_MASK__

/* ---------- Radio Abstraction ---------- */
#if (defined MRFI_SI4438)
#define MRFI_RADIO_PARTNUM          0x4438
#define MRFI_RADIO_MIN_VERSION      0

#elif (defined MRFI_SI4463)
#define MRFI_RADIO_PARTNUM          0x4463
#define MRFI_RADIO_MIN_VERSION      0

#else
#error "ERROR: Missing or unrecognized radio."
#endif


/* Max time we can be in a critical section within the delay function.
 * This could be fine-tuned by observing the overhead is calling the bsp delay
 * function. The overhead should be very small compared to this value.
 * Note that the max value for this must be less than 19 usec with the
 * default CLKCON.TICKSPD and CLKCON.CLOCKSPD settings and external 26 MHz
 * crystal as a clock source (which we use).
 *
 * Be careful of direct calls to Mrfi_DelayUsec().
 */
#define MRFI_MAX_DELAY_US 16 /* usec */

  /* The SW timer is calibrated by adjusting the call to the microsecond delay
   * routine. This allows maximum calibration control with repects to the longer
   * times requested by applicationsd and decouples internal from external calls
   * to the microsecond routine which can be calibrated independently.
   */
#if defined(SW_TIMER)
#define APP_USEC_VALUE    496
#else
#define APP_USEC_VALUE    850
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MRFI_SYNC_PIN_IS_HIGH()                     MRFI_IRQ_PIN_IS_HIGH()
#define MRFI_ENABLE_SYNC_PIN_INT()                  MRFI_ENABLE_IRQ_INT()
#define MRFI_DISABLE_SYNC_PIN_INT()                 MRFI_DISABLE_IRQ_INT()
#define MRFI_SYNC_PIN_INT_IS_ENABLED()              MRFI_IRQ_INT_IS_ENABLED()
#define MRFI_CLEAR_SYNC_PIN_INT_FLAG()              MRFI_CLEAR_IRQ_INT_FLAG()
#define MRFI_SYNC_PIN_INT_FLAG_IS_SET()             MRFI_IRQ_INT_FLAG_IS_SET()
#define MRFI_CONFIG_SYNC_PIN_FALLING_EDGE_INT()     MRFI_CONFIG_IRQ_FALLING_EDGE_INT()
#define Tx_mode     0
#define Rx_mode     1


extern uint16_t RxIrqMask;

uint8_t Lora_RSSI_VALID_WAIT(void);
uint8_t Lora_Rssiv_judge(void);
static void generate_timeinfo(mrfiPacket_t * pPacket);
void TX_func(uint8_t *, uint8_t);
void RE_ON(void);
void SX1276_Parameters_Select( void );
extern void Sx1278_GPIO_Inital(void);
extern PacketParams_t packetParams;
	
/* 
 * PA控制的相关宏定义
 */
#define MRFI_CONFIG_PA_PIN_AS_OUTPUT()  \
{                                       \
  MRFI_ENTX_PIN_AS_OUTPUT();            \
  MRFI_ENRX_PIN_AS_OUTPUT();            \
}

#define MRFI_ENABLE_PA_TX() \
{                           \
  MRFI_ENTX_HIGH();         \
  MRFI_ENRX_LOW();          \
}

#define MRFI_ENABLE_PA_RX() \
{                           \
  MRFI_ENTX_LOW();          \
  MRFI_ENRX_HIGH();         \
}

#define MRFI_DISABLE_PA()   \
{                           \
  MRFI_ENTX_LOW();          \
  MRFI_ENRX_LOW();          \
}

/* ------------------------------------------------------------------------------------------------
 *                                       Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void MRFI_GpioIsr(void); /* this called from mrfi_board.c */
static void Mrfi_SyncPinRxIsr(void);
static void Mrfi_RxModeOn(void);
static void Mrfi_RandomBackoffDelay(void);
static void Mrfi_DelayUsec(uint16_t howLong);
static void Mrfi_DelayUsecSem(uint16_t howLong);
static uint16_t Mrfi_CalculateCrc(uint8_t *, uint8_t );
static uint8_t Mrfi_FillCrc(uint8_t *);
static uint8_t Mrfi_IsCrcError(uint8_t *);
static void Mrfi_WaitForTransmitDone(void);
static uint8_t MRFI_RSSI_VALID_WAIT(void);
extern void  EXIT_PC10_Config(void);
/* ------------------------------------------------------------------------------------------------
 *                                       Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static uint8_t mrfiRadioState  = MRFI_RADIO_STATE_UNKNOWN;
mrfiPacket_t mrfiIncomingPacket;
static uint8_t mrfiRndSeed = 0;
static uint8_t mrfiChannelNum;                //信道号
static volatile uint8_t mrfiFrameTransmissionCount;    //帧发送计数
static volatile uint8_t mrfiFrameReceiveCount;    //帧接收计数

/* reply delay support */
static volatile uint8_t  sKillSem = 0;
static volatile uint8_t  sReplyDelayContext = 0;
static          uint16_t sReplyDelayScalar = 0;
static          uint16_t sBackoffHelper = 0;


/* ------------------------------------------------------------------------------------------------
 *                                       1280 Variables
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                       Global Variables
 * ------------------------------------------------------------------------------------------------
 */
 	
Data_Frm_Time Data_Frm_TimeInfo[NUM_CONNECTIONS];

uint8_t IRQ_INT_FLAG;//zeng add
uint8_t frameHasSendFlag = 0;
/**************************************************************************************************
 * @fn          MRFI_Init
 *
 * @brief       Initialize MRFI.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_Init(void)
{
  bspIState_t s;
	memset(&mrfiIncomingPacket, 0x0, sizeof(mrfiIncomingPacket));
  BSP_ENTER_CRITICAL_SECTION(s);
#ifdef SEGGER_DEBUG1  
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"MRFI INIT.\n"RTT_CTRL_RESET"\n");
#endif 	
	mrfiSpiInit();	
	
  EXIT_PC10_Config();
	
	TIM5_NVIC_Configuration();
	TIM5_Configuration();
	
	delay_ms(500);
	
  SX1280_Init();
	
	MRFI_RxOn();

	mrfiRadioState =  Mrfi_GetCurrentState();
  /* use most random bit of rssi to populate the random seed */
  {
    uint8_t currentRssi = MRFI_Rssi();
    uint8_t i;
    for(i=0; i<16; i++)
    {
       mrfiRndSeed = (mrfiRndSeed << 1) | (currentRssi & 0x01);
    }
  }

  /* Force the seed to be non-zero by setting one bit, just in case... */
  mrfiRndSeed |= 0x0080;


  /* Turn off RF.change RF to ready */
  MRFI_Ready();
	{
    /* processing on the peer + the Tx/Rx time plus more */
    sReplyDelayScalar = 1000;

    /* This helper value is used to scale the backoffs during CCA. At very
     * low data rates we need to backoff longer to prevent continual sampling
     * of valid frames which take longer to send at lower rates. Use the scalar
     * we just calculated divided by 32. With the backoff algorithm backing
     * off up to 16 periods this will result in waiting up to about 1/2 the total
     * scalar value. For high data rates this does not contribute at all. Value
     * is in microseconds.
     */
    sBackoffHelper = MRFI_BACKOFF_PERIOD_USECS + (sReplyDelayScalar>>5)*1000;
  }
	memset(mrfiIncomingPacket.frame, 0x00, sizeof(mrfiIncomingPacket.frame));
  memset(mrfiIncomingPacket.rxMetrics, 0x00, sizeof(mrfiIncomingPacket.rxMetrics));
	
	NVIC_Configuration();
  BSP_ENABLE_INTERRUPTS();
	
	BSP_EXIT_CRITICAL_SECTION(s);
}	


 /**************************************************************************************************
 * @fn          Lora_Transmit
 *
 * @brief       Transmit a packet using Lora.
 **************************************************************************************************
*/
uint8_t MRFI_Transmit(mrfiPacket_t * pPacket, uint8_t txType)
{

	//uint8_t rsp_cmd[8]={0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
  uint8_t ccaRetries;
  uint8_t txBufLen;
  uint8_t returnValue = MRFI_TX_RESULT_SUCCESS;
	uint8_t rssi_vld_flag = 0;
	uint8_t bCastAddr[4] = {0xff,0xff,0xff,0xff};
	
	MRFI_Ready();
	
	
	txBufLen = pPacket->frame[MRFI_LENGTH_FIELD_OFS] + MRFI_LENGTH_FIELD_SIZE;

#ifdef ENABLE_CRC
  txBufLen = Mrfi_FillCrc(pPacket->frame);
#endif
	
	GPIO_SetBits(TX_LED_GROUP,TX_LED_PORT);                             //发送指示灯亮    B10=0;
	
	Flg_RxFinish=0;
	
  generate_timeinfo(pPacket);
	
#ifdef SEGGER_DEBUG
  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"TX DATA"RTT_CTRL_RESET"\n");
	SEGGER_RTT_put_multi_char((uint8_t *)pPacket,txBufLen);
#endif 
	
	if(txType == MRFI_TX_TYPE_FORCED)     //直接发送
	{	
		  TX_func((uint8_t *)pPacket,txBufLen);								
	 }
	
	 else if(txType == MRFI_TX_TYPE_CCA)
   {
		 ccaRetries = MRFI_CCA_RETRIES;
		                              //Rx interrupt, which we don't want in this case.
		 
		 for (;;)
		 {
#if !defined(ENABLE_CAD)	
		  MRFI_RxOn();
#endif
#if defined(ENABLE_CAD)	
			 SX1276_LoRaCAD_Init();
			 SX1276_LoRa_CAD();
#endif			 
			 
//			 rssi_vld_flag = Lora_RSSI_VALID_WAIT();
			 rssi_vld_flag=1;
			 
			 if (rssi_vld_flag)
       {
				 
			   TX_func((uint8_t *)pPacket,txBufLen);								
         
         break;
       }
			 
			 else /* No CCA retries are left, abort */
         {
#ifdef SEGGER_DEBUG1  
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"MRFI CCA failed.\n"RTT_CTRL_RESET"\n");
#endif 

					 
					mrfiRadioState =  Mrfi_GetCurrentState();
				  MRFI_Ready();                      //Entry Standby mode
					 
					if (ccaRetries != 0)
          {
            /* delay for a random number of backoffs */
            Mrfi_RandomBackoffDelay();

            /* decrement CCA retries before loop continues */
            ccaRetries--;
          }
          else /* No CCA retries are left, abort */
          {
            /* set return value for failed transmit and break */
            returnValue = MRFI_TX_RESULT_FAILED;
            break;
          }

         }

		 }
	 } 	 
	 
	 //Lora_Receive(receive_array);
  
	 MRFI_RxOn();
	 
	 return( returnValue );
}


/**************************************************************************************************
 * @fn          MRFI_Receive
 *
 * @brief       Copies last packet received to the location specified.
 *              This function is meant to be called after the ISR informs
 *              higher level code that there is a newly received packet.
 *
 * @param       pPacket - pointer to location of where to copy received packet
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_Receive(mrfiPacket_t * pPacket)
{
  *pPacket = mrfiIncomingPacket;
}



 /**************************************************************************************************
 * @fn          Lora_Receive
 *
 * @brief       Receive a packet using Lora.
 **************************************************************************************************
 */

void MRFI_RxOn(void)
{
	TickTime_t Tick_Time_RX={RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE};
	RadioStatus_t statue;
	bspIState_t s;
  BSP_ENTER_CRITICAL_SECTION(s);

  GPIO_SetBits(RX_LED_GROUP,RX_LED_PORT);                               //接收指示灯亮    B10=0;
	
	Flg_RxFinish=0;

	mrfiRadioState = MRFI_RADIO_STATE_RX;
	
	MRFI_Ready();
	
  Radio.SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );	
	Radio.SetRx(Tick_Time_RX);
	delay_ms(1);
	statue=Radio.GetStatus();
	
  BSP_EXIT_CRITICAL_SECTION(s);
}

/**************************************************************************************************
 * @fn          Mrfi_SyncPinRxIsr
 *
 * @brief       This interrupt is called when the SYNC signal transition from high to low.
 *              The sync signal is routed to the sync pin which is a GPIO pin.  This high-to-low
 *              transition signifies a receive has completed.  The SYNC signal also goes from
 *              high to low when a transmit completes.   This is protected against within the
 *              transmit function by disabling sync pin interrupts until transmit completes.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void Mrfi_SyncPinRxIsr(void)
{
	static uint8_t led_state=1;
  uint8_t frameLen=0;
  uint8_t rxBytes;
	uint8_t rxData_pointer;

  /* We should receive this interrupt only in RX state
   * Should never receive it if RX was turned On only for
   * some internal mrfi processing like - during CCA.
   * Otherwise something is terribly wrong.
   */
//  MRFI_ASSERT( mrfiRadioState == MRFI_RADIO_STATE_RX );
	
//	  Mrfi_ClearAllPacketHandlerFlags();
	
	  Mrfi_GetRxFifoByteNum(&rxBytes,&rxData_pointer);

	/* ------------------------------------------------------------------
   *    FIFO empty?
   *   -------------
   */
  /*
   *  See if the receive FIFIO is empty before attempting to read from it.
   *  It is possible nothing the FIFO is empty even though the interrupt fired.
   *  This can happen if address check is enabled and a non-matching packet is
   *  received.  In that case, the radio automatically removes the packet from
   *  the FIFO.
   */
  if (rxBytes == 0)
  {
    /* receive FIFO is empty - do nothing, skip to end */
  }
	else
  {
	 /* receive FIFO is not empty, continue processing */
   //PRINT_DEBUG_MSG("Process MRFI packet...");  
		
		Radio.ReadBuffer(rxData_pointer,&frameLen,MRFI_LENGTH_FIELD_SIZE);
			
//		if ((rxBytes != (frameLen + MRFI_LENGTH_FIELD_SIZE))||
//       ((frameLen + MRFI_LENGTH_FIELD_SIZE) > MRFI_MAX_FRAME_SIZE) ||
//       (frameLen < MRFI_MIN_SMPL_FRAME_SIZE))  
    if(0)		
		{
			
		}
		else
    {     
      /* bytes-in-FIFO and frame length match up - continue processing */

      /* ------------------------------------------------------------------
       *    Get packet
       *   ------------
       */

      /* clean out buffer to help protect against spurious frames */
      memset(mrfiIncomingPacket.frame, 0x00, sizeof(mrfiIncomingPacket.frame));

      /* set length field */
      mrfiIncomingPacket.frame[MRFI_LENGTH_FIELD_OFS] = frameLen;
			
			/* get packet from FIFO */
		  Radio.ReadBuffer(rxData_pointer+MRFI_FRAME_BODY_OFS,&(mrfiIncomingPacket.frame[MRFI_FRAME_BODY_OFS]),frameLen);

      /* get receive metrics from FIFO */
      mrfiIncomingPacket.rxMetrics[0] = 0x0;
			
#ifdef SEGGER_DEBUG1
      SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"Rx MRFI packet:"RTT_CTRL_RESET"\n");
      SEGGER_RTT_put_multi_char(&(mrfiIncomingPacket.frame[0]), frameLen+1);
#endif   		


      /* determine if CRC failed */
#ifdef ENABLE_CRC
      if(Mrfi_IsCrcError(mrfiIncomingPacket.frame))
      {
        /* CRC failed - do nothing, skip to end */       
       // PRINT_DEBUG_MSG("...CRC failed.\n");
#ifdef SEGGER_DEBUG1    
        SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"...CRC failed."RTT_CTRL_RESET"\n");
#endif     
      }
      else
#endif			
      {

#ifdef SEGGER_DEBUG1  
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"...CRC OK."RTT_CTRL_RESET"\n");
#endif  						
         if (!MRFI_RxAddrIsFiltered(MRFI_P_DST_ADDR(&mrfiIncomingPacket)))   //
         {
            /* ------------------------------------------------------------------
             *    Receive successful
             *   --------------------
             */
            //PRINT_DEBUG_MSG("MIFI Rx OK.\n");
#ifdef SEGGER_DEBUG1  
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"MIFI Rx OK."RTT_CTRL_RESET"\n");
#endif  
            /* Convert the raw RSSI value and do offset compensation for this radio */
           mrfiIncomingPacket.rxMetrics[MRFI_RX_METRICS_RSSI_OFS] = Mrfi_GetCurrentRSSI();       //
        /* Remove the CRC valid bit from the LQI byte */
           mrfiIncomingPacket.rxMetrics[MRFI_RX_METRICS_CRC_LQI_OFS] =
           (mrfiIncomingPacket.rxMetrics[MRFI_RX_METRICS_CRC_LQI_OFS] & MRFI_RX_METRICS_LQI_MASK);

            /* call external, higher level "receive complete" processing routine */
           MRFI_RxCompleteISR();
           LED_TURN_ON_OFF(led_state);
					 led_state=~led_state;
         }						
				}
					
		}
	}
}

/**************************************************************************************************
 * @fn          Mrfi_ReadyMode
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void Mrfi_ReadyMode(void)
{
  /*disable receive interrupts */
  //MRFI_DISABLE_SYNC_PIN_INT();
  EXTI->IMR &= ~(1<<10);
  /* turn off radio,changge to ready state */
  //Mrfi_ChangeToReadyState();
  MRFI_Ready();
  /* flush the receive FIFO of any residual data */
  //Mrfi_ResetRxFifo();
  /* clear receive interrupt */
  //MRFI_CLEAR_SYNC_PIN_INT_FLAG();
  EXTI_ClearITPendingBit(EXTI_Line10);
}


/**************************************************************************************************
 * @fn          MRFI_Ready
 *
 * @brief       Put radio in idle mode (receiver if off).  No harm is done this function is
 *              called when radio is already idle.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_Ready(void)
{
  bspIState_t s;
  /* radio must be awake to move it to idle mode */
  //MRFI_ASSERT( mrfiRadioState != MRFI_RADIO_STATE_OFF );
 
  BSP_ENTER_CRITICAL_SECTION(s);
  
  /* if radio is on, turn it off */
  if(mrfiRadioState != MRFI_RADIO_STATE_STDBY_RC)
  {
    SX1280_Standby();
    mrfiRadioState = MRFI_RADIO_STATE_STDBY_RC;
  }
  BSP_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          MRFI_Sleep
 *
 * @brief       Request radio go to sleep.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_Sleep(void)
{
  /* Critical section necessary for watertight testing and
   * setting of state variables.
   */
  bspIState_t intState;
  BSP_ENTER_CRITICAL_SECTION(intState);

  /* If radio is not asleep, put it to sleep */
  if(mrfiRadioState==MRFI_RADIO_STATE_STDBY_RC |\
		 mrfiRadioState==MRFI_RADIO_STATE_STDBY_XOSC|\
	   mrfiRadioState==MRFI_RADIO_STATE_FS |\
	   mrfiRadioState==MRFI_RADIO_STATE_RX |\
	   mrfiRadioState==MRFI_RADIO_STATE_TX )
  {
    /* go to idle so radio is in a known state before sleeping */
    MRFI_Ready();

   // mrfiSpiCmdStrobe( SPWD );
    /* Our new state is OFF */
    mrfiRadioState = MRFI_RADIO_STATE_STDBY_RC;
  }

  BSP_EXIT_CRITICAL_SECTION(intState);
}


/**************************************************************************************************
 * @fn          MRFI_WakeUp
 *
 * @brief       Wake up radio from sleep state.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_WakeUp(void)
{
//  uint8_t  abApi_Write[16];
  /* if radio is already awake, just ignore wakeup request */
  if(mrfiRadioState != MRFI_RADIO_STATE_OFF)
  {
    return;
  }
  /* turn off radio,changge to ready state.enter idle mode */
//  mrfiRadioState = MRFI_RADIO_STATE_IDLE;   
//  abApi_Write[0] = CMD_CHANGE_STATE;        // Use change state command
//  abApi_Write[1] = 3;               // Go to Ready mode
//  bApi_SendCommand(2,abApi_Write);        // Send command to the radio IC
//  vApi_WaitforCTS();                // Wait for CTS
}


/**************************************************************************************************
 * @fn          MRFI_GpioIsr
 *
 * @brief       Interrupt Service Routine for handling GPIO interrupts.  The sync pin interrupt
 *              comes in through GPIO.  This function is designed to be compatible with "ganged"
 *              interrupts.  If the GPIO interrupt services more than just a single pin (very
 *              common), this function just needs to be called from the higher level interrupt
 *              service routine.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_GpioIsr(void)
{
  /* see if sync pin interrupt is enabled and has fired *///
  if ((MRFI_SYNC_PIN_INT_IS_ENABLED()) && MRFI_SYNC_PIN_INT_FLAG_IS_SET())
  {
    /* 统计帧的接收次数 */
    mrfiFrameReceiveCount++;
    
    /*  clear the sync pin interrupt, run sync pin ISR */

    /*
     *  NOTE!  The following macro clears the interrupt flag but it also *must*
     *  reset the interrupt capture.  In other words, if a second interrupt
     *  occurs after the flag is cleared it must be processed, i.e. this interrupt
     *  exits then immediately starts again.  Most microcontrollers handle this
     *  naturally but it must be verified for every target.
     */
    MRFI_CLEAR_SYNC_PIN_INT_FLAG();
    Mrfi_SyncPinRxIsr();
  }
}

/**************************************************************************************************
 * @fn          MRFI_Rssi
 *
 * @brief       Returns "live" RSSI value
 *
 * @param       none
 *
 * @return      RSSI value in units of dBm.
 **************************************************************************************************
 */
int8_t MRFI_Rssi(void)
{
  uint8_t regValue;
  
  /* Radio must be in RX state to measure rssi. */
  //MRFI_ASSERT( mrfiRadioState == MRFI_RADIO_STATE_RX );
  MRFI_RxOn();

  /* Read the RSSI value */
  regValue = Mrfi_GetCurrentRSSI();
  
  /* convert and do offset compensation */
  return(regValue);
}


/**************************************************************************************************
 * @fn          MRFI_RandomByte
 *
 * @brief       Returns a random byte. This is a pseudo-random number generator.
 *              The generated sequence will repeat every 256 values.
 *              The sequence itself depends on the initial seed value.
 *
 * @param       none
 *
 * @return      a random byte
 **************************************************************************************************
 */
uint8_t MRFI_RandomByte(void)
{
  mrfiRndSeed = (mrfiRndSeed*MRFI_RANDOM_MULTIPLIER) + MRFI_RANDOM_OFFSET;

  return mrfiRndSeed;
}

/**************************************************************************************************
 * @fn          Mrfi_RandomBackoffDelay
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void Mrfi_RandomBackoffDelay(void)
{
  uint8_t backoffs;
  uint8_t i;

  /* calculate random value for backoffs - 1 to 16 */
  backoffs = (MRFI_RandomByte() & 0x0F) + 17;

  /* delay for randomly computed number of backoff periods */
  for (i=0; i<backoffs; i++)
  {
    Mrfi_DelayUsec( sBackoffHelper );
  }
}

/****************************************************************************************************
 * @fn          Mrfi_DelayUsec
 *
 * @brief       Execute a delay loop using HW timer. The macro actually used to do the delay
 *              is not thread-safe. This routine makes the delay execution thread-safe by breaking
 *              up the requested delay up into small chunks and executing each chunk as a critical
 *              section. The chunk size is choosen to be the smallest value used by MRFI. The delay
 *              is only approximate because of the overhead computations. It errs on the side of
 *              being too long.
 *
 * input parameters
 * @param   howLong - number of microseconds to delay
 *
 * @return      none
 ****************************************************************************************************
 */
static void Mrfi_DelayUsec(uint16_t howLong)
{
  bspIState_t s;
  uint16_t count = howLong/MRFI_MAX_DELAY_US;

  if (howLong)
  {
    do
    {
      BSP_ENTER_CRITICAL_SECTION(s);
      BSP_DELAY_USECS(MRFI_MAX_DELAY_US);
      BSP_EXIT_CRITICAL_SECTION(s);
    } while (count--);
  }

  return;
}

/****************************************************************************************************
 * @fn          Mrfi_DelayUsecSem
 *
 * @brief       Execute a delay loop using a HW timer. See comments for Mrfi_DelayUsec().
 *              Delay specified number of microseconds checking semaphore for
 *              early-out. Run in a separate thread when the reply delay is
 *              invoked. Cleaner then trying to make MRFI_DelayUsec() thread-safe
 *              and reentrant.
 *
 * input parameters
 * @param   howLong - number of microseconds to delay
 *
 * @return      none
 ****************************************************************************************************
 */
static void Mrfi_DelayUsecSem(uint16_t howLong)
{
  bspIState_t s;
  uint16_t count = howLong/MRFI_MAX_DELAY_US;

  if (howLong)
  {
    do
    {
      BSP_ENTER_CRITICAL_SECTION(s);
      BSP_DELAY_USECS(MRFI_MAX_DELAY_US);
      BSP_EXIT_CRITICAL_SECTION(s);
      if (sKillSem)
      {
        break;
      }
    } while (count--);
  }

  return;
}

/**************************************************************************************************
 * @fn          MRFI_DelayMs
 *
 * @brief       Delay the specified number of milliseconds.
 *
 * @param       milliseconds - delay time
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_DelayMs(uint16_t milliseconds)
{
  while (milliseconds)
  {
    Mrfi_DelayUsec( APP_USEC_VALUE );
    milliseconds--;
  }
}

/**************************************************************************************************
 * @fn          MRFI_ReplyDelay
 *
 * @brief       Delay number of milliseconds scaled by data rate. Check semaphore for
 *              early-out. Run in a separate thread when the reply delay is
 *              invoked. Cleaner then trying to make MRFI_DelayMs() thread-safe
 *              and reentrant.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_ReplyDelay()
{
  bspIState_t s;
  uint16_t    milliseconds = sReplyDelayScalar;

  BSP_ENTER_CRITICAL_SECTION(s);
  sReplyDelayContext = 1;
  BSP_EXIT_CRITICAL_SECTION(s);

  while (milliseconds)
  {
    Mrfi_DelayUsecSem( APP_USEC_VALUE );
    if (sKillSem)
    {
      break;
    }
    milliseconds--;
  }

  BSP_ENTER_CRITICAL_SECTION(s);
  sKillSem           = 0;
  sReplyDelayContext = 0;
  BSP_EXIT_CRITICAL_SECTION(s);
}

/**************************************************************************************************
 * @fn          MRFI_PostKillSem
 *
 * @brief       Post to the loop-kill semaphore that will be checked by the iteration loops
 *              that control the delay thread.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_PostKillSem(void)
{

  if (sReplyDelayContext)
  {
    sKillSem = 1;
  }

  return;
}

/**************************************************************************************************
 * @fn          MRFI_GetRadioState
 *
 * @brief       Returns the current radio state.
 *
 * @param       none
 *
 * @return      radio state - off/idle/rx
 **************************************************************************************************
 */
uint8_t MRFI_GetRadioState(void)
{
  return mrfiRadioState;
}

/**************************************************************************************************
 * @fn          Mrfi_CalculateCrc
 *
 * @brief       Calculate CRC.
 *              Bit pattern used (1)1000 0000 0000 0101, X16+X15+X2+1
 *
 * @param       frame       - 要计算字节数组
 *              frame_len   - 要计算字节数组长度
 *
 * @return      CRC值
 **************************************************************************************************
 */
static uint16_t Mrfi_CalculateCrc(uint8_t *frame, uint8_t frame_len)
{
	uint8_t i,j;
	uint16_t wCrc = 0xffff;
    
	for(j = 0; j < frame_len; j++)
	{
		wCrc = wCrc ^ ((frame[j] & 0x00ff)<<8);

		for (i = 8; i != 0; i--)
		{
			if (wCrc & 0x8000)
			{
				wCrc = (wCrc << 1) ^ 0x8005;
			}
			else
			{
				wCrc <<= 1;
			}
		}
	}
	
  return wCrc;
}

/**************************************************************************************************
 * @fn          Mrfi_FillCrc
 *
 * @brief       在帧中填充CRC值,帧长度增加2字节 
 *
 * @param       frame -指向帧的字节数组
 *
 * @return     帧长，包括长度域
 **************************************************************************************************
 */
static uint8_t Mrfi_FillCrc(uint8_t *frame)
{
  uint8_t frameLen;
  uint16_t crcValue;
  
  /* 长度域存放帧长度,增加2字节用于存放CRC计算值 */
  frame[MRFI_LENGTH_FIELD_OFS] += 2;

  /* 获取帧长，括包长度域 */ 
  frameLen = frame[MRFI_LENGTH_FIELD_OFS] + MRFI_LENGTH_FIELD_SIZE;
  
  /* 计算CRC值, 最后2字节不参与计算 */
  crcValue = Mrfi_CalculateCrc(frame, frameLen - 2);
    
  /* 将CRC值写入最后2字节中,高字节在前 */
  frame[frameLen - 2] = ((crcValue & 0xff00)>>8);
  frame[frameLen - 1] = (crcValue & 0x00ff);
  
  return frameLen;
}

/**************************************************************************************************
 * @fn          Mrfi_IsCrcError
 *
 * @brief       检查帧中的CRC是否错误,帧长度减少2字节
 *
 * @param       frame - 指向帧的字节数组
 *
 * @return      0   - CRC正确
 *              非0 - CRC错误
 **************************************************************************************************
 */
static uint8_t Mrfi_IsCrcError(uint8_t *frame)
{
  uint8_t frameLen;
  uint16_t crcValue;
  
  /* 获取帧长，括包长度域 */ 
  frameLen = frame[MRFI_LENGTH_FIELD_OFS] + MRFI_LENGTH_FIELD_SIZE;
  
  /* 计算CRC值, 最后2字节(CRC值)也参与计算 */
  crcValue = Mrfi_CalculateCrc(frame, frameLen);
	
  /* 长度域存放帧长度,减少2字节以去除CRC计算值 */
  frame[MRFI_LENGTH_FIELD_OFS] -= 2;
  
  return crcValue;
}

/**************************************************************************************************
 * @fn          Mrfi_WaitForTransmitDone
 *
 * @brief       等待发送完成. 
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void Mrfi_WaitForTransmitDone(void)
{
  uint16_t i;
  
  for(i = 0; i < 10000; i++)
  {
    MRFI_DelayMs(1);
    
    if(GPIO_ReadInputDataBit(LORA_DIO0_GROUP,LORA_DIO0_PORT))
    {
      /* 统计帧的发送次数 */
	    Radio.ClearIrqStatus(0xFFFF);	                                      //Clear irq
	    Radio.SetStandby( STDBY_RC );                                           //Entry Standby mode  
		  GPIO_SetBits(TX_LED_GROUP,TX_LED_PORT);			
      mrfiFrameTransmissionCount++;
      break;
    }
  }
}

/**************************************************************************************************
 * @fn          MRFI_getFrameTransmissionCount
 *
 * @brief       获取射频芯片的帧发送计数.
 *
 * @param       none
 *
 * @return      帧收发计数
 **************************************************************************************************
 */
uint8_t MRFI_getFrameTransmissionCount(void)
{
  return mrfiFrameTransmissionCount;
}

/**************************************************************************************************
 * @fn          MRFI_getFrameReceiveCount
 *
 * @brief       获取射频芯片的帧接收计数.
 *
 * @param       none
 *
 * @return      帧收发计数
 **************************************************************************************************
 */
uint8_t MRFI_getFrameReceiveCount(void)
{
  return mrfiFrameReceiveCount;
}

/**************************************************************************************************
 * @fn          MRFI_SetChannelNum
 *
 * @brief       设置信道号变量.在下次Tx或Rx之后,射频芯片的信道才会实际改变.
 *
 * @param       num - 信道号
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_SetChannelNum(uint8_t num)
{
  mrfiChannelNum = num;
	MRFI_RxOn();
}

/**************************************************************************************************
 * @fn          MRFI_GetChannelNum
 *
 * @brief       获取信道号变量.
 *
 * @param       none
 *
 * @return      信道号
 **************************************************************************************************
 */
uint8_t MRFI_GetChannelNum(void)
{
  return mrfiChannelNum;
}

/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */


#define MRFI_RADIO_TX_FIFO_SIZE     256  /* from datasheet */

/* verify largest possible packet fits within FIFO buffer */
//#if ((MRFI_MAX_FRAME_SIZE + MRFI_RX_METRICS_SIZE) > MRFI_RADIO_TX_FIFO_SIZE)
#if (MRFI_MAX_FRAME_SIZE > MRFI_RADIO_TX_FIFO_SIZE)   //Si4463无MRFI_RX_METRICS_SIZE
#error "ERROR:  Maximum possible packet length exceeds FIFO buffer.  Decrease value of maximum application payload."
#endif

/**************************************************************************************************
*/


void SX1276_Parameters_Select(void)
{

		/****BW ??*********/
		//gb_BW=1;//10.4K  BW
		//gb_BW=2;//15.6K  BW 
		//gb_BW=3;//20.8K  BW 
		//gb_BW=4;//31.2K  BW 
		//gb_BW=5;//41.7K  BW 
		//gb_BW=6;//62.5K  BW 
		//gb_BW=7;//125KHz BW 
		gb_BW=9;//500KHz BW 
		/****BW ????*********/

		/*******SF ??*********/
		//gb_SF=0;// SF=6;  64 chips / symbol
		//gb_SF=1;// SF=7;  128 chips / symbol
		gb_SF=2;// SF=8;  256 chips / symbol
		//gb_SF=3;// SF=9;  9  512 chips / symbol
		//gb_SF=4;////SF=10; 1024 chips / symbol
		//gb_SF=5;////SF=11; 2048 chips / symbol
		//gb_SF=6;//SF=12;  4096 chips / symbol
		/****SF ????*********/

		/*******CR ??*********/
		CR=1;////  Error coding rate=4/5
		//CR=2;////  Error coding rate=4/6
		//CR=3;////  Error coding rate=4/7
		//CR=4;////  Error coding rate=4/8
		/*******CR ????*********/

}

void TX_func(uint8_t * pPacket,uint8_t TxBufLen)
{
	bspIState_t s;	
	EXTI->IMR &= ~(1<<10);	
  BSP_ENTER_CRITICAL_SECTION(s);
	
	
	SX1280_ParaCofig(TxBufLen);
	
	MRFI_Ready();
	
  Radio.SetPacketParams( &packetParams );
	
	SX1280_SendPacket(pPacket,TxBufLen);
  GPIO_ResetBits(TX_LED_GROUP,TX_LED_PORT);	
	Mrfi_WaitForTransmitDone();
	
  BSP_EXIT_CRITICAL_SECTION(s);	
  EXTI->IMR |= (1<<10); 
}						


uint8_t Lora_RSSI_VALID_WAIT(void)
{ 
	uint8_t rand_num;
  int16_t delay = MRFI_RSSI_VALID_DELAY_US;
  do
  {
    if(!Lora_Rssiv_judge())
    {
      break;
    }
		
#if defined(ENABLE_CAD)			
		SX1276_LoRa_CAD();
#endif
    //delay_ms(2);
	   rand_num=rand();
	   rand_num=(rand_num%10)*3+(rand())%10+5;
		 MRFI_DelayMs(rand_num);	
		//delay_us(64);
    //Mrfi_DelayUsec(64); /* sleep */
    delay -= rand_num;                 //delay -= 64;
  }while(delay > 0);
  return (delay>0)?1:0;
}

uint8_t Lora_Rssiv_judge(void)
{
#if !defined(ENABLE_CAD)	
	 uint8_t RSSI;
#ifdef SEGGER_DEBUG
	uint8_t * Rssi_Pointer;
	Rssi_Pointer=&RSSI;
#endif
   rssi_v = SPIRead((u8)(LR_RegRssiValue>>8));
	 RSSI=RSSI_LF_MODE-rssi_v;
	
#ifdef SEGGER_DEBUG
  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"RSSI VALUE"RTT_CTRL_RESET"\n");
	SEGGER_RTT_put_multi_char(Rssi_Pointer,1);
#endif 
	 if(RSSI>=90&RSSI<=RSSI_LF_MODE)
	 {
	   return 0;
	 }
	 else
	 {
	   return 1;
	 }
#endif 

	 
#ifdef ENABLE_CAD	 
   uint8_t channel_clear_state;
	 uint8_t Irq_flags;
	 uint8_t cad_done_flag;
	 cad_done_flag=((SPIRead((u8)(LR_RegIrqFlags>>8))&0x04));
   while(cad_done_flag==0)
   {
	   cad_done_flag=((SPIRead((u8)(LR_RegIrqFlags>>8))&0x04));
	 }

	 if(cad_done_flag==0x04)                     //CAD检测完成
   {
	    Irq_flags=SPIRead((u8)(LR_RegIrqFlags>>8)); 
		  channel_clear_state=(Irq_flags&0x01);
		 	SX1276_LoRaClearIrq();                                        //Clear irq
	 }
	 if(channel_clear_state==0)
   {
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"CHANNEL IS CLEAR"RTT_CTRL_RESET"\n");
#endif 
	   return 0;
	 }
	 else
   {
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"CHANNEL IS NOT CLEAR"RTT_CTRL_RESET"\n");
#endif 
	   return 1;		 
	 }
#endif 

}

void generate_timeinfo(mrfiPacket_t * pPacket)
{
	uint16_t test_arry[1];
	uint8_t i;
	
  test_arry[0]=TIM_GetCounter(TIM6);
	
	for(i=0;i<NUM_CONNECTIONS;i++)
  {
    if(Data_Frm_TimeInfo[i].Ed_Addr[0]==pPacket->frame[1]&&\
			 Data_Frm_TimeInfo[i].Ed_Addr[1]==pPacket->frame[2]&&\
		   Data_Frm_TimeInfo[i].Ed_Addr[2]==pPacket->frame[3]&&\
		   Data_Frm_TimeInfo[i].Ed_Addr[3]==pPacket->frame[4]
			)
		{
		  Data_Frm_TimeInfo[i].Tx_Time_info[0]=(test_arry[0]>>8);
			Data_Frm_TimeInfo[i].Tx_Time_info[1]=test_arry[0];
			break;
		}
	}
	if(i>=NUM_CONNECTIONS)
  {
	  for(i=0;i<NUM_CONNECTIONS;i++)
   {
    if(Data_Frm_TimeInfo[i].Ed_Addr[0]==0x00&&Data_Frm_TimeInfo[i].Ed_Addr[1]==0x00&&\
		   Data_Frm_TimeInfo[i].Ed_Addr[2]==0x00&&Data_Frm_TimeInfo[i].Ed_Addr[3]==0x00
			)
		  {
        Data_Frm_TimeInfo[i].Ed_Addr[0]=pPacket->frame[1];
			  Data_Frm_TimeInfo[i].Ed_Addr[1]=pPacket->frame[2];
		    Data_Frm_TimeInfo[i].Ed_Addr[2]=pPacket->frame[3];
		    Data_Frm_TimeInfo[i].Ed_Addr[3]=pPacket->frame[4];
		    Data_Frm_TimeInfo[i].Tx_Time_info[0]=(test_arry[0]>>8);
		  	Data_Frm_TimeInfo[i].Tx_Time_info[1]=test_arry[0];
				break;
		  }
	  }
	}
}

