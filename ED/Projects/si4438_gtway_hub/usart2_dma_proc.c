#include "usart2_dma_proc.h"
//#include "gtway_frm_proc.h"
#include "uart_frm_proc.h"
#include "stm32f10x_it.h"
#include "ed_frm_proc.h"
#include "net_frm_proc.h"
#include "stdlib.h"

uint16_t  uart2_frm_data_cnt= 0;  //串口2使用
uint16_t  uart1_frm_data_cnt= 0;  //串口1使用
uint8_t   USART2_SEND_DATA[256];
uint8_t   USART1_SEND_DATA[256];
DMA_RecvData_t usart2_recv_data = {0};
DMA_RecvData_t usart1_recv_data = {0};
usart_buffer_t  uart2_frm_rbuf = {0};
usart_buffer_t  uart1_frm_rbuf = {0};


volatile  static uint8_t usart_cur_frm_recv = 1;

void NVIC_ClearPendingFlag(uint8_t NVIC_IRQChannel)
{
	NVIC->ICPR[NVIC_IRQChannel >> 0x05] = (uint32_t)0x01 << (NVIC_IRQChannel & (uint8_t)0x1F);	
}

/***************************************************************************
 * @fn          Usart2_DMA_Configuration
 *     
 * @brief       配置USART1的DMA接收和发送,USART2->TX使用DMA1_Channel6,
 *              USART2->RX使用DMA1_Channel7。
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void Usart2_DMA_Configuration(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  /* DMA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//DMA1
//  DMA_InitTypeDef DMA_InitStructure;
//  /* DMA clock enable */
//  /* DMA1 Channel7 (triggered by USART2 Tx event) Config */
  DMA_DeInit(DMA1_Channel7);  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART2_SEND_DATA;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  //一定不能是DMA_Mode_Circular，否则会一直发送数据
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel7, &DMA_InitStructure);
  USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
  DMA_Cmd(DMA1_Channel7, DISABLE);    
  
  /* DMA1 Channel6 (triggered by USART2 Rx event) Config */
  DMA_DeInit(DMA1_Channel6);  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40004404;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)usart2_recv_data.RecvData;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = USART2_DMA_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel6, &DMA_InitStructure);
  //DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
  //DMA_ITConfig(DMA1_Channel6, DMA_IT_TE, ENABLE);
  
  /* Enable USART1 DMA RX request */
  USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
  DMA_Cmd(DMA1_Channel6, ENABLE);  
}


void Usart1_DMA_Configuration(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  /* DMA clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//DMA1
//  DMA_InitTypeDef DMA_InitStructure;
//  /* DMA clock enable */
//  /* DMA1 Channel4 (triggered by USART1 Tx event) Config */
  DMA_DeInit(DMA1_Channel4);  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART1_SEND_DATA;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  //一定不能是DMA_Mode_Circular，否则会一直发送数据
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel4, &DMA_InitStructure);
  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
  DMA_Cmd(DMA1_Channel4, DISABLE);    
  
  /* DMA1 Channel5 (triggered by USART1 Rx event) Config */
  DMA_DeInit(DMA1_Channel5);  
  DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40013804;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)usart1_recv_data.RecvData;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = USART1_DMA_BUFFER_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel5, &DMA_InitStructure);
  //DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
  //DMA_ITConfig(DMA1_Channel6, DMA_IT_TE, ENABLE);
  
  /* Enable USART1 DMA RX request */
  USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
  DMA_Cmd(DMA1_Channel5, ENABLE);  
}


/***************************************************************************
 * @fn          read_Usart2_DMA_FIFO
 *     
 * @brief       读取并处理DMA缓冲区中的数据
 *     
 * @data        2015年08月05日
 *     
 * @param       usart2_rbuf - 存储缓冲区信息
 *     
 * @return      void
 ***************************************************************************
 */ 
