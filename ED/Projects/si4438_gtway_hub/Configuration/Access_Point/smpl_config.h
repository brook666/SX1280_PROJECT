/**************************************************************************************************
  Filename:       smpl_config.dat
  Revised:        $Date: 2008-07-02 15:37:50 -0700 (Wed, 02 Jul 2008) $
  Revision:       $Revision: 17454 $
  Author:         $Author: lfriedman $

  Description:    This file supports the SimpliciTI Customer Configuration for End Devices.

  Copyright 2004-2007 Texas Instruments Incorporated. All rights reserved.

**************************************************************************************************/


/* Number of connections supported. each connection supports bi-directional
 * communication.  Access Points and Range Extenders can set this to 0 if they
 * do not host End Device objects
 */
#define NUM_CONNECTIONS 2

/*  ***  Size of low level queues for sent and received frames. Affects RAM usage  ***  */

/* AP needs larger input frame queue if it is supporting store-and-forward
 * clients because the forwarded messages are held here. Two is probably enough
 * for an End Device
 */
#define SIZE_INFRAME_Q 5

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
 * characters of length NET_ADDR_SIZE (found in nwk_types.h). the quotes (") are
 * necessary below unless the spaces are removed.
 */  
#define MAC_ADDR_SIZE 4
#define THIS_DEVICE_ADDRESS   {0x04, 0x03, 0xfd, 0xfd}   

#define ED_SERVER_IP {121, 41, 106, 93}

#define THIS_FIRMWARE_INFO  0x02000000
/* device type */
#define END_DEVICE
//#define RANGE_EXTENDER
#define TX_REMOTER
//#define ENABLE_RE

/* For polling End Devices we need to specify that they do so. Uncomment the 
 * macro definition below if this is a polling device. This field is used 
 * by the Access Point to know whether to reserve store-and-forward support 
 * for the polling End Device during the Join exchange.
 */
/* -DRX_POLLS */


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

#define ENABLE_CAD

#define IWDG_START

//#define RUOLIAN_AP
#define YIXIANG_AP


/**************************************************************************************************
*/
