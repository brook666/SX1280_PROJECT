/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   swsn_proc.c
// Description: ����������Դ�ļ�.
// Author:      Leidi
// Version:     1.2
// Date:        2012-3-24
// History:     2014-9-25   Leidi ɾ��ȫ�ֱ���ApAddr,ʹ�ùٷ�����nwk_getAPAddress()��ȡAP��ַ.
//              2014-9-26   Leidi ɾ��ȫ�ֱ���change_flag,check_flag.
//                                ������link_count����Ϊjoin_count,������ʵ��.
//              2014-9-30   Leidi ȫ�ֱ���myIpAddr�ڱ��ļ��ж���.
//                                ȫ�ֱ���proc_times_for_uart_frm_rbuf1/2��Ϊ�ⲿ����.
//                                �Ƴ�ͷ�ļ�net_freq_agility.h,�ݲ�ʹ���й���Ƶ�Ĵ���.
//                                �ڵ���յ����ݴ���ǰ����RDY_OUTPUT_LOW(),���������RDY_OUTPUT_HIGH().
//              2014-10-8   Leidi ��ȫ�ֱ���loop_num����ش���.�˱����������ڼ����ѭ���Ƿ���.
//              2014-11-10  Leidi �Ƴ�ͷ�ļ�stm8s_uart.h.
//                                ����������õ�ͷ�ļ�user_app_config.h.
//              2014-11-11  Leidi ʹ�õ�ָʾLED��LED2��ΪLED1.
//              2014-11-13  Leidi �޸���ЧAP�жϵ��߼�,ֻ����FLASH�е�AP��ַ��Ч�����.��Ϊjoin()��ʹ�÷ǹ㲥֡.
//              2014-12-09  Leidi �������ʧ�ܺ��ٳ�������,ֱ�ӽ�����ѭ��.
//              2015-01-04  Leidi ȥ����ѭ���д������������ݺ�Ӧ��֡�Ĵ��룬��ΪLED�������������õ�.
//                                ���������и�������ʼ�����ִ�����ȡΪ����wpan_init().
//                                ͨ����־����wpan_init_flag������ѭ����ÿ��һ��ʱ�䳢�Գ�ʼ��������.
//              2015-01-08  Leidi ����ѭ���м���־����Remoter_event_actuator_flag,ִ��ң���������¼�.
//                                ����ʱʹ�ú���Load_remoterItemsFromFLASH()��FLASH��ң������Ϣ��ȡ���ڴ���.
//              2015-02-10  Leidi ��δ����PWM_OUTPUT��,�����������н�ERR��RDY�����ø�,�Կ��Ƽ̵�������.
//                                ͨ���ⲿ������Ȩģʽʱ,���ú���Button_authOn(),���������ð������Ƶ���Ȩģʽ.
//                                �޸�BUG:EXTI_Pin2_IRQHandler��δ����жϱ�־λ.
//              2015-04-07 Leidi  ɾ��PWM_OUTPUT��.
//              2015-06-03 Leidi  ����ѭ���м�Ⲣִ�����Ӷ���.
//              2015-07-29 Leidi  �Ƴ���ЧAP�жϵĴ��룬��AP���ж�.�Ƴ�ͷ�ļ�����radio_config_Si4438.h.
//                                ɾ��ȫ�ֱ���myIpAddr.
//              2015-09-06 Leidi  ɾ���йش���IP��ַ�Ĵ���.
//              2015-09-14 Zengjia ������wpan_init_done����Ϊȫ�־�̬������ɾ���ڵ���Ϣ������ѡ��������롣
*****************************************************************************/


#include "swsn_proc.h"


app_interface_t appFuncDomain[10];


static uint8_t reset_mrfi_flag = 0; //������Ƶ�ı�־

void  initCodeVersionInfo(void);

/*�˱ܴ�����¼*/
uint8_t  retreat_time_arry[10];
uint8_t *retreat_pointer=retreat_time_arry;

/* ��ʼ����������־,�����ʾ��Ҫ��ʼ�������� */
uint8_t wpan_init_flag;

/*�������Ƿ�������ر�־��1��ʾ�������أ�0��ʾ���������أ�Ĭ��Ϊ��������*/
uint8_t hasGateway = 1; 


/* ����֡��⿪�� */
uint8_t time_sync_switch = TIME_SYNC_SWITCH;

/*Ӧ�ò�㲥֡������,�㲥֡Ϊ���ط��ͣ��ڵ����ͨ����֡������������Ƿ�������*/ 
extern volatile uint8_t bro_cast_total_num; 
extern volatile uint8_t net_frm_cur_num;
extern volatile uint8_t bro_cast_sem;
extern linkID_t sLinkID1;


static void wpan_init(void);