void read_Usart2_DMA_FIFO(usart_buffer_t* usart2_rbuf)
{
//DMA FIFO为环形FIFO，读数据时可能一帧的一部分在FIFO尾部，一部分在FIFO头部
  app_usart_proc_t rc = APP_USART_FRAME_NOT_COMPLETE;
  uint16_t dmaIndex2 = 0;
  
  memset(usart2_rbuf->rxbuf, 0x0, sizeof(usart2_rbuf->rxbuf));
  usart2_recv_data.curcnt = USART2_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel6); 
  while(dmaIndex2 < usart2_recv_data.curcnt)
  {
loop_check_head:    
    rc = check_usart2_frame_head(&dmaIndex2, &usart2_recv_data.curcnt);
    if(rc == APP_USART_FRM_HEAD_SUCCESS)
    {
loop_check_end: 
      rc =  check_usart2_frame_end_and_crc( &dmaIndex2 );
      if(rc == APP_USART_FRM_END_SUCCESS)
      {
        usart2_rbuf->rxbuf_payload_len = ((usart2_recv_data.RecvData[dmaIndex2+22]&0xffff)<<8) | \
                                           usart2_recv_data.RecvData[dmaIndex2+23];
        usart2_rbuf->rxbuf_len = usart2_rbuf->rxbuf_payload_len + 26;              
        memcpy(usart2_rbuf->rxbuf, &usart2_recv_data.RecvData[dmaIndex2], usart2_rbuf->rxbuf_len);
        dmaIndex2 += usart2_rbuf->rxbuf_len;
#ifdef SEGGER_DEBUG 
        SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"USART2 RECV:"RTT_CTRL_RESET"\n");        
        SEGGER_RTT_put_multi_char(&usart2_rbuf->rxbuf[0],usart2_rbuf->rxbuf_len);
#endif   
				
        //有中断说明有数据  
        usart_cur_frm_recv++;   
        uart2_frm_proc(usart2_rbuf->rxbuf,usart2_rbuf->rxbuf_len);
        memset(usart2_rbuf->rxbuf, 0x0, sizeof(usart2_rbuf->rxbuf)); 
//防止数据没有读完,延时1ms，看看数据有没有变化
        if((dmaIndex2 >= usart2_recv_data.curcnt) &&\
           ((dmaIndex2-usart2_rbuf->rxbuf_len) <= usart2_recv_data.curcnt))
        {
          //delay_ms(1);
          usart2_recv_data.precnt = usart2_recv_data.curcnt;
          usart2_recv_data.curcnt = USART2_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel6);
          if(usart2_recv_data.precnt == usart2_recv_data.curcnt)
          {
            break;          
          }
          else
          { 
            goto loop_check_head;        
          }
        }
      }
      else if(rc == APP_USART_CRC_ERR)
      {        
        dmaIndex2++;        
        usart2_rbuf->alarm_info[0] = 0x00;
        usart2_rbuf->alarm_info[1] = 0x04;  
        usart2_rbuf->alarm_info[2] = 0x03;
        usart2_rbuf->alarm_info[3] = 0x02;  
        usart2_rbuf->alarm_info_flag = 0x01;   
      }
      else if( rc == APP_USART_LEN_ERR)
      {        
        dmaIndex2++;  
        usart2_rbuf->alarm_info[0] = 0x00;
        usart2_rbuf->alarm_info[1] = 0x04;          
        usart2_rbuf->alarm_info[2] = 0x03;
        usart2_rbuf->alarm_info[3] = 0x03;  
        usart2_rbuf->alarm_info_flag = 0x01;           
      }
      else if( rc == APP_USART_FRAME_NOT_COMPLETE)
      {
        delay_ms(5);
        usart2_recv_data.precnt = usart2_recv_data.curcnt;
        usart2_recv_data.curcnt = USART2_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel6);
        if(usart2_recv_data.precnt == usart2_recv_data.curcnt)
        {          
          dmaIndex2++; 
          usart2_rbuf->alarm_info[0] = 0x00;
          usart2_rbuf->alarm_info[1] = 0x04;            
          usart2_rbuf->alarm_info[2] = 0x03;
          usart2_rbuf->alarm_info[3] = 0x03;            
          usart2_rbuf->alarm_info_flag = 0x01;            
        }
        else
        { 
          goto loop_check_end;
        }
      }      
    }
    else
    {      
      dmaIndex2++;  
      usart2_rbuf->alarm_info[0] = 0x00;
      usart2_rbuf->alarm_info[1] = 0x04;        
      usart2_rbuf->alarm_info[2] = 0x03;
      usart2_rbuf->alarm_info[3] = 0x03;  
      usart2_rbuf->alarm_info_flag = 0x01;    
    }
  }
  usart2_err_proc(usart2_rbuf);    
  
}

