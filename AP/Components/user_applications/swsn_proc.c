/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   swsn_proc.c
// Description: 主函数所在源文件.
// Author:      Leidi
// Version:     1.2
// Date:        2012-3-24
// History:     2014-9-25   Leidi 删除全局变量ApAddr,使用官方函数nwk_getAPAddress()获取AP地址.
//              2014-9-26   Leidi 删除全局变量change_flag,check_flag.
//                                将变量link_count改名为join_count,更符合实际.
//              2014-9-30   Leidi 全局变量myIpAddr在本文件中定义.
//                                全局变量proc_times_for_uart_frm_rbuf1/2该为外部定义.
//                                移除头文件net_freq_agility.h,暂不使用有关跳频的代码.
//                                节点接收到数据处理前设置RDY_OUTPUT_LOW(),处理后设置RDY_OUTPUT_HIGH().
//              2014-10-8   Leidi 除全局变量loop_num及相关代码.此变量可能用于检测主循环是否卡死.
//              2014-11-10  Leidi 移除头文件stm8s_uart.h.
//                                添加用于配置的头文件user_app_config.h.
//              2014-11-11  Leidi 使用的指示LED由LED2换为LED1.
//              2014-11-13  Leidi 修改有效AP判断的逻辑,只处理FLASH中的AP地址无效的情况.因为join()中使用非广播帧.
//              2014-12-09  Leidi 多次入网失败后不再尝试入网,直接进入主循环.
//              2015-01-04  Leidi 去除主循环中处理串口输入数据和应答帧的代码，因为LED控制器不会再用到.
//                                将主函数中个域网初始化部分代码提取为函数wpan_init().
//                                通过标志变量wpan_init_flag，在主循环中每隔一段时间尝试初始化个域网.
//              2015-01-08  Leidi 在主循环中检测标志变量Remoter_event_actuator_flag,执行遥控器触发事件.
//                                启动时使用函数Load_remoterItemsFromFLASH()将FLASH中遥控器信息读取到内存中.
//              2015-02-10  Leidi 若未定义PWM_OUTPUT宏,则在主函数中将ERR和RDY引脚置高,以控制继电器开启.
//                                通过外部进入授权模式时,调用函数Button_authOn(),进入由外置按键控制的授权模式.
//                                修复BUG:EXTI_Pin2_IRQHandler中未清除中断标志位.
//              2015-04-07 Leidi  删除PWM_OUTPUT宏.
//              2015-06-03 Leidi  在主循环中检测并执行闹钟动作.
//              2015-07-29 Leidi  移除有效AP判断的代码，在AP端判断.移除头文件引用radio_config_Si4438.h.
//                                删除全局变量myIpAddr.
//              2015-09-06 Leidi  删除有关处理IP地址的代码.
//              2015-09-14 Zengjia 将变量wpan_init_done更改为全局静态变量，删除节点信息后，重新选择网络加入。
*****************************************************************************/


#include "swsn_proc.h"

app_interface_t appFuncDomain[10];


static uint8_t reset_mrfi_flag = 0; //重启射频的标志




/* 心跳帧检测开关 */
uint8_t time_sync_switch = TIME_SYNC_SWITCH;

/* 初始化个域网标志,非零表示将要初始化个域网 */
uint8_t wpan_init_flag;

/*网络中是否存在网关标志，1表示存在网关，0表示不存在网关，默认为存在网关*/
uint8_t hasGateway = 1; 


/*应用层广播帧的数量,广播帧为网关发送，节点可以通过该帧来检测网络中是否有网关*/ 
extern volatile uint8_t bro_cast_total_num; 
extern volatile uint8_t net_frm_cur_num;
extern volatile uint8_t bro_cast_sem;
extern linkID_t sLinkID1;


static void wpan_init();

void swsn_info_init(void)
{
 //开启独立看门狗
#if defined ENABLE_WWDG
  BSP_WWDG_Init(); 
#elif defined ENABLE_IWDG
  BSP_IWDG_Init(); 
#endif
  //加载MAC地址
  Load_MacAddress();
  PRINT_DEBUG_MSG("MAC:");
  PRINT_DEBUG_BYTES((uint8_t *)nwk_getMyAddress(), NET_ADDR_SIZE);
  
 // BSP_TURN_ON_LED1() ;
  //初始化个域网参数(加入个域网) 
  wpan_init();
  //开启接收
  MRFI_RxOn();
  initAppFunc();
}

