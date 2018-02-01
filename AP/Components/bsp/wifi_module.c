/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   wifi_module.c
// Description: ����WiFiģ��֧���ļ�.
// Author:      Leidi
// Version:     1.1
// Date:        2014-12-24
// History:     2014-12-24  Leidi ��ʼ�汾����.
//              2015-05-25  Leidi ��������WIFI_ready_state_changed().
//                                ��������WIFI_set_usart_by_ready_state().
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "wifi_module.h"
#include "bsp_leds.h"
#include "delay.h"
#include "smpl_config.h"
#include <string.h>

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void wifi_GPIO_config(void);
static void reset_wifi_module(void);
static void wait_wifi_ready(uint16_t GPIO_Pinx);
static void usart_wifi_init(FunctionalState NewState);

/**************************************************************************************************
 * @fn          WiFi_Module_Init
 *
 * @brief       WiFiģ���ʼ��.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void WIFI_Module_Init(void)
{
  /* ��ʼ��WiFiģ��ʹ�õĴ��ڣ���ֹ���ڽ����ж� */
  usart_wifi_init(DISABLE);
  
  /* ���GPIO���� */
  wifi_GPIO_config();
  
  /* ����WiFiģ�� */
//  reset_wifi_module();
  
  /* �ȴ�WiFiģ��������� */
  wait_wifi_ready(GPIO_Pin_nReady);
}

/**************************************************************************************************
 * @fn          WiFi_Module_Reset
 *
 * @brief       WiFiģ���ʼ��.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void WiFi_Module_Reset(void)
{
  /* ��ʼ��WiFiģ��ʹ�õĴ��ڣ���ֹ���ڽ����ж� */
  usart_wifi_init(DISABLE);
  
  /* ����WiFiģ�� */
  reset_wifi_module();
  
}
/**************************************************************************************************
 * @fn          wifi_GPIO_config
 *
 * @brief       ����STM32��WiFiģ�����������ţ�
 *              GPIO_Pin_nLink_Router  - ����ģʽ���͵�ƽ��Ч����ʾWiFiģ��������·����
 *              GPIO_Pin_nLink_Server  - ����ģʽ���͵�ƽ��Ч����ʾWiFiģ�������Ϸ�����
 *              GPIO_Pin_nReady        - ����ģʽ���͵�ƽ��Ч����ʾWiFiģ���������
 *              GPIO_Pin_nReset        - ���ģʽ���͵�ƽ��Ч�����ڸ�λWiFiģ��
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void wifi_GPIO_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* ����GPIO������ʱ�� */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_Wifi_1 | RCC_APB2Periph_GPIO_Wifi_2, ENABLE);
  
  /* ����GPIO_Pin_nLink_Server��GPIO_Pin_nReady */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_nLink_Server | GPIO_Pin_nReady;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                 //��������
  GPIO_Init(GPIO_Port_Wifi_1, &GPIO_InitStructure);
  
  /* ����GPIO_Pin_nLink_Router */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_nLink_Router;  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                 //��������
  GPIO_Init(GPIO_Port_Wifi_2, &GPIO_InitStructure);
  
  /* ����GPIO_Pin_nReset */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_nReset;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;              //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;              //�������ٶ�2MHz
  GPIO_Init(GPIO_Port_Wifi_2, &GPIO_InitStructure);
  
  /* GPIO_Pin_ResetĬ��Ӧ����ߵ�ƽ��ֹWiFiģ�����⸴λ */
  GPIO_WriteBit(GPIO_Port_Wifi_2, GPIO_Pin_nReset, Bit_SET);
}

/**************************************************************************************************
 * @fn          reset_wifi_module
 *
 * @brief       ����WiFiģ��.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void reset_wifi_module(void)
{
  /* ��ʼ����WiFiģ�� */
  GPIO_WriteBit(GPIO_Port_Wifi_2, GPIO_Pin_nReset, Bit_RESET);
  
  delay_ms(10);
  
  /* GPIO_Pin_nResetĬ��Ӧ����ߵ�ƽ��ֹWiFiģ���������� */
  GPIO_WriteBit(GPIO_Port_Wifi_2, GPIO_Pin_nReset, Bit_SET);
}

/**************************************************************************************************
 * @fn          wait_wifi_ready
 *
 * @brief       ���WiFiģ������������źţ��ȴ���Ӧ���źŵ������ȴ��ڼ���˸LED���ȴ�������LED������
 *
 * @param       GPIO_Pinx - Ҫ����WiFiģ���ź�����
 *
 * @return      none
 **************************************************************************************************
 */