void read_Usart1_DMA_FIFO(usart_buffer_t* usart1_rbuf)
{
//DMA FIFO为环形FIFO，读数据时可能一帧的一部分在FIFO尾部，一部分在FIFO头部
  
  memset(usart1_rbuf->rxbuf, 0x0, sizeof(usart1_rbuf->rxbuf));
  usart1_recv_data.curcnt = USART1_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel5); 
  
loop_recv_data:   
 	
  if(1)
  {
     delay_ms(1);
     usart2_recv_data.precnt = usart2_recv_data.curcnt;
     usart2_recv_data.curcnt = USART2_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel5);
     if(usart2_recv_data.precnt == usart2_recv_data.curcnt)
     {    
			 
	     send_usartdata_to_ap(usart1_recv_data.RecvData,usart1_recv_data.curcnt);      			
			 
     }
     else
     { 
       goto loop_recv_data;        
     }
  }	
  
}


/***************************************************************************
 * @fn          check_usart2_frame_head
 *     
 * @brief       检测帧头
 *     
 * @data        2015年08月05日
 *     
 * @param       pBegin - 检测开始的位置
 *              pEnd   - 检测结束的位置
 *     
 * @return      APP_USART_FRM_HEAD_SUCCESS - 检测帧头成功
 *              APP_USART_FRM_HEAD_ERR     - 一直检测不到帧头
 ***************************************************************************
 */ 
app_usart_proc_t check_usart2_frame_head(uint16_t * pBegin, uint16_t * pEnd )
{
  app_usart_proc_t rc = APP_USART_FRAME_NOT_COMPLETE;
  
  while(*pBegin < *pEnd)
  {
    if(usart2_recv_data.RecvData[*pBegin] == FRM_HEAD)
    {
      rc = APP_USART_FRM_HEAD_SUCCESS; 
      break;
    }
    else
    {
      (*pBegin)++;
      rc = APP_USART_FRM_HEAD_ERR;
    }
  }
  return rc;
}

/***************************************************************************
 * @fn          check_usart2_frame_end_and_crc
 *     
 * @brief       检测帧尾并进行CRC校验
 *     
 * @data        2015年08月05日
 *     
 * @param       
 *     
 * @return      APP_USART_LEN_ERR            - 帧长度错误
 *              APP_USART_CRC_ERR            - CRC校验错误
 *              APP_USART_FRM_END_SUCCESS    - 帧尾正确，CRC校验通过
 *              APP_USART_FRAME_NOT_COMPLETE - 帧不完整
 ***************************************************************************
 */ 
app_usart_proc_t check_usart2_frame_end_and_crc(uint16_t *beginIndex)
{
  app_usart_proc_t rc = APP_USART_FRAME_NOT_COMPLETE;
  uint16_t rxbuf_payload_len = 0;
  uint16_t rxbuf_len = 0;
  rxbuf_payload_len= ((usart2_recv_data.RecvData[*beginIndex+22]&0xffff)<<8) | \
                       usart2_recv_data.RecvData[*beginIndex+23];
  rxbuf_len = rxbuf_payload_len + 26 ;
  if(rxbuf_payload_len > MAX_DATA_LEN)
  {
    rc = APP_USART_LEN_ERR;
  }
  else if(usart2_recv_data.RecvData[*beginIndex+rxbuf_len-1] == FRM_END)
  {
    
    if(!check_crc(&usart2_recv_data.RecvData[*beginIndex],rxbuf_len-1))
    {
      rc = APP_USART_CRC_ERR;
    } 
    else
    {    
      rc = APP_USART_FRM_END_SUCCESS;   
    }
  }
  else
  {
  }
  return rc;
}

