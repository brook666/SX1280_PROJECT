#include "usart1_dma_proc.h"
#include "uart_frm_proc.h"
#include "stm32f10x_it.h"
#include "net_frm_proc.h"
#include "ed_tx_buffer_fun.h"
#include "bui_pkg_fun.h"
#include "firmware_info_proc.h"
#include "ed_tx_buffer_fun.h"
#include "68_frm_proc.h"
#define  USART1_DMA_BUFFER_SIZE 1024

uint16_t  uart1_frm_data_cnt= 0;  //串口1使用
uint8_t   USART1_SEND_DATA[256];
DMA_RecvData_t usart1_recv_data = {0};
usart_buffer_t  uart1_frm_rbuf = {0};

void NVIC_USART1_ClearPendingFlag(uint8_t NVIC_IRQChannel)
{
	NVIC->ICPR[NVIC_IRQChannel >> 0x05] = (uint32_t)0x01 << (NVIC_IRQChannel & (uint8_t)0x1F);	
}

volatile  static uint8_t usart_cur_frm_recv = 1;
static uint8_t bui_recvdata(uint8_t *,uint8_t * );
/***************************************************************************
 * @fn          Usart1_Configuration
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
void Usart1_Configuration(uint32_t BaudRate,FunctionalState NewState)
{
  USART_InitTypeDef USART_InitStructure;        //定义一个串口结构体
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
	
  
  USART_InitStructure.USART_BaudRate            =BaudRate ;         //波特率115200
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;  //传输过程中使用8位数据
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;   //在帧结尾传输1位停止位
  USART_InitStructure.USART_Parity              = USART_Parity_No ;   //奇偶失能
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件流失能
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx; //接收和发送模式
  USART_Init(USART1, &USART_InitStructure);               //根据参数初始化串口寄存器
  USART_ITConfig(USART1,USART_IT_RXNE,NewState);              //使能串口中断接收

  __nop(); __nop();
  USART_Cmd(USART1, NewState);                          //使能串口外设
}

/***************************************************************************
 * @fn          USART1_NVIC_Configuration
 *     
 * @brief       配置串口1中断。
 *              USART1中断组为第3组，抢占优先级1，响应优先级0
 *
 * @data        2015年08月05日
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
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = NewState;
  NVIC_Init(&NVIC_InitStructure);
}

/***************************************************************************
 * @fn          Usart1_DMA_Configuration
 *     
 * @brief       配置USART1的DMA接收和发送,USART2->TX使用DMA1_Channel6,
 *              USART2->RX使用DMA1_Channel7。
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************/

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
 * @fn          TIM2_Configuration
 *     
 * @brief       TIM2配置,使用复位模式。TIM4的捕获/比较输入2和串口2的RX在硬件
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
void TIM2_Configuration(void)
{
  
  /* TIM2 configuration -------------------------------------*/ 
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef        TIM_OCInitStructure;
  TIM_ICInitTypeDef        TIM_ICInitStructure;
  NVIC_InitTypeDef         NVIC_InitStructure;   
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
	NVIC_USART1_ClearPendingFlag(USART1_IRQn);
  USART1_NVIC_Configuration(ENABLE);
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
//void read_Usart1_DMA_FIFO(usart_buffer_t* usart1_rbuf)
//{
////DMA FIFO为环形FIFO，读数据时可能一帧的一部分在FIFO尾部，一部分在FIFO头部
//  
//  memset(usart1_rbuf->rxbuf, 0x0, sizeof(usart1_rbuf->rxbuf));
//  usart1_recv_data.curcnt = USART1_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel5); 
//  
//loop_recv_data:   
// 	
//  if(1)
//  {
//     delay_ms(1);
//     usart1_recv_data.precnt = usart1_recv_data.curcnt;
//     usart1_recv_data.curcnt = USART2_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel5);
//     if(usart1_recv_data.precnt == usart1_recv_data.curcnt)
//     {    
//			 
//	     //send_usartdata_to_ap(usart1_recv_data.RecvData,usart1_recv_data.curcnt);      			
//			 send_bcast_pkg(usart1_recv_data.RecvData, usart1_recv_data.curcnt);
//     }
//     else
//     { 
//       goto loop_recv_data;        
//     }
//  }	
//  
//}
void read_Usart1_DMA_FIFO(usart_buffer_t* usart1_rbuf)
{
 //DMA FIFO为环形FIFO，读数据时可能一帧的一部分在FIFO尾部，一部分在FIFO头部
  app_usart_proc_t rc = APP_USART_FRAME_NOT_COMPLETE;
  uint16_t dmaIndex1 = 0;
	
	
  memset(usart1_rbuf->rxbuf, 0x0, sizeof(usart1_rbuf->rxbuf));
  usart1_recv_data.curcnt = USART1_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel5); 
		
	
  while(dmaIndex1 < usart1_recv_data.curcnt)
  {
loop_check_head:    
    rc = check_usart1_frame_head(&dmaIndex1, &usart1_recv_data.curcnt);
    if(rc == APP_USART_FRM_HEAD_SUCCESS)
    {
loop_check_end: 
      rc =  check_usart1_frame_end_and_crc( &dmaIndex1 );
      if(rc == APP_USART_FRM_END_SUCCESS)
      {
//				right_frm_num[0]++;
//#ifdef SEGGER_FRMCNT_DEBUG
//        SEGGER_RTT_put_multi_char(right_frm_num,1);	
//#endif					
        usart1_rbuf->rxbuf_payload_len = ((usart1_recv_data.RecvData[dmaIndex1+22]&0xffff)<<8) | \
                                           usart1_recv_data.RecvData[dmaIndex1+23];//数据域长度
        usart1_rbuf->rxbuf_len = usart1_rbuf->rxbuf_payload_len + 26; //69帧总长度             
        memcpy(usart1_rbuf->rxbuf, &usart1_recv_data.RecvData[dmaIndex1], usart1_rbuf->rxbuf_len);
        dmaIndex1 += usart1_rbuf->rxbuf_len;
#ifdef SEGGER_DEBUG 
        SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"USART1 RECV:"RTT_CTRL_RESET"\n");        
        SEGGER_RTT_put_multi_char(&usart1_rbuf->rxbuf[0],usart1_rbuf->rxbuf_len);
#endif  					
				
        //有中断说明有数据  
        usart_cur_frm_recv++;   
        uart_frm_proc(usart1_rbuf->rxbuf,usart1_rbuf->rxbuf_len);
        memset(usart1_rbuf->rxbuf, 0x0, sizeof(usart1_rbuf->rxbuf)); 
//防止数据没有读完,延时1ms，看看数据有没有变化
        if((dmaIndex1 >= usart1_recv_data.curcnt) &&\
           ((dmaIndex1-usart1_rbuf->rxbuf_len) <= usart1_recv_data.curcnt))
        {
          delay_ms(1);
          usart1_recv_data.precnt = usart1_recv_data.curcnt;
          usart1_recv_data.curcnt = USART1_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel5);
          if(usart1_recv_data.precnt == usart1_recv_data.curcnt)
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
        dmaIndex1++;        
        usart1_rbuf->alarm_info[0] = 0x00;
        usart1_rbuf->alarm_info[1] = 0x04;  
        usart1_rbuf->alarm_info[2] = 0x03;
        usart1_rbuf->alarm_info[3] = 0x02;  
        usart1_rbuf->alarm_info_flag = 0x01;   
      }
      else if( rc == APP_USART_LEN_ERR)
      {        
        dmaIndex1++;  
        usart1_rbuf->alarm_info[0] = 0x00;
        usart1_rbuf->alarm_info[1] = 0x04;          
        usart1_rbuf->alarm_info[2] = 0x03;
        usart1_rbuf->alarm_info[3] = 0x03;  
        usart1_rbuf->alarm_info_flag = 0x01;           
      }
      else if( rc == APP_USART_FRAME_NOT_COMPLETE)
      {
        delay_ms(5);
        usart1_recv_data.precnt = usart1_recv_data.curcnt;
        usart1_recv_data.curcnt = USART1_DMA_BUFFER_SIZE-DMA_GetCurrDataCounter(DMA1_Channel5);
        if(usart1_recv_data.precnt == usart1_recv_data.curcnt)
        {          
          dmaIndex1++; 
          usart1_rbuf->alarm_info[0] = 0x00;
          usart1_rbuf->alarm_info[1] = 0x04;            
          usart1_rbuf->alarm_info[2] = 0x03;
          usart1_rbuf->alarm_info[3] = 0x05;            
          usart1_rbuf->alarm_info_flag = 0x01;            
        }
        else
        { 
          goto loop_check_end;
        }
      }      
    }
    else
    {      
      dmaIndex1++;  
      usart1_rbuf->alarm_info[0] = 0x00;
      usart1_rbuf->alarm_info[1] = 0x04;        
      usart1_rbuf->alarm_info[2] = 0x03;
      usart1_rbuf->alarm_info[3] = 0x06;  
      usart1_rbuf->alarm_info_flag = 0x01;    
    }
  }
  usart1_err_proc(usart1_rbuf);    
   	
}

