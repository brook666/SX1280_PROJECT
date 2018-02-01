/**************************************************************************************************
  Revised:        $Date: 2009-01-13 16:32:00 -0700 (Wed, 13 Jan 2009) $
  Revision:       $Revision: 18768 $

**************************************************************************************************/

/* ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 *   MRFI (Minimal RF Interface)
 *   Definition and abstraction for radio targets.
 * ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 */
#ifndef MRFI_DEFS_H
#define MRFI_DEFS_H


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp.h"


/* ------------------------------------------------------------------------------------------------
 *                                       Common Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MRFI_CCA_RETRIES        4

#define MRFI_ASSERT(x)          BSP_ASSERT(x)
#define MRFI_FORCE_ASSERT()     BSP_FORCE_ASSERT()
#define MRFI_ASSERTS_ARE_ON     BSP_ASSERTS_ARE_ON


/* ------------------------------------------------------------------------------------------------
 *                                    Radio Family Assigment
 * ------------------------------------------------------------------------------------------------
 */

/* ------ Radio Family 1 ------ */
#if (defined MRFI_SI4438) /* 433MHz RF Transceiver */
#define MRFI_RADIO_FAMILY1

/* ------ Radio Family 2 ------ */
#elif (defined MRFI_SI4463) /* 433MHz¡¢915Mhz SoC */ 
#define MRFI_RADIO_FAMILY2

#else
#error "ERROR: Unknown or missing radio selection."
#endif


/* ------------------------------------------------------------------------------------------------
 *                                Radio Family 1 / Radio Family 2 
 * ------------------------------------------------------------------------------------------------
 */
#if (defined MRFI_RADIO_FAMILY1) || (defined MRFI_RADIO_FAMILY2)

#define __mrfi_LENGTH_FIELD_SIZE__      1
#define __mrfi_ADDR_SIZE__              4
#define __mrfi_MAX_PAYLOAD_SIZE__       20

#define __mrfi_RX_METRICS_SIZE__        2
#define __mrfi_RX_METRICS_RSSI_OFS__    0
#define __mrfi_RX_METRICS_CRC_LQI_OFS__ 1
#define __mrfi_RX_METRICS_CRC_OK_MASK__ 0x80
#define __mrfi_RX_METRICS_LQI_MASK__    0x7F

#define __mrfi_NUM_LOGICAL_CHANS__      4
#define __mrfi_NUM_POWER_SETTINGS__     6

#define __mrfi_BACKOFF_PERIOD_USECS__   250

#define __mrfi_LENGTH_FIELD_OFS__       0
#define __mrfi_DST_ADDR_OFS__           (__mrfi_LENGTH_FIELD_OFS__ + __mrfi_LENGTH_FIELD_SIZE__)
#define __mrfi_SRC_ADDR_OFS__           (__mrfi_DST_ADDR_OFS__ + __mrfi_ADDR_SIZE__)
#define __mrfi_PAYLOAD_OFS__            (__mrfi_SRC_ADDR_OFS__ + __mrfi_ADDR_SIZE__)

#define __mrfi_HEADER_SIZE__            (2 * __mrfi_ADDR_SIZE__)
#define __mrfi_FRAME_OVERHEAD_SIZE__    (__mrfi_LENGTH_FIELD_SIZE__ + __mrfi_HEADER_SIZE__)

#define __mrfi_GET_PAYLOAD_LEN__(p)     ((p)->frame[__mrfi_LENGTH_FIELD_OFS__] - __mrfi_HEADER_SIZE__)
#define __mrfi_SET_PAYLOAD_LEN__(p,x)   st( (p)->frame[__mrfi_LENGTH_FIELD_OFS__] = x + __mrfi_HEADER_SIZE__; )

#endif


/* ------------------------------------------------------------------------------------------------
 *                                   Radio Family Commonality
 * ------------------------------------------------------------------------------------------------
 */
#define __mrfi_P_DST_ADDR__(p)          (&((p)->frame[__mrfi_DST_ADDR_OFS__]))
#define __mrfi_P_SRC_ADDR__(p)          (&((p)->frame[__mrfi_SRC_ADDR_OFS__]))
#define __mrfi_P_PAYLOAD__(p)           (&((p)->frame[__mrfi_PAYLOAD_OFS__]))


/* ************************************************************************************************
 *                                   Compile Time Integrity Checks
 * ************************************************************************************************
 */

/* verify that only one supported radio is selected */
#define MRFI_NUM_SUPPORTED_RADIOS_SELECTED   ((defined MRFI_SI4438) + \
                                              (defined MRFI_SI4463))
#if (MRFI_NUM_SUPPORTED_RADIOS_SELECTED == 0)
#error "ERROR: A valid radio is not selected."
#elif (MRFI_NUM_SUPPORTED_RADIOS_SELECTED > 1)
#error "ERROR: More than one radio is selected."
#endif

/* verify that a radio family is selected */
#if ((!defined MRFI_RADIO_FAMILY1) && \
    (!defined MRFI_RADIO_FAMILY2))
#error "ERROR: A radio family has not been assigned."
#endif


/**************************************************************************************************
 */
#endif


