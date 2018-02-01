// History:     2014-12-24  Leidi 更新函数get_hex_uart2()为Get_hex_uart2()用以解决使用新的WiFi模块时的串口接收过载错误.

#include "stm32_usart2_gtway.h"
#include "delay.h"
#include "bsp.h"



/***************************************************************************
 * @fn          Usart2_Configuration
 *     
 * @brief       配置串口的基本参数
 *     
 * @data        2015年08月05日
 *     
 * @param       BaudRate - 波特率
 *              NewState - ENABLE/DISABLE
 *              
 * @return      void
 ***************************************************************************
 */ 
void Usart2_Configuration(uint32_t BaudRate,FunctionalState NewState)
{
  USART_InitTypeDef USART_InitStructure;        //定义一个串口结构体
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* config USART2 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
  
  /* USART1 GPIO config */
  /* Configure USART1 Tx (PA.2) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);    

  /* Configure USART1 Rx (PA.3) as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  USART_DeInit(USART2);
  
  USART_InitStructure.USART_BaudRate            = BaudRate ;         //波特率115200
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;  //传输过程中使用8位数据
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;   //在帧结尾传输1位停止位
  USART_InitStructure.USART_Parity              = USART_Parity_No ;   //奇偶失能
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件流失能
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx; //接收和发送模式
  USART_Init(USART2, &USART_InitStructure);               //根据参数初始化串口寄存器
  USART_ITConfig(USART2,USART_IT_RXNE,NewState);              //使能串口中断接收
  __nop(); __nop();
  USART_Cmd(USART2, NewState);                          //使能串口外设
}


/***************************************************************************
 * @fn          USART2_NVIC_Configuration
 *     
 * @brief       配置串口2中断。
 *              USART2中断组为第3组，抢占优先级1，响应优先级0
 *
 * @data        2015年08月05日
 *     
 * @param       NewState - ENABLE/DISABLE
 *     
 * @return      void
 ***************************************************************************
 */
void USART2_NVIC_Configuration(FunctionalState NewState)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_SetVectorTable (NVIC_VectTab_FLASH, NVIC_VECTTAB_FLASH_OFFSET);  
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
  NVIC_Init(&NVIC_InitStructure);
}

/***************************************************************************
 * @fn          put_string_uart2
 *     
 * @brief       发送字符串
 *     
 * @data        2015年08月05日
 *     
 * @param       ptr - 输入字符串
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_string_uart2( uint8_t *ptr)
{
  while(*ptr != '\0')
  {
    while (!(USART2->SR & USART_FLAG_TXE));    // TX缓存空闲？                  
    USART_SendData(USART2, *ptr++);  // 发送数据
  }
     
}

/***************************************************************************
 * @fn          put_char_uart2
 *     
 * @brief       发送一个字符
 *     
 * @data        2015年08月05日
 *     
 * @param       send_data - 输入字符
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_char_uart2(uint8_t send_data)
{
  unsigned char  str[4];
  if((send_data%16) <10)
    str[1] = send_data%16 + '0';
  else
    str[1] = send_data%16-10 + 'a';
  
  if((send_data/16) <10)
    str[0] = send_data/16 + '0';
  else
    str[0] = send_data/16-10 + 'a';
  
  str[2] = ' ';
  str[3] = '\0';
  put_string_uart2(str);   
}

/***************************************************************************
 * @fn          put_multi_char_uart2
 *     
 * @brief       发送多个字符
 *     
 * @data        2015年08月05日
 *     
 * @param       pMultiChar - 待发送数据
 *              len        - 待发送数据的长度
 *     
 * @return      
 ***************************************************************************
 */ 
void put_multi_char_uart2(uint8_t* pMultiChar, uint16_t len)
{
  uint16_t i;
  for( i=0; i<len; i++)
  {
    put_char_uart2(*pMultiChar);
    pMultiChar++;
  }
}

/***************************************************************************
 * @fn          put_hex_uart2
 *     
 * @brief       发送16进制数
 *     
 * @data        2015年08月05日
 *     
 * @param       send_data - 待发送16进制数
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_hex_uart2(uint8_t send_data)
{
   while (!(USART2->SR & USART_FLAG_TXE));   // USART2 TX buffer ready?
   USART_SendData(USART2, send_data);  // 发送数据
}

/***************************************************************************
 * @fn          put_multi_hex_uart2
 *     
 * @brief       发送多个16进制数
 *     
 * @data        2015年08月05日
 *     
 * @param       pSendData - 待发送的一组16进制数
 *              len       - 数据长度
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_multi_hex_uart2(uint8_t *pSendData, uint16_t len)
{

  uint16_t i; 
	
#ifdef SEGGER_DEBUG 
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"USART2 SEND:"RTT_CTRL_RESET"\n");        
  SEGGER_RTT_put_multi_char(pSendData,len);
#endif 
	
  for(i=0; i<len; i++)
  {
   put_hex_uart2(*pSendData);
   pSendData++;
  }  

}

/***************************************************************************
 * @fn          put_multi_hex_uart1
 *     
 * @brief       发送多个16进制数
 *     
 * @data        2015年08月05日
 *     
 * @param       pSendData - 待发送的一组16进制数
 *              len       - 数据长度
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_multi_hex_uart1( uint8_t *pSendData, uint16_t len)
{
  uint16_t i; 
  	
  for(i=0; i<len; i++)
  {
   put_hex_uart1(*pSendData);
   pSendData++;
  }  
}

/***************************************************************************
 * @fn          put_hex_uart1
 *     
 * @brief       发送16进制数
 *     
 * @data        2015年08月05日
 *     
 * @param       send_data - 待发送16进制数
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_hex_uart1(uint8_t send_data)
{
   while (!(USART1->SR & USART_FLAG_TXE));   // USART1 TX buffer ready?
   USART_SendData(USART1, send_data);  // 发送数据
}


/**************************************************************************************************
 * @fn          Get_hex_uart2
 *
 * @brief       从串口2的接收缓冲区读取1个字节.
 *
 * @param       rec_data - 从串口2接收到的单个字节
 *
 * @return      0x00  - 从串口接收字节出错
 *              0x01  - 从串口接收字节正常
 **************************************************************************************************
 */
//uint8_t Get_hex_uart2(uint8_t *rec_data)
//{
//  uint8_t ret=0;
//  
//  if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
//  {
//    USART_ClearFlag(USART2, USART_FLAG_ORE);
//    *rec_data = USART_ReceiveData(USART2);
//    ret = 0x00;
//  }

//  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
//  {   
//    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
//    *rec_data = USART_ReceiveData(USART2);

//    ret = 0x01;
//  }
//  
//  return ret;
//}



///重定向c库函数printf到USART2
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到USART1 */
		USART_SendData(USART2, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);		
	
		return (ch);
}
