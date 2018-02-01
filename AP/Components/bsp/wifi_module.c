/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   wifi_module.c
// Description: 自用WiFi模块支持文件.
// Author:      Leidi
// Version:     1.1
// Date:        2014-12-24
// History:     2014-12-24  Leidi 初始版本建立.
//              2015-05-25  Leidi 新增函数WIFI_ready_state_changed().
//                                新增函数WIFI_set_usart_by_ready_state().
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
 * @brief       WiFi模块初始化.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void WIFI_Module_Init(void)
{
  /* 初始化WiFi模块使用的串口，禁止串口接收中断 */
  usart_wifi_init(DISABLE);
  
  /* 相关GPIO配置 */
  wifi_GPIO_config();
  
  /* 重启WiFi模块 */
//  reset_wifi_module();
  
  /* 等待WiFi模块启动完成 */
  wait_wifi_ready(GPIO_Pin_nReady);
}

/**************************************************************************************************
 * @fn          WiFi_Module_Reset
 *
 * @brief       WiFi模块初始化.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void WiFi_Module_Reset(void)
{
  /* 初始化WiFi模块使用的串口，禁止串口接收中断 */
  usart_wifi_init(DISABLE);
  
  /* 重启WiFi模块 */
  reset_wifi_module();
  
}
/**************************************************************************************************
 * @fn          wifi_GPIO_config
 *
 * @brief       配置STM32与WiFi模块相连的引脚：
 *              GPIO_Pin_nLink_Router  - 输入模式，低电平有效，表示WiFi模块连接上路由器
 *              GPIO_Pin_nLink_Server  - 输入模式，低电平有效，表示WiFi模块连接上服务器
 *              GPIO_Pin_nReady        - 输入模式，低电平有效，表示WiFi模块启动完成
 *              GPIO_Pin_nReset        - 输出模式，低电平有效，用于复位WiFi模块
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void wifi_GPIO_config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 开启GPIO的外设时钟 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_Wifi_1 | RCC_APB2Periph_GPIO_Wifi_2, ENABLE);
  
  /* 配置GPIO_Pin_nLink_Server和GPIO_Pin_nReady */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_nLink_Server | GPIO_Pin_nReady;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                 //上拉输入
  GPIO_Init(GPIO_Port_Wifi_1, &GPIO_InitStructure);
  
  /* 配置GPIO_Pin_nLink_Router */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_nLink_Router;  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;                 //上拉输入
  GPIO_Init(GPIO_Port_Wifi_2, &GPIO_InitStructure);
  
  /* 配置GPIO_Pin_nReset */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_nReset;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;              //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;              //最大输出速度2MHz
  GPIO_Init(GPIO_Port_Wifi_2, &GPIO_InitStructure);
  
  /* GPIO_Pin_Reset默认应输出高电平防止WiFi模块意外复位 */
  GPIO_WriteBit(GPIO_Port_Wifi_2, GPIO_Pin_nReset, Bit_SET);
}

/**************************************************************************************************
 * @fn          reset_wifi_module
 *
 * @brief       重启WiFi模块.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void reset_wifi_module(void)
{
  /* 开始重启WiFi模块 */
  GPIO_WriteBit(GPIO_Port_Wifi_2, GPIO_Pin_nReset, Bit_RESET);
  
  delay_ms(10);
  
  /* GPIO_Pin_nReset默认应输出高电平防止WiFi模块意外重启 */
  GPIO_WriteBit(GPIO_Port_Wifi_2, GPIO_Pin_nReset, Bit_SET);
}

/**************************************************************************************************
 * @fn          wait_wifi_ready
 *
 * @brief       检测WiFi模块输出的引脚信号，等待相应的信号到来。等待期间闪烁LED，等待结束后LED常亮。
 *
 * @param       GPIO_Pinx - 要检测的WiFi模块信号引脚
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
 * @brief       初始化WiFi模块连接的串口。
 *
 * @param       NewState - 串口接收中断的开启或关闭
 *
 * @return      none
 **************************************************************************************************
 */
static void usart_wifi_init(FunctionalState NewState)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* 使能 USART2 和相应的 GPIO 和复用功能时钟 */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

  /* USART2 使用的 GPIO 初始化：PA.2(Tx) */
  GPIO_InitStructure.GPIO_Pin                   = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode                  = GPIO_Mode_AF_PP;      //复用推挽输出
  GPIO_InitStructure.GPIO_Speed                 = GPIO_Speed_10MHz;     //最大输出速度10MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);    

  /* USART2 使用的 GPIO 初始化：PA.3(Rx) */
  GPIO_InitStructure.GPIO_Pin                   = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode                  = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* 清除串口参数 */
  USART_DeInit(USARTx_Wifi);
  
  /* 初始化串口参数 */
  USART_InitStructure.USART_BaudRate            = 115200;               //固定波特率115200
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;  //传输过程中使用8位数据
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;     //在帧结尾传输1位停止位
  USART_InitStructure.USART_Parity              = USART_Parity_No ;     //奇偶失能
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件流失能
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx; //接收和发送模式
  USART_Init(USARTx_Wifi, &USART_InitStructure);
  
  /* 设置串口中断 */
  USART_ITConfig(USARTx_Wifi, USART_IT_RXNE, NewState);
  
  /* 使能串口 */
  USART_Cmd(USARTx_Wifi, NewState);
}

/**************************************************************************************************
 * @fn          WIFI_ready_state_changed
 *
 * @brief       检测WiFi模块的nReady引脚的电平是否变化.
 *
 * @param       none
 *
 * @return      0 - nReady引脚的电平与上一次检测相比无变化
 *              1 - nReady引脚的电平与上一次检测相比有变化
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
 * @brief       根据nReady引脚电平设置WiFi模块串口的中断状态.在nReady为高电平时关闭串口中断，在nReady为
 *              低电平时设开启串口中断.
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
    /* 关闭串口中断 */
    USART_ITConfig(USARTx_Wifi, USART_IT_RXNE, DISABLE);
    BSP_TURN_OFF_LED1();
  }
  else
  {
    /* 设开启串口中断 */
    USART_ITConfig(USARTx_Wifi, USART_IT_RXNE, ENABLE);
    BSP_TURN_OFF_LED1();
  }
}

/**************************************************************************************************
*/