void swsn_info_init(void)
{
 //�����������Ź�
#if defined ENABLE_WWDG
  BSP_WWDG_Init(); 
#elif defined ENABLE_IWDG
  BSP_IWDG_Init(); 
#endif
  //����MAC��ַ
  Load_MacAddress();
	initCodeVersionInfo();
//  PRINT_DEBUG_MSG("MAC:");
 // PRINT_DEBUG_BYTES((uint8_t *)nwk_getMyAddress(), NET_ADDR_SIZE);
  
 // BSP_TURN_ON_LED1() ;
  //��ʼ������������(���������) 
  wpan_init();
  //��������
  MRFI_RxOn();
  initAppFunc();
}

void swsn_app_proc(void)
{
  if(reset_mrfi_flag)
  {
    bspIState_t intState;
    reset_mrfi_flag = 0;
    BSP_ENTER_CRITICAL_SECTION(intState);
    MRFI_Init(); //������Ƶģ��
    MRFI_RxOn(); 
    BSP_EXIT_CRITICAL_SECTION(intState);
  }
  
  // ÿ��һ��ʱ�䳢�Գ�ʼ��������    
  if(wpan_init_flag)
  {
    wpan_init_flag = 0;
    wpan_init();
  }
  // �е���֡������ 
  if(net_frm_cur_num)
  {
#ifdef TEST_LED
    BSP_TOGGLE_LED1();
#endif
    net_frm_proc();
  }
  
  // �й㲥֡������ 
  if(bro_cast_sem)
  {
#ifdef TEST_LED
    BSP_TOGGLE_LED1();  
#endif
    bcast_frm_proc();
  }
  
#if defined ENABLE_WWDG
    BSP_WWDG_Refresh();
#elif defined ENABLE_IWDG
    BSP_IWDG_Refresh();
#endif
}


/**************************************************************************************************
 * @fn          wpan_init
 *
 * @brief       ��ʼ�������������������Լ������������û�����ӵ���ȷ��AP������ೢ��3�Σ����Ѿ��ɹ�
 *              ���ӣ���ֱ�ӷ���.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void wpan_init(void)
{
  uint8_t join_count = 0, link_count = 0;
  static uint8_t wpan_init_cnt = 0;
  static uint8_t wpan_init_done = 0;
  // ���Ѿ��ɹ������������ֱ�ӷ��� 
  if(wpan_init_done)// || !nwk_hasAP())
  {
    return;
  }

  do
  {
//    PRINT_DEBUG_MSG("\nTry to Join AP...");
    
    if(SMPL_SUCCESS == SMPL_Init(sCB))//��Ƶ��Դ��ʼ��,����(ʹ�÷ǹ㲥֡)����AP 
    {     
#ifdef SEGGER_DEBUG
SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"Join Rx Done"RTT_CTRL_RESET"\n");
#endif

      // ����AP��ַ����,���˷���ЧAP������֡ 
      MRFI_SetApAddrFilter(TRUE);
      nwk_setAPAlive(1);
      
      NWK_DELAY(500);
      if(SMPL_SUCCESS == SMPL_Link(&sLinkID1))//����(ʹ�÷ǹ㲥֡)����AP,���LID
      {        
#ifdef SEGGER_DEBUG
SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"Link AP SUCCESS"RTT_CTRL_RESET"\n");
#endif
        wpan_init_done = 1;//���óɹ������������־
        
        break;
      }
      else
      {
        NWK_DELAY(200);
   //     PRINT_DEBUG_MSG("Link AP failed!\n");
        link_count++;
      }
    }
    else
    {
      NWK_DELAY(200);
     // PRINT_DEBUG_MSG("Join AP failed.\n");
      join_count++;
    }
  }while((join_count < 3) && (link_count < 3));//��ೢ��3��
  
  wpan_init_cnt++;
  if(wpan_init_cnt == 3)
  {
    wpan_init_cnt = 0;
    nwk_setAPAlive(0);
  }
}


uint8_t initAppFunc(void)
{
  
  appFuncDomain[0].commandId_DI2 = 0x01;
  appFuncDomain[0].pFunx = &net_para_proc;
  appFuncDomain[1].commandId_DI2 = 0x02;
  appFuncDomain[1].pFunx = &net_status_proc;
	appFuncDomain[2].commandId_DI2 = 0xFF;
	appFuncDomain[2].pFunx = NULL;
	appFuncDomain[3].commandId_DI2 = 0xFF;
	appFuncDomain[3].pFunx = NULL;
	appFuncDomain[4].commandId_DI2 = 0xFF;
	appFuncDomain[4].pFunx = NULL;
	appFuncDomain[5].commandId_DI2 = 0xFF;
	appFuncDomain[5].pFunx = NULL;
	appFuncDomain[6].commandId_DI2 = 0xFF;
	appFuncDomain[6].pFunx = NULL;
	appFuncDomain[7].commandId_DI2 = 0xFF;
	appFuncDomain[7].pFunx = NULL;
	appFuncDomain[8].commandId_DI2 = 0xFF;
	appFuncDomain[8].pFunx = NULL;
	appFuncDomain[9].commandId_DI2 = 0xFF;
	appFuncDomain[9].pFunx = NULL;
 // appFuncDomain[2].commandId_DI2 = 0x80;
 // appFuncDomain[2].pFunx = &ed_intra_proc;
  
  return 1;
}

uint8_t registerAppFuncDomain(uint8_t commd_DI2,pAppFunc appFunc)
{
  uint8_t appIndex = 0;
  
  for(appIndex =0;appIndex<MAX_APP_FUNC;appIndex++)
  {
    if(appFuncDomain[appIndex].commandId_DI2 != 0)
      continue;
    appFuncDomain[appIndex].commandId_DI2 = commd_DI2;
    appFuncDomain[appIndex].pFunx = appFunc;
    break;
  }
  return 1;
}


/********************************** ****************************************************************
 * @fn          TIM5_UPD_OVF_TRG_IRQHandler
 *
 * @brief       TIM5����жϣ�ÿ��500ms�ж�һ��.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Time5_Pro_Func(void)
{
  static uint8_t count_1sec  = 0;
  static uint8_t count_500ms = 0;
  static uint8_t frame_transmission_count;

  /* clear update interrupt flag */
  
  count_500ms++;
  if(count_500ms == 2)
  {
    count_500ms = 0;
    count_1sec ++;     
  }
  
