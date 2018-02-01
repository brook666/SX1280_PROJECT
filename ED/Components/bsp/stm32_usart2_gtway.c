// History:     2014-12-24  Leidi ���º���get_hex_uart2()ΪGet_hex_uart2()���Խ��ʹ���µ�WiFiģ��ʱ�Ĵ��ڽ��չ��ش���.

#include "stm32_usart2_gtway.h"
#include "delay.h"
#include "bsp.h"



/***************************************************************************
 * @fn          Usart2_Configuration
 *     
 * @brief       ���ô��ڵĻ�������
 *     
 * @data        2015��08��05��
 *     
 * @param       BaudRate - ������
 *              NewState - ENABLE/DISABLE
 *              
 * @return      void
 ***************************************************************************
 */ 
void Usart2_Configuration(uint32_t BaudRate,FunctionalState NewState)
{
  USART_InitTypeDef USART_InitStructure;        //����һ�����ڽṹ��
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
	
  
  USART_InitStructure.USART_BaudRate            =BaudRate ;         //������115200
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;  //���������ʹ��8λ����
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;   //��֡��β����1λֹͣλ
  USART_InitStructure.USART_Parity              = USART_Parity_No ;   //��żʧ��
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ����ʧ��
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx; //���պͷ���ģʽ
  USART_Init(USART2, &USART_InitStructure);               //���ݲ�����ʼ�����ڼĴ���
  USART_ITConfig(USART2,USART_IT_RXNE,NewState);              //ʹ�ܴ����жϽ���

  __nop(); __nop();
  USART_Cmd(USART2, NewState);                          //ʹ�ܴ�������
}


/***************************************************************************
 * @fn          USART2_NVIC_Configuration
 *     
 * @brief       ���ô���2�жϡ�
 *              USART2�ж���Ϊ��3�飬��ռ���ȼ�1����Ӧ���ȼ�0
 *
 * @data        2015��08��05��
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
 * @fn          Usart1_Configuration
 *     
 * @brief       ���ô��ڵĻ�������
 *     
 * @data        2015��08��05��
 *     
 * @param       BaudRate - ������
 *              NewState - ENABLE/DISABLE
 *              
 * @return      void
 ***************************************************************************
 */ 
void Usart1_Configuration(uint32_t BaudRate,FunctionalState NewState)
{
  USART_InitTypeDef USART_InitStructure;        //����һ�����ڽṹ��
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* config USART1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
  
  /* USART1 GPIO config */
  /* Configure USART1 Tx (PA.9) as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);    

  /* Configure USART1 Rx (PA.10) as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  USART_DeInit(USART1);
	
  
  USART_InitStructure.USART_BaudRate            =BaudRate ;         //������115200
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;  //���������ʹ��8λ����
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;   //��֡��β����1λֹͣλ
  USART_InitStructure.USART_Parity              = USART_Parity_No ;   //��żʧ��
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ����ʧ��
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx; //���պͷ���ģʽ
  USART_Init(USART1, &USART_InitStructure);               //���ݲ�����ʼ�����ڼĴ���
  USART_ITConfig(USART1,USART_IT_RXNE,NewState);              //ʹ�ܴ����жϽ���

  __nop(); __nop();
  USART_Cmd(USART1, NewState);                          //ʹ�ܴ�������
}

/***************************************************************************
 * @fn          USART1_NVIC_Configuration
 *     
 * @brief       ���ô���1�жϡ�
 *              USART1�ж���Ϊ��3�飬��ռ���ȼ�1����Ӧ���ȼ�0
 *
 * @data        2015��08��05��
 *     
 * @param       NewState - ENABLE/DISABLE
 *     
 * @return      void
 ***************************************************************************
 */
void USART1_NVIC_Configuration(FunctionalState NewState)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_SetVectorTable (NVIC_VectTab_FLASH, NVIC_VECTTAB_FLASH_OFFSET);  
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
  NVIC_Init(&NVIC_InitStructure);
}
/***************************************************************************
 * @fn          put_string_uart2
 *     
 * @brief       �����ַ���
 *     
 * @data        2015��08��05��
 *     
 * @param       ptr - �����ַ���
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_string_uart2( uint8_t *ptr)
{
  while(*ptr != '\0')
  {
    while (!(USART2->SR & USART_FLAG_TXE));    // TX������У�                  
    USART_SendData(USART2, *ptr++);  // ��������
  }
     
}

/***************************************************************************
 * @fn          put_char_uart2
 *     
 * @brief       ����һ���ַ�
 *     
 * @data        2015��08��05��
 *     
 * @param       send_data - �����ַ�
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
 * @brief       ���Ͷ���ַ�
 *     
 * @data        2015��08��05��
 *     
 * @param       pMultiChar - ����������
 *              len        - ���������ݵĳ���
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
 * @fn          put_multi_hex_uart2
 *     
 * @brief       ���Ͷ��16������
 *     
 * @data        2015��08��05��
 *     
 * @param       pSendData - �����͵�һ��16������
 *              len       - ���ݳ���
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_multi_hex_uart2(uint8_t *pSendData, uint16_t len)
{
  uint16_t i; 
  	
  for(i=0; i<len; i++)
  {
   put_hex_uart2(*pSendData);
   pSendData++;
  }  
}

/***************************************************************************
 * @fn          put_multi_hex_uart1
 *     
 * @brief       ���Ͷ��16������
 *     
 * @data        2015��08��05��
 *     
 * @param       pSendData - �����͵�һ��16������
 *              len       - ���ݳ���
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_multi_hex_uart1(const uint8_t *pSendData, uint8_t len)
{
  uint16_t i; 
  	
  for(i=0; i<len; i++)
  {
   put_hex_uart1(*pSendData);
   pSendData++;
  }  
}

/***************************************************************************
 * @fn          put_hex_uart2
 *     
 * @brief       ����16������
 *     
 * @data        2015��08��05��
 *     
 * @param       send_data - ������16������
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_hex_uart2(uint8_t send_data)
{
   while (!(USART2->SR & USART_FLAG_TXE));   // USART2 TX buffer ready?
   USART_SendData(USART2, send_data);  // ��������
}

/***************************************************************************
 * @fn          put_hex_uart1
 *     
 * @brief       ����16������
 *     
 * @data        2015��08��05��
 *     
 * @param       send_data - ������16������
 *     
 * @return      void
 ***************************************************************************
 */ 
void put_hex_uart1(uint8_t send_data)
{
   while (!(USART1->SR & USART_FLAG_TXE));   // USART1 TX buffer ready?
   USART_SendData(USART1, send_data);  // ��������
}



/**************************************************************************************************
 * @fn          Get_hex_uart2
 *
 * @brief       �Ӵ���2�Ľ��ջ�������ȡ1���ֽ�.
 *
 * @param       rec_data - �Ӵ���2���յ��ĵ����ֽ�
 *
 * @return      0x00  - �Ӵ��ڽ����ֽڳ���
 *              0x01  - �Ӵ��ڽ����ֽ�����
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



///�ض���c�⺯��printf��USART2
int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ�USART1 */
		USART_SendData(USART2, (uint8_t) ch);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);		
	
		return (ch);
}
