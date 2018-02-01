/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   mrfi_config.h
// Description: 射频接口(MRFI)配置文件.
// Author:      Leidi
// Version:     1.0
// Date:        2012-11-10
// History:     2014-11-10   Leidi 初始版本建立.其主要内容来自smpl_nwk_config.h.
//              2015-07-29   Leidi 将宏ENABLE_CRC,MANCHESTER_ENCODING从smpl_nwk_config.h中移入本文件中.
//                                 使用宏DEFAULT_TX_POWER_INDEX设置发射功率,移除宏DATARATE_x_KBAUD.
*****************************************************************************/
#ifndef _MRFI_CONFIG_H
#define _MRFI_CONFIG_H

/* 配置默认的射频发射功率. */
// INDEX最大为MRFI_NUM_POWER_SETTINGS - 1

#define DEFAULT_TX_POWER_INDEX 5

/* 启用CRC校验，过滤CRC错误的帧. */
//#define ENABLE_CRC

/* 启用曼彻斯特编码. */
#define MANCHESTER_ENCODING

#endif




/**************************************************************************************************
 */
 








