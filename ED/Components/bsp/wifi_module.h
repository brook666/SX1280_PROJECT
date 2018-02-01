/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   wifi_module.h
// Description: 自用WiFi模块支持文件.
// Author:      Leidi
// Version:     1.0
// Date:        2014-12-24
// History:     2014-12-24  Leidi 初始版本建立.
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
/* WiFi模块使用的GPIO总线 */
#define RCC_APB2Periph_GPIO_Wifi_1  RCC_APB2Periph_GPIOB
#define RCC_APB2Periph_GPIO_Wifi_2  RCC_APB2Periph_GPIOC

/* WiFi模块与单片机连接的Port */
#define GPIO_Port_Wifi_1            GPIOB
#define GPIO_Port_Wifi_2            GPIOC

/* WiFi模块与单片机连接的Pin */
#define GPIO_Pin_nLink_Router       GPIO_Pin_3
#define GPIO_Pin_nLink_Server       GPIO_Pin_5
#define GPIO_Pin_nReady             GPIO_Pin_6
#define GPIO_Pin_nReset             GPIO_Pin_2

/* WiFi模块与单片机连接的串口 */
#define USARTx_Wifi                 USART2

/* ------------------------------------------------------------------------------------------------
 *                                            Typedefs
 * ------------------------------------------------------------------------------------------------
 */
/* WiFi模块信息状态枚举 */
typedef enum {
  WIFI_INFO_INVALID,
  WIFI_INFO_VALID,
}wifi_info_state_t;

/* WiFi模块认证信息 */
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
