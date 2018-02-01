/**************************************************************************************************
  Revised:        $Date: 2007-07-06 11:19:00 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

**************************************************************************************************/

/* ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 *   MRFI (Minimal RF Interface)
 *   Board code file.
 *   Target : Texas Instruments MSP-EXP430FG4618
 *            "MSP430FG4618/F2013 Experimenter Board"
 *   Radios : CC1100, CC1101, CC2500
 * ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"
#include "radio.h"

/* ------------------------------------------------------------------------------------------------
 *                                      External Prototypes
 * ------------------------------------------------------------------------------------------------
 */
 
extern void MRFI_GpioIsr(void);
//zeng add
extern uint8_t IRQ_INT_FLAG;
//zeng add end
extern void Mrfi_RXConfi(void);
/**************************************************************************************************
 * @fn          MRFI_GpioPort1Isr
 *
 * @brief       -
 *
 * @param       -
 *
 * @return      -
 **************************************************************************************************
 */

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line10) != RESET) //ȷ���Ƿ������EXTI Line�ж�
  {
		
	 uint16_t irq_statue;	
		
	 irq_statue=Radio.GetIrqStatus();

	 if(irq_statue==IRQ_RX_DONE)	 
	 {
	   IRQ_INT_FLAG=1;	
		 Radio.ClearIrqStatus(IRQ_RX_DONE);		 
	   MRFI_GpioIsr();
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"RXDONE Interrupt Happened"RTT_CTRL_RESET"\n");
#endif		 
	 }
	 else
   {
		 IRQ_INT_FLAG=1;	
		 Radio.ClearIrqStatus(IRQ_RADIO_ALL);			 
	   MRFI_CLEAR_SYNC_PIN_INT_FLAG();
	 }

	 irq_statue=0;	

	 MRFI_RxOn();
	 
  }  
}
/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */



/**************************************************************************************************
 */


