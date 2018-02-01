/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   app_layer.h
// Description: 应用层相关函数.
// Author:      Zengjia
// Version:     1.0
// Date:        2015-12-24
// History:     2015-12-24  Zengjia 初始版本建立.
*****************************************************************************/

#ifndef _SWSN_LIB_H
#define _SWSN_LIB_H

//包含stm8l外设驱动的头文件
#include "swsn_types.h"


/*
  版级资源初始化
*/
  void BSP_Init(void); 

/*
  flash操作，保存一些必要的信息。
  startAddress -- 起始地址；programByte -- 指向待写入的数据；programLen -- 待写入数据的长度
*/
void FLASH_WriteProgram(uint32_t startAddress, const uint8_t *programByte, uint16_t programLen);

/*
  日期的获取和设置：
  BSP_setDateTime -- 设置当前的日期和时间.daytime - (输入)日期时间结构体
  BSP_getDateTime -- 获取当前的日期和时间.daytime - (输出)日期时间结构体
*/
void BSP_setDateTime(daytime_t* daytime);
void BSP_getDateTime(daytime_t* daytime);
void BSP_RCT_Reinit_for_Awu(void);
void swsn_wait_relay_stable(void);
/*
上电复位一分钟内允许遥控器直接配对和解除相关函数：
abolish_remoter_direct_pairing --上电一分钟后取消遥控器直接配对和解除功能
isAllow_remoter_direct_pairing --判断是否允许遥控器直接配对和解除
*/
uint8_t abolish_remoter_direct_pairing(void);
uint8_t isAllow_remoter_direct_pairing(void);

//非精确延时，如果开启看门狗，喂狗动作会在函数内部执行，
//不用考虑该延时函数的超时时间。
void SWSN_DELAY(uint16_t  ms);

/*
  协议栈初始化、连接集中器
*/
void swsn_info_init(void);

/*
  处理swsn协议的网络事件，不可丢失
*/
void swsn_app_proc(void);

/*
  注册功能模块函数。
 命令标识分为4个字节 DI3 DI2 DI1 DI0,其中DI3表示公司代号，DI2表示功能域，
 DI1表示功能域里面不同的功能，DI0：1表示写，0表示读。
 commd_DI2 -- 命令标识DI2
 appFunc   -- 功能域函数
*/
uint8_t registerAppFuncDomain(uint8_t commd_DI2,pAppFunc appFunc);


/*向集中器发来的帧，发送回复帧。
  in_frame   -- 输入的的帧
  reply_para -- 回复的数据
  para_len   -- 回复数据的长度
*/
uint8_t send_reply_msg(const app_frame_t *in_frame, const uint8_t *reply_para, uint8_t para_len);

/*向集中器发来的帧，发送告警帧。
  in_frame   -- 输入的的帧
  err_info   -- 告警数据
  para_len   -- 告警数据的长度
*/
uint8_t send_alarm_msg(const app_frame_t *in_frame, const uint8_t *err_info, uint8_t err_len);

/*节点主动向集中器发送命令帧。
  commandID   -- 命令标识
  data        -- 有效数据
  data_len    -- 有效数据的长度
*/
uint8_t send_command_msg(uint8_t *commandID, uint8_t *data, uint8_t data_len);

#endif

