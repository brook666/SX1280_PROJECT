/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ed_buffer_fun.c
// Description: User APP.
//              节点收发缓冲区处理头文件.
// Author:      Leidi
// Version:     1.1
// Date:        2014-10-21
// History:     2014-10-21  Leidi 初始版本建立.
//              2014-10-31  Leidi 修改函数proc_tx_ed_info()的逻辑,不需要应答的帧直接发送.
//              2014-11-03  Leidi 修改函数Write_tx_ed_buffer(),增加组播帧(0x67帧)的处理.
//              2014-04-24  Leidi 在每个节点的缓冲区中增加读写ID，以形成环形队列.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "ed_rx_buffer_fun.h"
#include "80_frm_proc.h"
#include "mrfi.h"
extern mrfiPacket_t mrfiIncomingPacket;
extern Data_Frm_Time Data_Frm_TimeInfo[10];
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
ed_rx_buffer_t   Ed_rx_buffer[SIZE_INFRAME_Q] = {0}; 
volatile uint8_t app_frm_num = 0;
/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
*/


/**************************************************************************************************
 * @fn          Write_rx_ed_buffer
 *
 * @brief       将接收到的节点68帧信息写入接收缓冲区.
 *
 * @param       lid        - 节点的lid
 *
 * @return      none
 **************************************************************************************************
 */
uint8_t Write_rx_ed_buffer(uint8_t lid)//将接收到的节点68帧信息写入接受缓冲区
{
	
	uint16_t time2_cnt[1];
	uint8_t buffer_index = 0;
	uint8_t frm_num=0;
	bspIState_t intState;
  uint8_t rc = 0;
  uint8_t  msg[MAX_APP_PAYLOAD], len;
    
  rc= recv_user_msg(lid, msg, &len); //从指定的用户读取数据，读到返回1，没读到返回0
	
	frm_num=msg[6];//帧序号
	time2_cnt[0]=TIM_GetCounter(TIM6);
	
#ifdef SEGGER_DEBUG
    SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"APP PACKET TIME"RTT_CTRL_RESET"\n");
#endif
	
	
	if((msg[1]&0x0F)==0x0C)
  {
		uint8_t i=0,j=0;
		uint8_t fram_cs=0;
		uint8_t msg_copy[MAX_APP_PAYLOAD];
#ifdef SEGGER_DEBUG1 
    SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"DATA PACKET:"RTT_CTRL_RESET"\n");          
#endif
		memcpy(msg_copy, msg, len); 		
    len=len+4;//数据帧长度+4	
    msg[7]=msg[7]+4;//数据域长度+4
		
	for(i=0;i<NUM_CONNECTIONS;i++)
  {
    if(Data_Frm_TimeInfo[i].Ed_Addr[0]==mrfiIncomingPacket.frame[5]&&Data_Frm_TimeInfo[i].Ed_Addr[1]==mrfiIncomingPacket.frame[6]&&\
			 Data_Frm_TimeInfo[i].Ed_Addr[2]==mrfiIncomingPacket.frame[7]&&Data_Frm_TimeInfo[i].Ed_Addr[3]==mrfiIncomingPacket.frame[8])   //mrfiIncomingPachet.frame[5]~[8]
		{
			Data_Frm_TimeInfo[i].Rx_Time_info[0]=time2_cnt[0]>>8;
			Data_Frm_TimeInfo[i].Rx_Time_info[1]=time2_cnt[0];
		  msg[8]=Data_Frm_TimeInfo[i].Tx_Time_info[0];
	  	msg[9]=Data_Frm_TimeInfo[i].Tx_Time_info[1];
	  	msg[10]=Data_Frm_TimeInfo[i].Rx_Time_info[0];
	  	msg[11]=Data_Frm_TimeInfo[i].Rx_Time_info[1];
			break;
		}
	}		

		for(i=0;i<(msg_copy[7]);i++)
		{
		  msg[12+i]=msg_copy[8+i];
		}
		for(j=0;j<(len-2);j++)
		{
		  fram_cs=fram_cs+msg[j];
		}
		msg[len-2]=fram_cs;
		msg[len-1]=0x16;
	}
  if(1 ==rc )
  {
#ifdef SEGGER_DEBUG1 
    SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"APP packet:"RTT_CTRL_RESET"\n");        
    SEGGER_RTT_put_multi_char(msg,len);     
#endif 		
		
		
    for(buffer_index = 0; buffer_index < SIZE_INFRAME_Q; buffer_index++)//写入缓冲区
    {
      if(Ed_rx_buffer[buffer_index].usage == RX_BUFFER_AVALIABLE)
      {
        Ed_rx_buffer[buffer_index].usage = RX_BUFFER_IN_USE;
        Ed_rx_buffer[buffer_index].lid   = lid;
        Ed_rx_buffer[buffer_index].ed_rx_info_len = len;
        memcpy(Ed_rx_buffer[buffer_index].ed_rx_info, msg, len);
							
        BSP_ENTER_CRITICAL_SECTION(intState);
        app_frm_num++;
        BSP_EXIT_CRITICAL_SECTION(intState);
        
        break;
      }
    }
  }
  return frm_num;
}

/**************************************************************************************************
 * @fn          Iterates_rx_ed_buffer
 *
 * @brief       遍历节点接收缓冲区,处理其中的信息.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Iterates_rx_ed_buffer(void) //遍历节点接收缓冲区，处理其中的信息
{
	uint8_t buffer_index = 0;
	bspIState_t intState;
  uint8_t out_msg_usart1[255];
	uint8_t out_msg_len;
	
  if (app_frm_num == 0)
  {
    return;
  }
  
  for(buffer_index = 0; buffer_index < SIZE_INFRAME_Q; buffer_index++)
  {
  	if(Ed_rx_buffer[buffer_index].usage == RX_BUFFER_IN_USE)
  	{
  		proc_net_msg(Ed_rx_buffer[buffer_index].lid, Ed_rx_buffer[buffer_index].ed_rx_info, \
  		             Ed_rx_buffer[buffer_index].ed_rx_info_len);
			if(((*(Ed_rx_buffer[buffer_index].ed_rx_info+1))&0x03)==0x00)       //如果是数据帧 则再发送给USART1
			{
//			  out_msg_len=buid_80_frm(Ed_rx_buffer[buffer_index].lid,Ed_rx_buffer[buffer_index].ed_rx_info, Ed_rx_buffer[buffer_index].ed_rx_info_len,out_msg_usart1);
//			  put_multi_hex_uart1(out_msg_usart1, out_msg_len);
//				  put_multi_hex_uart1(Ed_rx_buffer[buffer_index].ed_rx_info, Ed_rx_buffer[buffer_index].ed_rx_info_len);
  		}
			Ed_rx_buffer[buffer_index].usage = RX_BUFFER_AVALIABLE;
  		BSP_ENTER_CRITICAL_SECTION(intState);
  	  app_frm_num--;
  	  BSP_EXIT_CRITICAL_SECTION(intState);
  	}	
  }
  
  return ;
}

void Find_ed_info_in_rx_buffer(void)
{
	uint8_t buffer_index = 0;
	bspIState_t intState;
	
  
  for(buffer_index = 0; buffer_index < SIZE_INFRAME_Q; buffer_index++)
  {
  	if(Ed_rx_buffer[buffer_index].usage == RX_BUFFER_IN_USE)
  	{
  	  BSP_ENTER_CRITICAL_SECTION(intState);
  	  app_frm_num++;
  	  BSP_EXIT_CRITICAL_SECTION(intState);
  	  
  		break;
  	}
  }	
}





/**************************************************************************************************
 */
