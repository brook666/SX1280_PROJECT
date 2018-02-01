
/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *   BSP (Board Support Package)
 *   Target : Texas Instruments MSP-EXP430FG4618
 *            "MSP430FG4618/F2013 Experimenter Board"
 *   Board configuration file.
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H
#include "smpl_config.h"  //zeng add 

/**************************************************************************************************
 *                                       Configuration                                            *
 **************************************************************************************************
 */
 
//#define HANGXUN_TEST
//#define DEBUG
/*
 *  Supported clock speeds : 1, 2, 4, 6, 8, 10, 12, and 16 MHz.
 *
 *  NOTE!  The clock speeds are approximate as they are derived from an internal
 *         digitally controlled oscillator.
 */
#if defined (SYSCLK_FREQ_HSE)
    #define BSP_CONFIG_CLOCK_MHZ_SELECT     8  /* approximate MHz */
#elif defined (SYSCLK_FREQ_72MHz)
    #define BSP_CONFIG_CLOCK_MHZ_SELECT     72  /* approximate MHz */
#else
    #error "select right clock!"  
#endif

/*
 *  Custom clock configuration is available.  Provide values for FLLDx, N, and FN_x as
 *  shown in the example below.  The clock speed itself must be provided too.  Providing
 *  a custom configuration overrides the above selection.
 *
 *  #define BSP_CONFIG_CUSTOM_CLOCK_MHZ         1.9988
 *  #define BSP_CONFIG_MSP430_CUSTOM_FLLDx      0
 *  #define BSP_CONFIG_MSP430_CUSTOM_N          60
 *  #define BSP_CONFIG_MSP430_CUSTOM_FN_x       0
 */


/* ------------------------------------------------------------------------------------------------
 *                                Exported Clock Configuration 
 * ------------------------------------------------------------------------------------------------
 */

/*
 *   DCO is adjusted via three values:
 *      N      - multiplier value for DCO (see register SCFQCTL)
 *      FLLDx  - additional multiplier for DCO (see register SCFI0)
 *      FN_x   -  range control for DCO (see register SCFI0)
 *  
 *   Clock frequency is derived with following formula:
 *      freq = 2^FLLDx * (N + 1) * faclk;   where faclk = 32.768kHz (X2 crystal from board)
 *  
 *   Clock frequency must fall within range specifed via FN_x:
 *      0 for 0.65 - 6.1 MHz
 *      1 for 1.3 - 12.1 MHz
 *      2 for 2.0 - 17.9 MHz
 *      4 for 2.8 - 26.6 MHz
 *      8 for 4.2 - 46 MHz
 */

#if (defined BSP_CONFIG_CUSTOM_CLOCK_MHZ)    || \
    (defined BSP_CONFIG_MSP430_CUSTOM_FLLDx) || \
    (defined BSP_CONFIG_MSP430_CUSTOM_N)     || \
    (defined BSP_CONFIG_MSP430_CUSTOM_FN_x)
#define BSP_CONFIG_CLOCK_MHZ        BSP_CONFIG_CUSTOM_CLOCK_MHZ
#define BSP_CONFIG_MSP430_FLLDx     BSP_CONFIG_MSP430_CUSTOM_FLLDx
#define BSP_CONFIG_MSP430_N         BSP_CONFIG_MSP430_CUSTOM_N
#define BSP_CONFIG_MSP430_FN_x      BSP_CONFIG_MSP430_CUSTOM_FN_x
#else

#if (BSP_CONFIG_CLOCK_MHZ_SELECT == 1)
#define BSP_CONFIG_CLOCK_MHZ      1.0158
#define BSP_CONFIG_MSP430_FLLDx   0
#define BSP_CONFIG_MSP430_N       30
#define BSP_CONFIG_MSP430_FN_x    0
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 2)
#define BSP_CONFIG_CLOCK_MHZ      1.9988
#define BSP_CONFIG_MSP430_FLLDx   0
#define BSP_CONFIG_MSP430_N       60
#define BSP_CONFIG_MSP430_FN_x    0
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 4)
#define BSP_CONFIG_CLOCK_MHZ      3.9977
#define BSP_CONFIG_MSP430_FLLDx   0
#define BSP_CONFIG_MSP430_N       121
#define BSP_CONFIG_MSP430_FN_x    0
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 6)
#define BSP_CONFIG_CLOCK_MHZ      6.0293
#define BSP_CONFIG_MSP430_FLLDx   1
#define BSP_CONFIG_MSP430_N       91
#define BSP_CONFIG_MSP430_FN_x    0
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 8)
#define BSP_CONFIG_CLOCK_MHZ      7.9954
#define BSP_CONFIG_MSP430_FLLDx   1
#define BSP_CONFIG_MSP430_N       121
#define BSP_CONFIG_MSP430_FN_x    2
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 10)
#define BSP_CONFIG_CLOCK_MHZ      9.9615
#define BSP_CONFIG_MSP430_FLLDx   2
#define BSP_CONFIG_MSP430_N       75
#define BSP_CONFIG_MSP430_FN_x    2
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 12)
#define BSP_CONFIG_CLOCK_MHZ      12.0586
#define BSP_CONFIG_MSP430_FLLDx   2
#define BSP_CONFIG_MSP430_N       91
#define BSP_CONFIG_MSP430_FN_x    4

#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 16)
#define BSP_CONFIG_CLOCK_MHZ      15.9908
#define BSP_CONFIG_MSP430_FLLDx   2
#define BSP_CONFIG_MSP430_N       121
#define BSP_CONFIG_MSP430_FN_x    4

#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 72)	 //AP1Ê±ÖÓÎª36Mhz
#define BSP_CONFIG_CLOCK_MHZ      72
#define BSP_CONFIG_MSP430_FLLDx   2
#define BSP_CONFIG_MSP430_N       121
#define BSP_CONFIG_MSP430_FN_x    4
#else
#error "ERROR: Unsupported clock speed.  Custom clock speeds are possible.  See comments in code."
#endif

#endif


/**************************************************************************************************
 */
#endif