void swsn_app_proc(void)
{
/*  if(reset_mrfi_flag)
  {
    bspIState_t intState;
    reset_mrfi_flag = 0;
    BSP_ENTER_CRITICAL_SECTION(intState);
    MRFI_Init(); //重启射频模块
    MRFI_RxOn(); 
    BSP_EXIT_CRITICAL_SECTION(intState);
  }*/
  
  // 每隔一段时间尝试初始化个域网    
  if(wpan_init_flag)
  {
    wpan_init_flag = 0;
    wpan_init();
  }
  // 有单播帧待处理 
  if(net_frm_cur_num)
  {
#ifdef TEST_LED
    BSP_TOGGLE_LED1();
#endif
    net_frm_proc();
  }
  
  // 有广播帧待处理 
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
 * @brief       初始化个域网参数，并尝试加入个域网。若没有连接到正确的AP，则最多尝试3次；若已经成功
 *              连接，则直接返回.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
static void wpan_init()
{
  uint8_t join_count = 0, link_count = 0;
  static uint8_t wpan_init_cnt = 0;
  static uint8_t wpan_init_done = 0;
  // 若已经成功加入个域网，直接返回 
  if(wpan_init_done || !nwk_hasAP())
  {
    return;
  }

  do
  {
    PRINT_DEBUG_MSG("\nTry to Join AP...");
    
    if(SMPL_SUCCESS == SMPL_Init(sCB))//射频资源初始化,尝试(使用非广播帧)加入AP 
    {     
      PRINT_DEBUG_MSG("My AP is:");
      PRINT_DEBUG_BYTES((uint8_t *)nwk_getAPAddress(), NET_ADDR_SIZE);
      // 开启AP地址过滤,过滤非有效AP发来的帧 
      MRFI_SetApAddrFilter(TRUE);
      nwk_setAPAlive(1);
      
      NWK_DELAY(100);
      if(SMPL_SUCCESS == SMPL_Link(&sLinkID1))//尝试(使用非广播帧)连接AP,获得LID
      {        
        PRINT_DEBUG_MSG("Link AP success!\n");

        wpan_init_done = 1;//设置成功加入个域网标志
        
        break;
      }
      else
      {
        NWK_DELAY(100);
        PRINT_DEBUG_MSG("Link AP failed!\n");
        link_count++;
      }
    }
    else
    {
      NWK_DELAY(100);
     // PRINT_DEBUG_MSG("Join AP failed.\n");
      join_count++;
    }
  }while((join_count < 3) && (link_count < 3));//最多尝试3次
  
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
//  appFuncDomain[2].commandId_DI2 = 0x80;
//  appFuncDomain[2].pFunx = &ed_intra_proc;
  
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
 * @fn          TIM4_UPD_OVF_TRG_IRQHandler
 *
 * @brief       TIM4溢出中断，每隔100ms中断一次.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
BSP_ISR_FUNCTION(TIM4_UPD_OVF_TRG_IRQHandler, TIM4_UPD_OVF_TRG_IRQn+2)
{
  static uint8_t count_1sec  = 0;
  static uint8_t count_500ms = 0;
  static uint8_t frame_transmission_count;

  /* clear update interrupt flag */
  TIM4_CLEAN_UPDATE_INTERRUPT_FLAG();
  
  count_500ms++;
  if(count_500ms == 2)
  {
    count_500ms = 0;
    count_1sec ++;     
  }
  
//设置初始化个域网标志，这样主循环中会尝试初始化个域网  
  if(nwk_hasAP())
  {
    if((count_1sec%3 == 0) && (count_500ms == 0))
    {
      wpan_init_flag = 1; 
    }
  }
  
/* 同步帧检测 */
  if(time_sync_switch)//同步帧开启标志
  {
/* 如果网络中存在网关，则每隔1.5s检测有无帧的收发并判断是否重启射频 ； 
 * 如果网络中没有网关，则每个5分钟检测一次有无帧的变化并判断是否重启射频
*/
   /* if((count_1sec%8 == 0) && (count_500ms == 1))
    {
      // 如果帧的收发量无变化 
      if(frame_transmission_count == MRFI_getFrameTransmissionCount())
      {
        reset_mrfi_flag = 1;
      }
      
      //更新帧的收发量 
      frame_transmission_count = MRFI_getFrameTransmissionCount(); 
    }*/
  }
}