uint8_t bui_recvdata(uint8_t * buided_68_msg,uint8_t * recvdata)
{
	uint8_t i;
  uint8_t msg_len;
	uint8_t crc_config;
	uint16_t frm_68_len=(recvdata[22]<<8)|recvdata[23];
	uint8_t frm_69_crc_num=recvdata[24+frm_68_len];
	
	app_pkg_t *app_in_pkg;

  crc_config=gen_crc(recvdata,(24+frm_68_len));
	
	if((recvdata[0]==0x80)&&(crc_config==frm_69_crc_num))
	{
		msg_len=recvdata[9];
    buided_68_msg[0]=0x68;
    buided_68_msg[1]=0x00;
		buided_68_msg[2]=recvdata[7];
    buided_68_msg[3]=recvdata[8];
		buided_68_msg[4]=recvdata[3];
		buided_68_msg[5]=recvdata[4];
		buided_68_msg[6]=0x00;
		buided_68_msg[7]=msg_len;
		for(i=0;i<msg_len;i++)
    {
		  buided_68_msg[8+i]=recvdata[10+i];
		}
		buided_68_msg[8+msg_len]=gen_crc(buided_68_msg,8+msg_len);
		buided_68_msg[8+1+msg_len]=0x16;
		
//		bui_app_pkg(buided_68_msg, pIn_gtway_pkg);
		
		
	  return (12+msg_len);
	}

	if((recvdata[0]==0x69)&&(crc_config==frm_69_crc_num))
	{
		msg_len=recvdata[31];
    buided_68_msg[0]=0x68;
    buided_68_msg[1]=0x00;
		buided_68_msg[2]=recvdata[26];
    buided_68_msg[3]=recvdata[27];
		buided_68_msg[4]=recvdata[28];
		buided_68_msg[5]=recvdata[29];
		buided_68_msg[6]=0x00;
		buided_68_msg[7]=msg_len;
		for(i=0;i<msg_len;i++)
    {
		  buided_68_msg[8+i]=recvdata[32+i];
		}
		buided_68_msg[8+msg_len]=gen_crc(buided_68_msg,8+msg_len);
		buided_68_msg[8+1+msg_len]=0x16;
		
//		bui_app_pkg(buided_68_msg, pIn_gtway_pkg);
		
		
	  return (12+msg_len);
	}	
	
	else
  {
	  return 0;
	}

}


