/**************************************************************************************************
  Filename:       smpl_nwk_config.dat
  Revised:        $Date: 2009-02-07 14:21:07 -0700 (Sat, 07 Feb 2009) $
  Revision:       $Revision: 19010 $
  Author:         $Author: lfriedman $

  Description:    This file supports the swsn Customer Configuration for overall network.

**************************************************************************************************/
#ifndef SMPL_NWK_CONFIG_H
#define SMPL_NWK_CONFIG_H

/* max hop count */
#define MAX_HOPS 3

/* max hops away from and AP. Keeps hop count and therefore replay
 * storms down for sending to and from polling End Devices. Also used
 * when joining since the EDs can't be more than 1 hop away.
 */
#define MAX_HOPS_FROM_AP 1

/* Maximum size of Network application payload. Do not change unless
 * protocol changes are reflected in different maximum network
 * application payload size.
 */
#define MAX_NWK_PAYLOAD 9

/* Maximum size of application payload */
#define MAX_APP_PAYLOAD 200

/* default Link token */
#define DEFAULT_LINK_TOKEN   0x02030405

/* default Join token */   
#define DEFAULT_JOIN_TOKEN 0x050108fe        //智能展厅

/* define Frequency Agility as active for this build */
//#define FREQUENCY_AGILITY

/* Remove 'x' corruption to enable application autoacknowledge support. Requires extended API as well */
#define APP_AUTO_ACK

/* Remove 'x' corruption to enable Extended API */
#define EXTENDED_API

/* Remove 'x' corruption to enable security. */
#define xSMPL_SECURE

/* Remove 'x' corruption to enable NV object support. */
#define xNVOBJECT_SUPPORT

/* Remove 'x' corruption to enable software timer. */
//#define SW_TIMER
//#define FEC_ENABLE

#define  SYNC_TIME   1

#define  BROAD_TIME  5  //s 
#define  BROAD_DELAY 1  //s
/* 配置射频数据收发速率. */
//#define DATARATE_1_2KBAUD
//#define DATARATE_38_4KBAUD
#define DATARATE_100KBAUD
//#define DATARATE_250KBAUD

/* 配置默认的射频发射功率. */
//#define TX_POWER_10_DBM
#define TX_POWER_7_DBM
//#define TX_POWER_0_DBM
//#define TX_POWER_MINUS10_DBM
//#define TX_POWER_MINUS15_DBM
//#define TX_POWER_MINUS20_DBM
//#define TX_POWER_MINUS30_DBM

#define ENABLE_CRC
//#define MANCHESTER_ENCODING


#endif




