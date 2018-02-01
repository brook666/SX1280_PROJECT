/**************************************************************************************************
  Revised:        $Date: 2007-07-06 11:19:00 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

**************************************************************************************************/

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *   BSP (Board Support Package)
 *   Include file for core BSP services.
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

#ifndef BSP_H
#define BSP_H


/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp_board_defs.h"
#include "bsp_macros.h"
//zeng add 
#include "bsp_stm32_defs.h" 
#include "stm32f10x.h"
#include "core_cm3.h"
#include "stm32_usart2_gtway.h"
//#include "gtway_config.h"
#include "smpl_nwk_config.h"
#include "smpl_config.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "stm32_flash.h"
#include "stm32_beep.h"
#include "stm32_reset.h"
#include "stm32_rtc.h"
#include "SEGGER_RTT.h"
#include "flash_start_addr.h"
//zeng add end
/* ------------------------------------------------------------------------------------------------
 *                                          BSP Defines
 * ------------------------------------------------------------------------------------------------
 */
#define BSP
#define BSP_VER       100  /* BSP version 1.00a */
#define BSP_SUBVER    a

//#define DEBUG
//#define DEBUG_0
//#define PA_EN
#define COMP_AP_ADDR
#define COMP_JOIN_TOKEN
#define CRC_TEST
//#define MANNUL_FREQ_AGILITY
//#define ED_ACTIVE_FREQ_HOP_TEST   //广播信道帧时注释掉
///////////////////////////////////////////

/* ------------------------------------------------------------------------------------------------
 *                                            Clock
 * ------------------------------------------------------------------------------------------------
 */
#define BSP_CLOCK_MHZ   __bsp_CLOCK_MHZ__


/* ------------------------------------------------------------------------------------------------
 *                                            Memory
 * ------------------------------------------------------------------------------------------------
 */
#ifndef __bsp_LITTLE_ENDIAN__
#error ERROR: Endianess not defined
#endif

#define BSP_LITTLE_ENDIAN   __bsp_LITTLE_ENDIAN__

#define CODE    __bsp_CODE_MEMSPACE__
#define XDATA   __bsp_XDATA_MEMSPACE__

/* ------------------------------------------------------------------------------------------------
 *                                            Interrupts
 * ---------------------------------------------------------------------------------------------
 */


#define BSP_ISR_FUNCTION(func,vect)     __bsp_ISR_FUNCTION__(func,vect)

#define BSP_ENABLE_INTERRUPTS()         __bsp_ENABLE_INTERRUPTS__()
#define BSP_DISABLE_INTERRUPTS()        __bsp_DISABLE_INTERRUPTS__()
#define BSP_INTERRUPTS_ARE_ENABLED()    __bsp_INTERRUPTS_ARE_ENABLED__()


/* ------------------------------------------------------------------------------------------------
 *                                         Critical Sections
 * ------------------------------------------------------------------------------------------------
 */
typedef __bsp_ISTATE_T__  bspIState_t;


#define BSP_ENTER_CRITICAL_SECTION(x)    st(x = OS_CPU_SR_Save();)
#define BSP_EXIT_CRITICAL_SECTION(x)     OS_CPU_SR_Restore(x)
#define BSP_CRITICAL_STATEMENT(x)       st( bspIState_t s;                   \
                                            BSP_ENTER_CRITICAL_SECTION();    \
                                            x;                                \
                                            BSP_EXIT_CRITICAL_SECTION(); )



/* ------------------------------------------------------------------------------------------------
 *                                           Asserts
 * ------------------------------------------------------------------------------------------------
 */

/*
 *  BSP_ASSERT( expression ) - The given expression must evaluate as "true" or else the assert
 *  handler is called.  From here, the call stack feature of the debugger can pinpoint where
 * the problem occurred.
 *
 *  BSP_FORCE_ASSERT() - If asserts are in use, immediately calls the assert handler.
 *
 *  BSP_ASSERTS_ARE_ON - can use #ifdef to see if asserts are enabled
 *
 *  Asserts can be disabled for optimum performance and minimum code size (ideal for
 *  finalized, debugged production code).
 */

#if (!defined BSP_NO_DEBUG)
#ifndef BSP_ASSERT_HANDLER
#define BSP_ASSERT_HANDLER()      st( __disable_fiq();  while(1); )
#endif
#define BSP_ASSERT(expr)          st( if (!(expr)) BSP_ASSERT_HANDLER(); )
#define BSP_FORCE_ASSERT()        BSP_ASSERT_HANDLER()
#define BSP_ASSERTS_ARE_ON
#else
#define BSP_ASSERT(expr)          /* empty */
#define BSP_FORCE_ASSERT()        /* empty */
#endif

/* static assert */
#define BSP_STATIC_ASSERT(expr)   void bspDummyPrototype( char dummy[1/((expr)!=0)] )


/* ------------------------------------------------------------------------------------------------
 *                                           Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void BSP_Init(void);
//void TIM1_NVIC_Configuration(void);
//void TIM1_Configuration(FunctionalState NewState);
void TIM6_NVIC_Configuration(FunctionalState NewState);
void TIM6_Configuration(FunctionalState NewState);
void NVIC_Configuration(void);
void TIM5_NVIC_Configuration(void);
void TIM5_Configuration(void);
//void BoardConfig(char cmd);
/**************************************************************************************************
 */

/****************************************************************************************
 *                                 BEGIN ENDIAN SUPPORT
 *
 * Security encrypt/decrypt operates on unsigned long quantities. These must match on
 * source and destination platforms. These macros enforce the standard conversions.
 * Currently all platforms (CC2520/CC2x30 and MSP430) are little endian.
 *
 *******************   Network order for encryption is LITTLE ENDIAN   ******************
 *
 ****************************************************************************************/

#if (BSP_LITTLE_ENDIAN != 0)
#define   swsn_ntohs(x)    (x)
#define   swsn_htons(x)    (x)

#define   swsn_ntohl(x)    (x)
#define   swsn_htonl(x)    (x)

#else

#define   swsn_ntohs(x)    (((x>>8) & 0xFF) | ((x & 0xFF)<<8))
#define   swsn_htons(x)    (((x>>8) & 0xFF) | ((x & 0xFF)<<8))

#define   swsn_ntohl(x)    ( ((x>>24) & 0xFF) | ((x>>8) & 0xFF00) | \
                        ((x & 0xFF00)<<8) | ((x & 0xFF)<<24)   \
                      )
#define   swsn_htonl(x)    ( ((x>>24) & 0xFF) | ((x>>8) & 0xFF00) | \
                        ((x & 0xFF00)<<8) | ((x & 0xFF)<<24)   \
                      )



#endif  /* (BSP_LITTLE_ENDIAN != 0) */

/***************************************************************************************
 *                                 END ENDIAN SUPPORT
 ***************************************************************************************/



#endif
