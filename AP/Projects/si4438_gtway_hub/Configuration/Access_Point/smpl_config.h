/**************************************************************************************************
  Filename:       smpl_config.dat
  Revised:        $Date: 2008-11-18 16:54:54 -0800 (Tue, 18 Nov 2008) $
  Revision:       $Revision: 18453 $
  Author:         $Author: lfriedman $

  Description:    This file supports the swsn Customer Configuration for Access Points.

**************************************************************************************************/

// History:     2014-11-7	Leidi	将射频输入队列缓冲区长度SIZE_INFRAME_Q由6改为10.

/* Number of connections supported. Each connection supports bi-directional
 * communication.  Access Points and Range Extenders can set this to 0 if they
 * do not host End Device objects.
 */
#ifndef SMPL_CONFIG_H
#define SMPL_CONFIG_H


#define NUM_CONNECTIONS  100

/*  ***  Size of low level queues for sent and received frames. Affects RAM usage  ***  */

/* AP needs larger input frame queue if it is supporting store-and-forward
 * clients because the forwarded messages are held here.
 */
#define SIZE_INFRAME_Q  20

/* The output frame queue can be small since Tx is done synchronously. Actually
 * 1 is probably enough. If an Access Point device is also hosting an End Device 
 * that sends to a sleeping peer the output queue should be larger -- the waiting 
 * frames in this case are held here. In that case the output frame queue should 
 * be bigger. 
 */
#define SIZE_OUTFRAME_Q 2

/* This device's address. The first byte is used as a filter on the CC1100/CC2500
 * radios so THE FIRST BYTE MUST NOT BE either 0x00 or 0xFF. Also, for these radios
 * on End Devices the first byte should be the least significant byte so the filtering
 * is maximally effective. Otherwise the frame has to be processed by the MCU before it
 * is recognized as not intended for the device. APs and REs run in promiscuous mode so
 * the filtering is not done. This macro intializes a static const array of unsigned
 * characters of length NET_ADDR_SIZE (found in nwk_types.h). The quotes (") are
 * necessary below unless the spaces are removed.
*/
#define THIS_FIRMWARE_INFO  0x02000000

#define MAC_ADDR_SIZE 4
#define MAC_TYPE_SIZE 2
#define THIS_DEVICE_TYPE       {0x08, 0xfe} 
//#define THIS_DEVICE_ADDRESS    {0xFE, 0x02, 0x08, 0xfe}     //测试用
//#define THIS_DEVICE_ADDRESS    {0x40, 0x01, 0x08, 0xfe}  
#define THIS_DEVICE_ADDRESS    {0xE5,0x00,0x00,0x08} 
#define REMOTER_ADDRESS    {0xfe, 0xfe, 0xfe, 0xfe} 

#define PERMISSION_ADDRESS {0xfd,0xfd,0xfa,0xfb,\
                            0xfa,0xfb,0xfd,0xfd,\
                            0xfc,0xfd,0xfd,0xfd,\
                            0xfd,0xfd,0xfa,0xfb}



/* device type */
#define ACCESS_POINT

/* In the spcial case in which the AP is a data hub, the AP will automaically
 * listen for a link each time a new device joins the network. This is a special
 * case scenario in which all End Device peers are the AP and every ED links
 * to the AP. In this scenario the ED must automatically try and link after the
 * Join reply.
 */
#define AP_IS_DATA_HUB

/* Store and forward support: number of clients */
#define NUM_STORE_AND_FWD_CLIENTS 3

#define STARTUP_JOINCONTEXT_ON

/*FEC enable*/
//#define  FEC


#if defined (STM32F10X_LD_VL) || (defined STM32F10X_MD_VL) || (defined STM32F10X_HD_VL)
/* #define SYSCLK_FREQ_HSE    HSE_VALUE */
 #define SYSCLK_FREQ_24MHz  24000000
#else
// #define SYSCLK_FREQ_HSE    HSE_VALUE 
/* #define SYSCLK_FREQ_24MHz  24000000 */ 
/* #define SYSCLK_FREQ_36MHz  36000000 */
/* #define SYSCLK_FREQ_48MHz  48000000 */
/* #define SYSCLK_FREQ_56MHz  56000000 */
#define SYSCLK_FREQ_72MHz  72000000	
#endif

//#define IWDG_START

#define ENABLE_CAD
//#define TIME_COUNT

//#define RUOLIAN_AP
#define YIXIANG_AP


#define ORIGINAL_LOGIN_NAME  {"admin"}  //{'a','d','m','i','n'}
#define ORIGINAL_LOGIN_KEY   {"admin"}  //{'a','d','m','i','n'}

#endif



