#include "usart2_dma_proc.h"
#include "gtway_frm_proc.h"
#include "uart_frm_proc.h"
#include "stm32f10x_it.h"


uint16_t  uart2_frm_data_cnt= 0;  //����2ʹ��
uint8_t   USART2_SEND_DATA[100];
DMA_RecvData_t usart2_recv_data = {0};
usart_buffer_t  uart2_frm_rbuf = {0};


volatile  static uint8_t usart_cur_frm_recv = 1;

void NVIC_ClearPendingFlag(uint8_t NVIC_IRQChannel)
{
	NVIC->ICPR[NVIC_IRQChannel >> 0x05] = (uint32_t)0x01 << (NVIC_IRQChannel & (uint8_t)0x1F);	
}

/***************************************************************************
 * @fn          Usart2_DMA_Configuration
 *     
 * @brief       ����USART1��DMA���պͷ���,USART2->TXʹ��DMA1_Channel6,
 *              USART2->RXʹ��DMA1_Channel7��
 *     
 * @data        2015��08��05��
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
  //һ��������DMA_Mode_Circular�������һֱ��������
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

/***************************************************************************
 * @fn          read_Usart2_DMA_FIFO
 *     
 * @brief       ��ȡ������DMA�������е�����
 *     
 * @data        2015��08��05��
 *     
 * @param       usart2_rbuf - �洢��������Ϣ
 *     
 * @return      void
 ***************************************************************************
 */ 
void read_Usart2_DMA_FIFO(usart_buffer_t* usart2_rbuf)
{
//DMA FIFOΪ����FIFO��������ʱ����һ֡��һ������FIFOβ����һ������FIFOͷ��
  app_usart_proc_t rc = APP_USART_FRAME_NOT_COMPLETE;
  uint16_t dmaIndex2 = 0;
//  static uint8_t right_frm_num[1]={0};
	
	
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
				
        //���ж�˵��������  
        usart_cur_frm_recv++;   
        uart_frm_proc(usart2_rbuf->rxbuf,usart2_rbuf->rxbuf_len);
        memset(usart2_rbuf->rxbuf, 0x0, sizeof(usart2_rbuf->rxbuf)); 
//��ֹ����û�ж���,��ʱ1ms������������û�б仯
        if((dmaIndex2 >= usart2_recv_data.curcnt) &&\
           ((dmaIndex2-usart2_rbuf->rxbuf_len) <= usart2_recv_data.curcnt))
        {
          delay_ms(1);
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
          usart2_rbuf->alarm_info[3] = 0x05;            
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
      usart2_rbuf->alarm_info[3] = 0x06;  
      usart2_rbuf->alarm_info_flag = 0x01;    
    }
  }
  usart2_err_proc(usart2_rbuf);    
  
}


/***************************************************************************
 * @fn          check_usart2_frame_head
 *     
 * @brief       ���֡ͷ
 *     
 * @data        2015��08��05��
 *     
 * @param       pBegin - ��⿪ʼ��λ��
 *              pEnd   - ��������λ��
 *     
 * @return      APP_USART_FRM_HEAD_SUCCESS - ���֡ͷ�ɹ�
 *              APP_USART_FRM_HEAD_ERR     - һֱ��ⲻ��֡ͷ
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
 * @brief       ���֡β������CRCУ��
 *     
 * @data        2015��08��05��
 *     
 * @param       
 *     
 * @return      APP_USART_LEN_ERR            - ֡���ȴ���
 *              APP_USART_CRC_ERR            - CRCУ�����
 *              APP_USART_FRM_END_SUCCESS    - ֡β��ȷ��CRCУ��ͨ��
 *              APP_USART_FRAME_NOT_COMPLETE - ֡������
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
 * @brief       ֡������
 *     
 * @data        2015��08��05��
 *     
 * @param       usart_rbuf - ���ڻ���������
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
    len =bui_gtway_alarm(NULL,usart_rbuf->alarm_info, 4, msg);
#ifdef SEGGER_FRMCNT_DEBUG
        SEGGER_RTT_put_multi_char(msg,len);	
#endif				
		
		Send_gtway_msg_to_uart(msg,len);
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
 * @brief       TIM4����,ʹ�ø�λģʽ��TIM4�Ĳ���/�Ƚ�����2�ʹ���2��RX��Ӳ��
 *              ������, ����������ʱ��TIM4һֱ��λ��һ������û�����ݣ���TIM4
 *              ���ٸ�λ��TIM4��ʱ�󣬻����һ����ʱ�жϡ�
 *     
 * @data        2015��08��05��
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
  TIM_OCInitStructure.TIM_Pulse = 10;
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

  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
  TIM_Cmd(TIM4, ENABLE);  
  
}  

/***************************************************************************
 * @fn          TIM4_IRQHandler
 *     
 * @brief       TIM4��ʱ�ж�
 *     
 * @data        2015��08��05��
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
	test_arry[0]=TIM_GetCounter(TIM6);
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
//  NVIC->ICPR[1] =0X40;//������ڶ��жϱ�־
  //Usart2_Configuration(115200,ENABLE);
  USART2_NVIC_Configuration(ENABLE);
}

/***************************************************************************
 * @fn          usart_getFrameReceiveCount
 *     
 * @brief       ��ȡ���ڽ���֡������
 *     
 * @data        2016��03��03��
 *     
 * @param       void
 *     
 * @return      ���ش��ڽ���֡������
 ***************************************************************************
 */ 

uint8_t usart_getFrameReceiveCount(void)
{
  return usart_cur_frm_recv;
}

/***************************************************************************
 * @fn          Iterates_usart2_buffer
 *     
 * @brief       ��ȡ���ڷ��͵�֡�����д���
 *     
 * @data        2016��03��03��
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




