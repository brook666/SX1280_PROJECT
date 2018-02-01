/**************************************************************************************************
  Filename:       bsp_board.c
  Revised:        $Date: 2009-10-11 16:48:20 -0700 (Sun, 11 Oct 2009) $
  Revision:       $Revision: 20896 $

**************************************************************************************************/

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *   BSP (Board Support Package)
 *   Target : Texas Instruments MSP-EXP430FG4618
 *            "MSP430FG4618/F2013 Experimenter Board"
 *   Top-level board code file.
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"
#include "bsp_config.h"
#include "delay.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#if defined (SYSCLK_FREQ_HSE)
    #define BSP_TIMER_CLK_MHZ     BSP_CONFIG_CLOCK_MHZ_SELECT  /* approximate MHz */
#elif defined (SYSCLK_FREQ_72MHz)
    #define BSP_TIMER_CLK_MHZ     (BSP_CONFIG_CLOCK_MHZ_SELECT)  /* approximate MHz */
#else
    #error "select right clock!"  
#endif

/* ------------------------------------------------------------------------------------------------
 *                                            Local Variables
 * ------------------------------------------------------------------------------------------------
 */
#if defined(SW_TIMER)
static uint8_t sIterationsPerUsec = 0;
#endif

/**************************************************************************************************
 * @fn          BSP_EARLY_INIT
 *
 * @brief       This function is called by start-up code before doing the normal initialization
 *              of data segments. If the return value is zero, initialization is not performed.
 *              The global macro label "BSP_EARLY_INIT" gets #defined in the bsp_msp430_defs.h
 *              file, according to the specific compiler environment (CCE or IAR). In the CCE
 *              environment this macro invokes "_system_pre_init()" and in the IAR environment
 *              this macro invokes "__low_level_init()".
 *
 * @param       None
 *
 * @return      0 - don't intialize data segments / 1 - do initialization
 **************************************************************************************************
*/
void RCC_Configuration(void)
{
  /* Setup the microcontroller system. Initialize the Embedded Flash Interface,
     initialize the PLL and update the SystemFrequency variable. */
  SystemInit();
}


/**************************************************************************************************
 * @fn          BSP_InitBoard
 *
 * @brief       Initialize the board.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void BSP_InitBoard(void)
{
//  TIM_TimeBaseInitTypeDef  TIM_BaseInitStructure;     //定义一个定时器结构体变量
  RCC_Configuration();
/*******************************************/
  /* Configure Timer3 for use by the delay function */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);    //使能定时器3
  TIM_DeInit( TIM3);                                  //将TIM3定时器初始化位复位值
  TIM_InternalClockConfig(TIM3);              //配置 TIM3 内部时钟 
  TIM_PrescalerConfig(TIM3, 71,TIM_PSCReloadMode_Immediate);
  TIM_ARRPreloadConfig(TIM3, DISABLE);          //禁止ARR 预装载缓冲器

#if defined(SW_TIMER)
#define MHZ_CLOCKS_PER_USEC      BSP_CLOCK_MHZ
#define MHZ_CLOCKS_PER_ITERATION 10

  sIterationsPerUsec = (uint8_t)(((MHZ_CLOCKS_PER_USEC)/(MHZ_CLOCKS_PER_ITERATION))+.5);

  if (!sIterationsPerUsec)
  {
    sIterationsPerUsec = 1;
  }
#endif   /* SW_TIMER */
}

/**************************************************************************************************
 * @fn          BSP_Delay
 *
 * @brief       Sleep for the requested amount of time.
 *
 * @param       # of microseconds to sleep.
 *
 * @return      none
 **************************************************************************************************
 */
void BSP_Delay(uint16_t usec)
#if !defined(SW_TIMER)
{ 
  volatile uint8_t i=0;  
  TIM_SetCounter(TIM3,0);                         
//  TIM_SetCompare3(TIM3,BSP_TIMER_CLK_MHZ*usec);
  TIM_SetCompare3(TIM3,usec);
  TIM_Cmd(TIM3, ENABLE);
//  while(!(TIM_GetFlagStatus(TIM3,TIM_FLAG_CC3 )));
  while(!(TIM_GetFlagStatus(TIM3,TIM_FLAG_CC3 )))
  {
    i++;
    if(i == usec*3)
    {
      i = 0;
      break;
    }
  }
  TIM_Cmd(TIM3, DISABLE);
  TIM_ClearFlag(TIM3,TIM_FLAG_CC3);
#ifdef IWDG_START 
		IWDG_ReloadCounter();  
#endif
}
#else   /* !SW_TIMER */
{
  
  /* Declared 'volatile' in case User optimizes for speed. This will
   * prevent the optimizer from eliminating the loop completely. But
   * it also generates more code...
   */
  volatile uint16_t repeatCount = (sIterationsPerUsec*usec)/2;

  while (repeatCount--) ;

  return;
}

#endif  /* !SW_TIMER */
/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
#if (!defined BSP_CONFIG_MSP430_N) || \
    (BSP_CONFIG_MSP430_N == 0) || (BSP_CONFIG_MSP430_N > 127)
#error "ERROR: Missing or illegal value for N (see register SCFQCTL)."
#endif

#if (!defined BSP_CONFIG_MSP430_FLLDx) || (BSP_CONFIG_MSP430_FLLDx > 3)
#error "ERROR: Missing or illegal value for FLLDx (see register SCFI0)."
#endif

#if (!defined BSP_CONFIG_MSP430_FN_x) || (BSP_CONFIG_MSP430_FN_x > 15)
#error "ERROR: Missing or illegal value for FLLDx (see register SCFI0)."
#endif


/**************************************************************************************************
*/

