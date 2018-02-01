/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   wifi_module.h
// Description: ����WiFiģ��֧���ļ�.
// Author:      Leidi
// Version:     1.0
// Date:        2014-12-24
// History:     2014-12-24  Leidi ��ʼ�汾����.
*****************************************************************************/
#ifndef _WIFI_MODULE_H
#define _WIFI_MODULE_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"

/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
/* WiFiģ��ʹ�õ�GPIO���� */
#define RCC_APB2Periph_GPIO_Wifi_1  RCC_APB2Periph_GPIOB
#define RCC_APB2Periph_GPIO_Wifi_2  RCC_APB2Periph_GPIOC

/* WiFiģ���뵥Ƭ�����ӵ�Port */
#define GPIO_Port_Wifi_1            GPIOB
#define GPIO_Port_Wifi_2            GPIOC

/* WiFiģ���뵥Ƭ�����ӵ�Pin */
#define GPIO_Pin_nLink_Router       GPIO_Pin_3
#define GPIO_Pin_nLink_Server       GPIO_Pin_5
#define GPIO_Pin_nReady             GPIO_Pin_6
#define GPIO_Pin_nReset             GPIO_Pin_2

/* WiFiģ���뵥Ƭ�����ӵĴ��� */
#define USARTx_Wifi                 USART2

/* ------------------------------------------------------------------------------------------------
 *                                            Typedefs
 * ------------------------------------------------------------------------------------------------
 */
/* WiFiģ����Ϣ״̬ö�� */
typedef enum {
  WIFI_INFO_INVALID,
  WIFI_INFO_VALID,
}wifi_info_state_t;

/* WiFiģ����֤��Ϣ */
typedef struct
{
  wifi_info_state_t  info_state;
  char wifi_ssid[30];
  char wifi_key[30];
}wifi_auth_info_t;

/* ------------------------------------------------------------------------------------------------
 *                                            Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void WIFI_Module_Init(void);
void WiFi_Module_Reset(void);
uint8_t WIFI_ready_state_changed(void);
void WIFI_set_usart_by_ready_state(void);
void WIFI_Set_STA_auth_info(char (*ssid_str)[], char (*key_str)[]);
void WIFI_Init_STA(void);

#endif

/**************************************************************************************************
 */