/***************************************************************************
 * @fn          usart2_err_proc
 *     
 * @brief       帧错误处理
 *     
 * @data        2015年08月05日
 *     
 * @param       usart_rbuf - 串口缓冲区数据
 *     
 * @return      void
 ***************************************************************************
 */ 
void usart2_err_proc(usart_buffer_t* usart_rbuf)
{
  uint8_t msg[30];
  uint8_t len = 0;    
  if(usart_rbuf->alarm_info_flag)  
  {
    usart_rbuf->alarm_info_flag = 0;
    len =bui_ed_alarm(NULL,usart_rbuf->alarm_info, 4, msg);
		Send_ed_msg_to_uart(msg,len);
//    TCP_Send_Filter(msg,len);    
    memset(&usart_rbuf,0x00,sizeof(usart_rbuf));         
  }
  else
  {
  }
}

/***************************************************************************
 * @fn          TIM4_Configuration
 *     
 * @brief       TIM4配置,使用复位模式。TIM4的捕获/比较输入2和串口2的RX在硬件
 *              上相连, 串口有数据时，TIM4一直复位，一旦串口没有数据，则TIM4
 *              不再复位。TIM4超时后，会产生一个超时中断。
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void TIM4_Configuration(void)
{
  
  /* TIM4 configuration -------------------------------------*/ 
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;   
      /* Enable TIM4 clock */

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);
  TIM_DeInit(TIM4);
  TIM_TimeBaseStructure.TIM_Period = 65535;                           //65535
  TIM_TimeBaseStructure.TIM_Prescaler = (7200 - 1);                   //3600
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  TIM_ARRPreloadConfig(TIM4, DISABLE);  
  
  /* Output Compare  Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
  TIM_OCInitStructure.TIM_Pulse = 3;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OC1Init(TIM4, &TIM_OCInitStructure);
  
   /* TIM4 Channel 2 Input Capture Configuration */
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(TIM4, &TIM_ICInitStructure);

  /* TIM4 Input trigger configuration: External Trigger connected to TI2 */
  TIM_SelectInputTrigger(TIM4, TIM_TS_TI2FP2);
  
  /* TIM4 configuration in slave reset mode  where the timer counter is 
     re-initialied in response to rising edges on an input capture (TI2) */
  TIM_SelectSlaveMode(TIM4,  TIM_SlaveMode_Reset);
  /* TIM4 IT CC1 enable */
  TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
	
	NVIC_SetVectorTable (NVIC_VectTab_FLASH, NVIC_VECTTAB_FLASH_OFFSET);
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
  TIM_Cmd(TIM4, ENABLE);  
  
}  

void TIM2_Configuration(void)
{
  
  /* TIM2 configuration -------------------------------------*/ 
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;   
      /* Enable TIM2 clock */

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
  TIM_DeInit(TIM2);
  TIM_TimeBaseStructure.TIM_Period = 65535;                           //65535
  TIM_TimeBaseStructure.TIM_Prescaler = (7200 - 1);                   //3600
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  TIM_ARRPreloadConfig(TIM2, DISABLE);  
  
  /* Output Compare  Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
  TIM_OCInitStructure.TIM_Pulse = 10;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OC1Init(TIM2, &TIM_OCInitStructure);
  
   /* TIM2 Channel 2 Input Capture Configuration */
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(TIM2, &TIM_ICInitStructure);

  /* TIM4 Input trigger configuration: External Trigger connected to TI2 */
  TIM_SelectInputTrigger(TIM2, TIM_TS_TI2FP2);
  
  /* TIM4 configuration in slave reset mode  where the timer counter is 
     re-initialied in response to rising edges on an input capture (TI2) */
  TIM_SelectSlaveMode(TIM2,  TIM_SlaveMode_Reset);
  /* TIM4 IT CC1 enable */
  TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

	NVIC_SetVectorTable (NVIC_VectTab_FLASH, NVIC_VECTTAB_FLASH_OFFSET);
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
  TIM_Cmd(TIM2, ENABLE);  
  
}  


