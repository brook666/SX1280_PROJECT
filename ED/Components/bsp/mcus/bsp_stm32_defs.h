/**************************************************************************************************
  Filename:       bsp_msp430_defs.h
  Revised:        $Date: 2009-10-11 18:52:46 -0700 (Sun, 11 Oct 2009) $
  Revision:       $Revision: 20897 $

**************************************************************************************************/

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *   BSP (Board Support Package)
 *   MCU : STM32
 *   Microcontroller definition file.
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

#ifndef BSP_STM32_DEFS_H
#define BSP_STM32_DEFS_H

/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */
#define BSP_MCU_STM32

/* ------------------------------------------------------------------------------------------------
 *                                     Compiler Abstraction
 * ------------------------------------------------------------------------------------------------
 */

#include "stm32f10x.h" 
#include "core_cm3.h"

typedef uint8_t   __istate_t;

#define __bsp_ISTATE_T__            __istate_t   //UNSIGNED CHAR
#define __bsp_ISR_FUNCTION__(f,v)   __bsp_QUOTED_PRAGMA__(vector=v) __interrupt void f(void); \
                                    __bsp_QUOTED_PRAGMA__(vector=v) __interrupt void f(void)

#define __bsp_ENABLE_INTERRUPTS__()       st(__enable_fault_irq(); )
#define __bsp_DISABLE_INTERRUPTS__()      st(__disable_fault_irq();)
#define __bsp_INTERRUPTS_ARE_ENABLED__()  ()


/* ------------------------------------------------------------------------------------------------
 *                                          Common
 * ------------------------------------------------------------------------------------------------
 */
#define __bsp_LITTLE_ENDIAN__   1
#define __bsp_CODE_MEMSPACE__   /* blank */
#define __bsp_XDATA_MEMSPACE__  /* blank */


#ifndef NULL
#define NULL 0
#endif

/**************************************************************************************************
 */
#endif
