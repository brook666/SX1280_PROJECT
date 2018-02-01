/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   ed_buffer_fun.c
// Description: User APP.
//              �ڵ㷢��ʧ�ܻ���������ͷ�ļ�.
//              �ڵ㷢��ʧ�ܺ󣬽���Ϣ����û��������������·��͡�
// Author:      Zengjia
// Version:     1.1
// Date:        2014-10-21
// History:     2015-09-29  Zengjia ��ʼ�汾����.
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "ed_tx_fail_buffer_fun.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
ed_tx_fail_buffer_t   Ed_tx_fail_buffer[SIZE_INFRAME_Q] = {0};
volatile uint8_t tx_fail_frm_num = 0;
//�ñ�־��λ�󣬿���ִ�ж���ǰ����ʧ�ܵ����ݽ����ط���
//�ñ�־�ڽڵ㻺�����������������λ��Ȼ�����ִ��һ���ط���
//֮��ÿ��1����λһ�Σ�һ������ʧ�ܵ����ݰ�ֻ�ط�3�Σ��ñ�־λ�ڶ�ʱ����ִ�����κ������λ��
//�ڳ�ʼ�������ж�ʱ������ֵ���������㣬��͵��¿�������ֻ�ط������Σ����������Σ�
//���Ͽ��Կ�����һ�����ݰ����������¿��ܱ�����3~4�Ρ�
volatile uint8_t emergency_dispatch_flag = 0; 
volatile uint8_t start_emergency_dispatch_in_rtc = 0;
extern volatile net_frm_back_t net_frm_back ;
/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
*/


/**************************************************************************************************
 * @fn          Write_tx_fail_ed_buffer
 *
 * @brief       �����յ��Ľڵ�68֡��Ϣд����ջ�����.
 *
 * @param       lid        - �ڵ��lid
 *              pIn_msg    - ָ��ڵ�68֡��Ϣ��ָ��
 *              pIn_len    - ���볤�ȵ�ָ��
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
 * @brief       �����ڵ���ջ�����,�������е���Ϣ.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Iterates_tx_fail_ed_buffer(void)
{
	static uint8_t buffer_index = 0;
//	uint8_t buffer_index = 0;
	bspIState_t intState;
  uint8_t  rc;
  uint8_t i,send_frm_num;
	linkID_t lid;
	
  if (!tx_fail_frm_num)
  {
    start_emergency_dispatch_in_rtc = 0; //�رն�ʱ���ͱ�־
    emergency_dispatch_flag = 0;
    return;
  }
  
  if(emergency_dispatch_flag)
  {
//    for(buffer_index=0; buffer_index < SIZE_INFRAME_Q; buffer_index++)
		while(buffer_index < SIZE_INFRAME_Q)
    {
      if(Ed_tx_fail_buffer[buffer_index].usage == TX_FAIL_BUFFER_IN_USE)
      {
				lid=Ed_tx_fail_buffer[buffer_index].lid;
				send_frm_num=Ed_tx_fail_buffer[buffer_index].ed_tx_fail_info[6];
        rc = send_net_pkg(Ed_tx_fail_buffer[buffer_index].lid, Ed_tx_fail_buffer[buffer_index].ed_tx_fail_info, \
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
        
        if(Ed_tx_fail_buffer[buffer_index].sendTimes == 2) 
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
				break;
			}
			else
      {
			  return;
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




/**************************************************************************************************
 */
