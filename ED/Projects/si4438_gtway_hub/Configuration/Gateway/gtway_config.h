/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   gtway_config.h
// Description: 网关配置文件.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-23
// History:     2014-10-23  Leidi 添加服务器IP地址配置宏.
//              2014-10-25  Leidi 删除不再使用的宏定义MAX_RESVR_PKG.
//              2015-01-21  Leidi GTWAY_SERVER_IP由{42, 121, 125, 45}改为{121, 41, 106, 93}.
*****************************************************************************/
   
#ifndef _GTWAY_CONFIG_H
#define _GTWAY_CONFIG_H

/* 网关所连接的服务器IP地址 */
#define GTWAY_SERVER_IP {121, 41, 106, 93}


#define this_gtway_address  {0x01,0x00,0x00,0x00}
#define this_gtway_type     {0xFC,0x00}


#endif


/**************************************************************************************************
 */