/**************************************************************************************************
 * @fn          bui_usart1_outnet_frm
 *
 * @brief       将串口1（从GPRS模块获得的）内部帧填充到要输出的网外通信帧中.
 *
 * @param       pOu_pkg         - 输出帧
 *              pInner_msg      - 内部帧
 *              inner_msg_len   - 内部帧长度
 *
 * @return      输出的网外通信帧长度
 **************************************************************************************************
 */


uint16_t bui_usart1_outnet_frm(uint8_t *pOut_msg, uint8_t *pInner_msg, uint16_t inner_msg_len)
{
  uint16_t ctr_code=0;
	uint8_t mobile_addr[6]={0x01,0x02,0x03,0x04,0x05,0x06};
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  
  if (pInner_msg== NULL )
  {
    return 0;
  }
	
	ctr_code = Reverse_trans_direction(0x0A);
	
  pOut_msg[0] = GTWAY_FRAME_HEAD;
  pOut_msg[1] = 0x00;                      //Reserve
  pOut_msg[2] = (uint8_t)(ctr_code >> 8);  //Control code(MSB)
  pOut_msg[3] = (uint8_t)(ctr_code);       //Control code(LSB)
  pOut_msg[4] = pMyGtwayAddr->type[1];     //Gateway Type
  pOut_msg[5] = pMyGtwayAddr->type[0];
  pOut_msg[6] = pMyGtwayAddr->addr[3];     //Gateway Addr
  pOut_msg[7] = pMyGtwayAddr->addr[2];
  pOut_msg[8] = pMyGtwayAddr->addr[1];
  pOut_msg[9] = pMyGtwayAddr->addr[0];
  memcpy(&pOut_msg[10], sGtwayServerIP.addr, 4);  //Gateway Server IP
  memcpy(&pOut_msg[14], mobile_addr, 6); //Mobile Phone MAC
  pOut_msg[20] = 0x00;                            //Frame counter(MSB)
  pOut_msg[21] = 0x00;                            //Frame counter(LSB)
  pOut_msg[22] = (uint8_t)(inner_msg_len >> 8);   //Data Length(MSB)
  pOut_msg[23] = (uint8_t)(inner_msg_len);        //Data Length(LSB)
  memcpy(&pOut_msg[24], pInner_msg, inner_msg_len); //Data
  pOut_msg[24 + inner_msg_len] = gen_crc(pOut_msg, 24 + inner_msg_len); //Check sum
  pOut_msg[25 + inner_msg_len] = GTWAY_FRAME_TAIL;	
	
  
//  /* 重新设置控制码的传输方向位 */
//  ctr_code = Reverse_trans_direction(0x0A);

//  pOu_pkg->frm_head = GTWAY_FRAME_HEAD;
//  pOu_pkg->reserve1 = 0xFF; //Reserve
//  pOu_pkg->ctr_code = 0x0000;
//  pOu_pkg->gtway_type = ((pMyGtwayAddr->type[1]<<8)|pMyGtwayAddr->type[0]); //Gateway Type
//  pOu_pkg->gtway_addr[0] = pMyGtwayAddr->addr[3]; //Gateway Addr
//  pOu_pkg->gtway_addr[1] = pMyGtwayAddr->addr[2];
//  pOu_pkg->gtway_addr[2] = pMyGtwayAddr->addr[1];
//  pOu_pkg->gtway_addr[3] = pMyGtwayAddr->addr[0];
//  memcpy(pOu_pkg->server_addr, sGtwayServerIP.addr, 4);  //Gateway Server IP
//  memcpy(pOu_pkg->mobile_addr, mobile_addr, 6); //Mobile Phone MAC
//  pOu_pkg->pid_num = 0x0000;  //Frame counter(MSB)
//  pOu_pkg->data_len = inner_msg_len;                //Data Length
//  memcpy(pOu_pkg->msg, pInner_msg, inner_msg_len); //Data
//  pOu_pkg->cs = 0x00;                               //Check sum
//  pOu_pkg->frm_end = GTWAY_FRAME_TAIL;

  return (26 + inner_msg_len);
}