//���ó�ʼ����������־��������ѭ���л᳢�Գ�ʼ��������   
 // if(nwk_hasAP())
  {
    if((count_1sec%10 == 0) && (count_500ms == 0))     //5sʱ����
    {
      wpan_init_flag = 1; 
    }
  }
  
/* ͬ��֡��� */
  if(time_sync_switch)//ͬ��֡������־
  {
/* ��������д������أ���ÿ��5s�������֡���շ����ж��Ƿ�������Ƶ �� 
 * ���������û�����أ���ÿ��5���Ӽ��һ������֡�ı仯���ж��Ƿ�������Ƶ
*/
    if((count_1sec%5 == 0) && (count_500ms == 1))
    {
      // ���֡���շ����ޱ仯 
      if(frame_transmission_count == MRFI_getFrameTransmissionCount())
      {
        reset_mrfi_flag = 1;
      }
      
      //����֡���շ��� 
      frame_transmission_count = MRFI_getFrameTransmissionCount(); 
    }
  }
}


/***************************************************************************
 * @fn          restoreFactorySettings
 *     
 * @brief       ������汾��Ϣд��flash,DI1=0x88,DI0=1
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */
void RestoreFactorySettings(void)
{
  bspIState_t intState;
  BSP_ENTER_CRITICAL_SECTION(intState);
  
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR); 
#ifdef IWDG_START 
  IWDG_ReloadCounter();  
#endif													
  FLASH_ErasePage(LIST_IND_PTR);											
  FLASH_ErasePage(SYSINFO_PTR);											
  FLASH_ErasePage(SYSINFO_PTR+ONE_BLOCK_SIZE);  										
  FLASH_ErasePage(JOIN_LIST_START_ADDR);      //�������ڵ����Ϣ��
  FLASH_Lock();
 
  BSP_EXIT_CRITICAL_SECTION(intState);
}

/***************************************************************************
 * @fn          initCodeVersionInfo
 *     
 * @brief       ��ʼ������汾��Ϣ
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void  initCodeVersionInfo(void)
{
  uint32_t cur_version = *(uint32_t*)FIRMWARE_INFO_PTR;
  if((cur_version == 0) || (cur_version == 0xffffffff))
  {
    cur_version = THIS_FIRMWARE_INFO;
    WriteCodeVersionInfoToFlash(&cur_version);
  }
}

/***************************************************************************
 * @fn          WriteCodeVersionInfoToFlash
 *     
 * @brief       ������汾��Ϣд��flash,DI1=0x88,DI0=1
 *     
 * @data        2015��08��05��
 *     
 * @param       curVersion - ����汾��Ϣ
 *     
 * @return      void
 ***************************************************************************
 */ 
void  WriteCodeVersionInfoToFlash(uint32_t *curVersion)
{
  bspIState_t intState;
  
  BSP_ENTER_CRITICAL_SECTION(intState);
  
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
  FLASH_ErasePage(FIRMWARE_INFO_PTR);
  
  FLASH_ProgramWord(FIRMWARE_INFO_PTR, *curVersion);

  FLASH_Lock();

  BSP_EXIT_CRITICAL_SECTION(intState);    
  
}
