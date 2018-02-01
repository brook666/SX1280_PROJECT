/**************************************************************************************************
  Revised:        $Date: 2007-07-06 11:19:00 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

**************************************************************************************************/

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *   BSP (Board Support Package)
 *   Generic LED driver code file.
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp_leds.h"
#include "mrfi.h"
#include "radio_config_Sx1280.h"
/* ------------------------------------------------------------------------------------------------
 *                       Macros
 * @fn          BSP_InitLeds
 *
 * @brief       Initialize LED hardware and driver.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
 
 void Sx1280_GPIO_Inital(void)
{
   GPIO_InitTypeDef GPIO_InitStructureA;       //output
   GPIO_InitTypeDef GPIO_InitStructureBI;       //input
	 GPIO_InitTypeDef GPIO_InitStructureBO;       //ouput
	 GPIO_InitTypeDef GPIO_InitStructureC;       //intput
	  
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);     
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	
	
	 GPIO_InitStructureA.GPIO_Pin = RADIO_nRESET_PIN;                   //NREST   OUTPUT
	 GPIO_InitStructureA.GPIO_Mode =  GPIO_Mode_Out_PP;
	 GPIO_InitStructureA.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOA, &GPIO_InitStructureA);
	
	 GPIO_InitStructureBO.GPIO_Pin = LED_TX_PIN;                         //LED   OUTPUT
	 GPIO_InitStructureBO.GPIO_Mode =  GPIO_Mode_Out_PP;
	 GPIO_InitStructureBO.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOB, &GPIO_InitStructureBO);
	
	 GPIO_InitStructureBI.GPIO_Pin = RADIO_BUSY_PIN;                     //BUSY  INPUT
	 GPIO_InitStructureBI.GPIO_Mode =  GPIO_Mode_IPD;
	 GPIO_InitStructureBI.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOB, &GPIO_InitStructureBI);
	
	 GPIO_InitStructureC.GPIO_Pin = RADIO_DIOx_PIN;                       //DIO1  INPUT
	 GPIO_InitStructureC.GPIO_Mode =  GPIO_Mode_IPD;
	 GPIO_InitStructureC.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(GPIOC, &GPIO_InitStructureC);
}

/**************************************************************************************************
*/

void LED_TURN_ON_OFF(uint8_t led_state)
{
  if(led_state==1)
  {
		GPIO_ResetBits(RX_LED_GROUP,RX_LED_PORT);
	}
	else
  {
		GPIO_SetBits(RX_LED_GROUP,RX_LED_PORT);
	}
}	
   