/***************************************************************************
 * @fn          check_usart1_frame_head
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
app_usart_proc_t check_usart1_frame_head(uint16_t * pBegin, uint16_t * pEnd )
{
  app_usart_proc_t rc = APP_USART_FRAME_NOT_COMPLETE;
  
  while(*pBegin < *pEnd)
  {
    if(usart1_recv_data.RecvData[*pBegin] == FRM_HEAD)
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
 * @fn          check_usart1_frame_end_and_crc
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
app_usart_proc_t check_usart1_frame_end_and_crc(uint16_t *beginIndex)
{
  app_usart_proc_t rc = APP_USART_FRAME_NOT_COMPLETE;
  uint16_t rxbuf_payload_len = 0;
  uint16_t rxbuf_len = 0;
  rxbuf_payload_len= ((usart1_recv_data.RecvData[*beginIndex+22]&0xffff)<<8) | \
                       usart1_recv_data.RecvData[*beginIndex+23];//数据域长度
  rxbuf_len = rxbuf_payload_len + 26 ;//69帧总长度
  if(rxbuf_payload_len > MAX_DATA_LEN)
  {
    rc = APP_USART_LEN_ERR;
  }
  else if(usart1_recv_data.RecvData[*beginIndex+rxbuf_len-1] == FRM_END)
  {
    
    if(!check_crc(&usart1_recv_data.RecvData[*beginIndex],rxbuf_len-1))
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
 * @fn          usart1_err_proc
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
void usart1_err_proc(usart_buffer_t* usart_rbuf)
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