/***************************************************************************
 * @fn          TIM4_IRQHandler
 *     
 * @brief       TIM4超时中断
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void TIM4_IRQHandler(void)
{
#ifdef TIME_COUNT	
  uint16_t test_arry[1]; 
  uint8_t time_info[2];				
	test_arry[0]=TIM_GetCounter(TIM2);
	time_info[0]=((test_arry[0]>>8));
	time_info[1]=((test_arry[0]));
#ifdef SEGGER_DEBUG
 SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"TIM4 IRQHandler"RTT_CTRL_RESET"\n");
	SEGGER_RTT_put_multi_char((uint8_t *)(time_info),2);
#endif 
#endif
  TIM_Cmd(TIM4, DISABLE); 
  TIM_ClearFlag(TIM4, TIM_IT_CC1);  
  TIM_SetCounter(TIM4,0);
  
  if( globalFlag.uart2_dma_flag)
  {
    globalFlag.uart2_dma_flag = 0;   
    usart2_recv_data.recvFlag = 1;
    usart2_recv_data.recvfrmcnt++;
  } 
	NVIC_ClearPendingFlag(USART2_IRQn);
  USART2_NVIC_Configuration(ENABLE);
}

/***************************************************************************
 * @fn          TIM2_IRQHandler
 *     
 * @brief       TIM2超时中断
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void TIM2_IRQHandler(void)
{

  TIM_Cmd(TIM2, DISABLE); 
  TIM_ClearFlag(TIM2, TIM_IT_CC1);  
  TIM_SetCounter(TIM2,0);
  
  if( globalFlag.uart1_dma_flag)
  {
    globalFlag.uart1_dma_flag = 0;   
    usart1_recv_data.recvFlag = 1;
    usart1_recv_data.recvfrmcnt++;
  } 
	NVIC_ClearPendingFlag(USART1_IRQn);
  USART1_NVIC_Configuration(ENABLE);
}


/***************************************************************************
 * @fn          usart_getFrameReceiveCount
 *     
 * @brief       获取串口接收帧的数量
 *     
 * @data        2016年03月03日
 *     
 * @param       void
 *     
 * @return      返回串口接收帧的数量
 ***************************************************************************
 */ 

uint8_t usart_getFrameReceiveCount(void)
{
  return usart_cur_frm_recv;
}

/***************************************************************************
 * @fn          Iterates_usart2_buffer
 *     
 * @brief       读取串口发送的帧并进行处理
 *     
 * @data        2016年03月03日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void Iterates_usart2_buffer(void)
{    
  while(usart2_recv_data.recvfrmcnt)
  {
	
    usart2_recv_data.recvFlag = 0;   
    usart2_recv_data.recvfrmcnt--;        
    usart2_recv_data.curcnt = USART2_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel6); 
    
    read_Usart2_DMA_FIFO(&uart2_frm_rbuf);
		  
		
    Usart2_DMA_Configuration();
    usart2_recv_data.precnt = 0;
    usart2_recv_data.curcnt = 0; 
    memset(&usart2_recv_data.RecvData[0],0x0,sizeof(usart2_recv_data.RecvData));      
  }  
}

void Iterates_usart1_buffer(void)
{    
  while(usart1_recv_data.recvfrmcnt)
  {
	
    usart1_recv_data.recvFlag = 0;   
    usart1_recv_data.recvfrmcnt--;        
    usart1_recv_data.curcnt = USART1_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel5); 
    
    read_Usart1_DMA_FIFO(&uart1_frm_rbuf);
		  
    Usart1_DMA_Configuration();
    usart1_recv_data.precnt = 0;
    usart1_recv_data.curcnt = 0; 
    memset(&usart1_recv_data.RecvData[0],0x0,sizeof(usart1_recv_data.RecvData));      
  }  
}

