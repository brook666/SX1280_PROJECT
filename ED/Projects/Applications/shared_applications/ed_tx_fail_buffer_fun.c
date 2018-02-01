#include "ed_tx_fail_buffer_fun.h"





ed_tx_fail_buffer_t   Ed_tx_fail_buffer[SIZE_INFRAME_Q] = {0};
volatile uint8_t tx_fail_frm_num = 0;
//该标志置位后，可以执行对先前发送失败的数据进行重发；
//该标志在节点缓冲区发送完后，立刻置位，然后可以执行一次重发；
//之后每个1秒置位一次，一个发送失败的数据包只重发3次，该标志位在定时器中执行两次后结束置位。
//在初始化过程中定时器计数值并不会清零，这就导致可能数据只重发的两次，而不是三次；
//从上可以看出，一个数据包在最坏的情况下可能被发送3~4次。
volatile uint8_t emergency_dispatch_flag = 0; 
volatile uint8_t start_emergency_dispatch_in_rtc = 0;
/**************************************************************************************************
 * @fn          Write_tx_fail_ed_buffer
 *
 * @brief       将接收到的节点68帧信息写入接收缓冲区.
 *
 * @param       lid        - 节点的lid
 *              pIn_msg    - 指向节点68帧信息的指针
 *              pIn_len    - 输入长度的指针
 *
 * @return      none
 **************************************************************************************************
 */
void Write_tx_fail_ed_buffer(uint8_t lid, uint8_t *pIn_msg, uint8_t* pIn_len)
{
	uint8_t buffer_index = 0;
	bspIState_t intState;
	
  if((pIn_msg == NULL) || (pIn_len == NULL))
  {
  	return ;
  }
  for(buffer_index = 0; buffer_index < SIZE_INFRAME_Q; buffer_index++)
  {
    if(Ed_tx_fail_buffer[buffer_index].usage == TX_FAIL_BUFFER_IN_USE)
    {
      if(Ed_tx_fail_buffer[buffer_index].lid == lid)
      {
        if(!memcmp(pIn_msg, Ed_tx_fail_buffer[buffer_index].ed_tx_fail_info,* pIn_len))
        {
          return;
        }
      }
    }
  }
  
  for(buffer_index = 0; buffer_index < SIZE_INFRAME_Q; buffer_index++)
  {
  	if(Ed_tx_fail_buffer[buffer_index].usage == TX_FAIL_BUFFER_AVALIABLE)
  	{
  		Ed_tx_fail_buffer[buffer_index].usage = TX_FAIL_BUFFER_IN_USE;
  		Ed_tx_fail_buffer[buffer_index].lid   = lid;
  		Ed_tx_fail_buffer[buffer_index].sendTimes = 0;
      Ed_tx_fail_buffer[buffer_index].ed_tx_fail_info_len = *pIn_len;
  		memcpy(Ed_tx_fail_buffer[buffer_index].ed_tx_fail_info, pIn_msg, *pIn_len);

  	  BSP_ENTER_CRITICAL_SECTION(intState);
  	  tx_fail_frm_num++;
  	  BSP_EXIT_CRITICAL_SECTION(intState);
  		break;
  	}
  }
  return;
}


/**************************************************************************************************
 * @fn          Iterates_tx_fail_ed_buffer
 *
 * @brief       遍历节点发送失败缓冲区,处理其中的信息.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Iterates_tx_fail_ed_buffer(void)
{
	static uint8_t buffer_index = 0;
	bspIState_t intState;
  uint8_t  rc;
  
  if (!tx_fail_frm_num)
  {
    start_emergency_dispatch_in_rtc = 0; //关闭定时发送标志
    emergency_dispatch_flag = 0;
    return;
  }
	if (tx_fail_frm_num>0)
  {
    start_emergency_dispatch_in_rtc =1 ; //开启定时发送标志
  }
  
  if(emergency_dispatch_flag)
  {
 //   for(buffer_index = 0; buffer_index < SIZE_INFRAME_Q; buffer_index++)
		while(buffer_index < SIZE_INFRAME_Q)
    {
      if(Ed_tx_fail_buffer[buffer_index].usage == TX_FAIL_BUFFER_IN_USE)
      {
        rc = send_usart_data_pkg(Ed_tx_fail_buffer[buffer_index].lid, Ed_tx_fail_buffer[buffer_index].ed_tx_fail_info, \
                          Ed_tx_fail_buffer[buffer_index].ed_tx_fail_info_len);
        if(1 == rc)
        {
          Ed_tx_fail_buffer[buffer_index].usage = TX_FAIL_BUFFER_AVALIABLE;
          BSP_ENTER_CRITICAL_SECTION(intState);
          tx_fail_frm_num--;
          BSP_EXIT_CRITICAL_SECTION(intState);
        }
        else
        {	
          Ed_tx_fail_buffer[buffer_index].sendTimes ++; 
        }
        
        if(Ed_tx_fail_buffer[buffer_index].sendTimes == 4) 
        { 
          Ed_tx_fail_buffer[buffer_index].usage = TX_FAIL_BUFFER_AVALIABLE;
          BSP_ENTER_CRITICAL_SECTION(intState);
          tx_fail_frm_num--;
          BSP_EXIT_CRITICAL_SECTION(intState);  			
        }	
      }	
			buffer_index++;
			if(buffer_index==SIZE_INFRAME_Q)
      {
			  buffer_index=0;
			}
			else
      {
			  break;
			}			
    }
  }
  emergency_dispatch_flag = 0;
  
  return ;
}

void Find_ed_info_in_tx_fail_buffer(void)
{
	uint8_t buffer_index = 0;
	bspIState_t intState;
	
  
  for(buffer_index = 0; buffer_index < SIZE_INFRAME_Q; buffer_index++)
  {
  	if(Ed_tx_fail_buffer[buffer_index].usage == TX_FAIL_BUFFER_IN_USE)
  	{
  	  BSP_ENTER_CRITICAL_SECTION(intState);
  	  tx_fail_frm_num++;
  	  BSP_EXIT_CRITICAL_SECTION(intState);
  	  
  		break;
  	}
  }	
}