static void wait_wifi_ready(uint16_t GPIO_Pinx)
{
	volatile static uint8_t cnt_ready_time_500ms = 0 ;
  if(GPIO_Pin_nLink_Server == GPIO_Pinx)
  {
    while(Bit_RESET != GPIO_ReadInputDataBit(GPIO_Port_Wifi_1, GPIO_Pin_nLink_Server))
    {
      delay_ms(500);
			cnt_ready_time_500ms++;
			if(cnt_ready_time_500ms == 120)
			{
				cnt_ready_time_500ms = 0;
				break;
			}
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif
      BSP_TOGGLE_LED4();
    }
    BSP_TURN_ON_LED4();        
  }
  else if(GPIO_Pin_nLink_Router == GPIO_Pinx)
  {
    while(Bit_RESET != GPIO_ReadInputDataBit(GPIO_Port_Wifi_2, GPIO_Pin_nLink_Router))
    {
      delay_ms(500);
			cnt_ready_time_500ms++;
			if(cnt_ready_time_500ms == 120)
			{
				cnt_ready_time_500ms = 0;
				break;
			}
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif
      BSP_TOGGLE_LED4();
    }
    BSP_TURN_ON_LED4();
  }
  else if(GPIO_Pin_nReady == GPIO_Pinx)
  {
    while(Bit_RESET != GPIO_ReadInputDataBit(GPIO_Port_Wifi_1, GPIO_Pin_nReady))
    {			
      delay_ms(500);
			cnt_ready_time_500ms++;
			if(cnt_ready_time_500ms == 120)
			{
				cnt_ready_time_500ms = 0;
				break;
			}
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif
      BSP_TOGGLE_LED4();
    }
    BSP_TURN_ON_LED4();
  }
  else
  {
    //Do nothing
  }
}

/**************************************************************************************************
 * @fn          usart_wifi_init
 *
 * @brief       ��ʼ��WiFiģ�����ӵĴ��ڡ�
 *
 * @param       NewState - ���ڽ����жϵĿ�����ر�
 *
 * @return      none
 **************************************************************************************************
 */
static void usart_wifi_init(FunctionalState NewState)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* ʹ�� USART2 ����Ӧ�� GPIO �͸��ù���ʱ�� */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

  /* USART2 ʹ�õ� GPIO ��ʼ����PA.2(Tx) */
  GPIO_InitStructure.GPIO_Pin                   = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode                  = GPIO_Mode_AF_PP;      //�����������
  GPIO_InitStructure.GPIO_Speed                 = GPIO_Speed_10MHz;     //�������ٶ�10MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);    

  /* USART2 ʹ�õ� GPIO ��ʼ����PA.3(Rx) */
  GPIO_InitStructure.GPIO_Pin                   = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode                  = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* ������ڲ��� */
  USART_DeInit(USARTx_Wifi);
  
  /* ��ʼ�����ڲ��� */
  USART_InitStructure.USART_BaudRate            = 115200;               //�̶�������115200
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;  //���������ʹ��8λ����
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;     //��֡��β����1λֹͣλ
  USART_InitStructure.USART_Parity              = USART_Parity_No ;     //��żʧ��
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ����ʧ��
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx; //���պͷ���ģʽ
  USART_Init(USARTx_Wifi, &USART_InitStructure);
  
  /* ���ô����ж� */
  USART_ITConfig(USARTx_Wifi, USART_IT_RXNE, NewState);
  
  /* ʹ�ܴ��� */
  USART_Cmd(USARTx_Wifi, NewState);
}

/**************************************************************************************************
 * @fn          WIFI_ready_state_changed
 *
 * @brief       ���WiFiģ���nReady���ŵĵ�ƽ�Ƿ�仯.
 *
 * @param       none
 *
 * @return      0 - nReady���ŵĵ�ƽ����һ�μ������ޱ仯
 *              1 - nReady���ŵĵ�ƽ����һ�μ������б仯
 **************************************************************************************************
 */
uint8_t WIFI_ready_state_changed(void)
{
  static uint8_t lastPinState = 0;
  uint8_t currentPinState;
  uint8_t ret = 0;
  
  currentPinState = GPIO_ReadInputDataBit(GPIO_Port_Wifi_1, GPIO_Pin_nReady);
  if(lastPinState != currentPinState)
  {
    lastPinState = currentPinState;
    ret = 1;
  }
  
  return ret;
}

/**************************************************************************************************
 * @fn          WIFI_set_usart_by_ready_state
 *
 * @brief       ����nReady���ŵ�ƽ����WiFiģ�鴮�ڵ��ж�״̬.��nReadyΪ�ߵ�ƽʱ�رմ����жϣ���nReadyΪ
 *              �͵�ƽʱ�迪�������ж�.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void WIFI_set_usart_by_ready_state(void)
{
  if(Bit_RESET != GPIO_ReadInputDataBit(GPIO_Port_Wifi_1, GPIO_Pin_nReady))
  {
    /* �رմ����ж� */
    USART_ITConfig(USARTx_Wifi, USART_IT_RXNE, DISABLE);
    BSP_TURN_OFF_LED1();
  }
  else
  {
    /* �迪�������ж� */
    USART_ITConfig(USARTx_Wifi, USART_IT_RXNE, ENABLE);
    BSP_TURN_OFF_LED1();
  }
}

/**************************************************************************************************
*/
