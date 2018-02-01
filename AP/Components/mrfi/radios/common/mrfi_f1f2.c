/**************************************************************************************************
  Revised:        $Date: 2007-07-06 11:19:00 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

**************************************************************************************************/

/* ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 *   MRFI (Minimal RF Interface)
 *   Shared code between Radio Family 1 & Family 2
 * ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mrfi.h"
#include "mrfi_defs.h"
#include "mrfi_f1f2.h"
#include "bsp.h"
#include "bsp_macros.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Common
 * ------------------------------------------------------------------------------------------------
 */
#ifdef MRFI_ASSERTS_ARE_ON
#define RX_FILTER_ADDR_INITIAL_VALUE  0xFF
#endif


/* ------------------------------------------------------------------------------------------------
 *                                       Radio Abstraction
 * ------------------------------------------------------------------------------------------------
 */

/* ----- Radio Family 1 ----- */
#if (defined MRFI_RADIO_FAMILY1)
#include "../family1/mrfi_spi.h"
#define MRFI_WRITE_REGISTER(x,y)      st( x = y; )

/* ----- Radio Family 2 ----- */
#elif (defined MRFI_RADIO_FAMILY2)
#define MRFI_WRITE_REGISTER(x,y)      st( x = y; )

/* unknown or missing radio family */
#else
#error "ERROR: Expected radio family not specified.  Most likely a project issue."
#endif


/* ------------------------------------------------------------------------------------------------
 *                                    Global Constants
 * ------------------------------------------------------------------------------------------------
 */
const uint8_t mrfiBroadcastAddr[] = { 0xFF, 0xFF, 0xFF, 0xFF };
const uint8_t mrfiRemoterAddr[]   = REMOTER_ADDRESS;

/* verify number of table entries matches the corresponding #define */
BSP_STATIC_ASSERT(MRFI_ADDR_SIZE == ((sizeof(mrfiBroadcastAddr)/sizeof(mrfiBroadcastAddr[0])) * sizeof(mrfiBroadcastAddr[0])));


/* ------------------------------------------------------------------------------------------------
 *                                    Local Constants
 * ------------------------------------------------------------------------------------------------
 */

/*
 *  Logical channel table - this table translates logical channel into
 *  actual radio channel number.  Channel 0, the default channel, is
 *  determined by the channel exported from SmartRF Studio.  The other
 *  table entries are derived from that default.  Each derived channel is
 *  masked with 0xFF to prevent generation of an illegal channel number.
 *
 *  This table is easily customized.  Just replace or add entries as needed.
 *  If the number of entries changes, the corresponding #define must also
 *  be adjusted.  It is located in mrfi_defs.h and is called __mrfi_NUM_LOGICAL_CHANS__.
 *  The static assert below ensures that there is no mismatch.
 */
#if defined( MRFI_SI4438 )
static const uint8_t mrfiLogicalChanTable[] =
{
  0,
  1,
  2,
  3
};
#elif defined(MRFI_SI4463)
static const uint8_t mrfiLogicalChanTable[] =
{
  SMARTRF_SETTING_CHANNR,
  40,
  60,
  80
};
#else
#error "ERROR: A valid radio is not specified."
#endif

/* verify number of table entries matches the corresponding #define */
BSP_STATIC_ASSERT(__mrfi_NUM_LOGICAL_CHANS__ == ((sizeof(mrfiLogicalChanTable)/sizeof(mrfiLogicalChanTable[0])) * sizeof(mrfiLogicalChanTable[0])));

/*
 *  RF Power setting table - this table translates logical power value
 *  to radio register setting.  The logical power value is used directly
 *  as an index into the power setting table. The values in the table are
 *  from low to high. The default settings set 3 values: -20 dBm, -10 dBm,
 *  and 0 dBm. The default at startup is the highest value. Note that these
 *  are approximate depending on the radio. Information is taken from the
 *  data sheet.
 *
 *  This table is easily customized.  Just replace or add entries as needed.
 *  If the number of entries changes, the corresponding #define must also
 *  be adjusted.  It is located in mrfi_defs.h and is called __mrfi_NUM_POWER_SETTINGS__.
 *  The static assert below ensures that there is no mismatch.
 */
#if defined( MRFI_SI4438 )
static const uint8_t mrfiRFPowerTable[] =
{
  0x01,    //-40dbm
  0x07,   //0dbm
  0x0d,   //5dbm
  0x15,   //10dbm
  0x28,   //15dbm
  0x7f    //20dbm
};

#elif defined( MRFI_SI4463 )
static const uint8_t mrfiRFPowerTable[] =
{
  0x07,   //0dbm
  0x0d,   //5dbm
  0x15,   //10dbm
  0x28,   //15dbm
  0x7f    //20dbm
};
#endif

/* verify number of table entries matches the corresponding #define */
BSP_STATIC_ASSERT(__mrfi_NUM_POWER_SETTINGS__ == ((sizeof(mrfiRFPowerTable)/sizeof(mrfiRFPowerTable[0])) * sizeof(mrfiRFPowerTable[0])));


/* ------------------------------------------------------------------------------------------------
 *                                       Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static uint8_t mrfiRxFilterEnabled=0;
static uint8_t mrfiRxFilterAddr[MRFI_ADDR_SIZE] = { RX_FILTER_ADDR_INITIAL_VALUE };

/**************************************************************************************************
 * @fn          MRFI_SetLogicalChannel
 *
 * @brief       Set logical channel.
 *
 * @param       chan - logical channel number
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_SetLogicalChannel(uint8_t chan)
{
  /* logical channel is not valid? */
  MRFI_ASSERT( chan < MRFI_NUM_LOGICAL_CHANS );

  /* make sure radio is in ready mode before changing power levels */
  MRFI_Ready(); 
  
  MRFI_SetChannelNum(chan);

  /* turn radio back on if it was on before channel change */
  if(mrfiRadioState == MRFI_RADIO_STATE_RX)
  {
    MRFI_RxOn();
  }
}

/**************************************************************************************************
 * @fn          MRFI_SetRFPwr
 *
 * @brief       Set RF Output power level.
 *
 * @param       idx - index into power table.
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_SetRFPwr(uint8_t idx)
{
  /* is power level specified valid? */
  MRFI_ASSERT( idx < MRFI_NUM_POWER_SETTINGS );

  /* make sure radio is in ready mode before changing power levels */
  MRFI_Ready(); 
 
  Mrfi_SetPaPower( mrfiRFPowerTable[idx] );
  
  /* turn radio back on if it was on before power level change */
  if(mrfiRadioState == MRFI_RADIO_STATE_RX)
  {
    MRFI_RxOn();
  }
}

/**************************************************************************************************
 * @fn          MRFI_SetRxAddrFilter
 *
 * @brief       Set the address used for filtering received packets.
 *
 * @param       pAddr - pointer to address to use for filtering
 *
 * @return      zero     : successfully set filter address
 *              non-zero : illegal address
 **************************************************************************************************
 */
uint8_t MRFI_SetRxAddrFilter(uint8_t * pAddr)
{
  /*
   *  If first byte of filter address match fir byte of broadcast address,
   *  there is a conflict with hardware filtering.
   */
//  if (pAddr[0] == mrfiBroadcastAddr[0])
//  {
//    /* unable to set filter address */
//    return( 1 );
//  }

  /*
   *  Set the hardware address register.  The hardware address filtering only recognizes
   *  a single byte but this does provide at least some automatic hardware filtering.
   */
  //MRFI_WRITE_REGISTER( ADDR, pAddr[0] );

  /* save a copy of the filter address */
  {
    uint8_t i;

    for (i=0; i<MRFI_ADDR_SIZE; i++)
    {
      mrfiRxFilterAddr[i] = pAddr[i];
    }
  }

  /* successfully set filter address */
  return( 0 );
}


/**************************************************************************************************
 * @fn          MRFI_EnableRxAddrFilter
 *
 * @brief       Enable received packet filtering.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_EnableRxAddrFilter(void)
{
  /* filter address must be set before enabling filter */
  //MRFI_ASSERT(mrfiRxFilterAddr[0] != mrfiBroadcastAddr[0]); 

  /* set flag to indicate filtering is enabled */
  mrfiRxFilterEnabled = 1;

  /* enable hardware filtering on the radio */
 // MRFI_WRITE_REGISTER( PKTCTRL1, PKTCTRL1_ADDR_FILTER_ON );
}


/**************************************************************************************************
 * @fn          MRFI_DisableRxAddrFilter
 *
 * @brief       Disable received packet filtering.
 *
 * @param       pAddr - pointer to address to test for filtering
 *
 * @return      none
 **************************************************************************************************
 */
void MRFI_DisableRxAddrFilter(void)
{
  /* clear flag that indicates filtering is enabled */
  mrfiRxFilterEnabled = 0;

  /* disable hardware filtering on the radio */
 // MRFI_WRITE_REGISTER( PKTCTRL1, PKTCTRL1_ADDR_FILTER_OFF );
}


/**************************************************************************************************
 * @fn          MRFI_RxAddrIsFiltered
 *
 * @brief       Determine if address is filtered.
 *
 * @param       none
 *
 * @return      zero     : address is not filtered
 *              non-zero : address is filtered
 **************************************************************************************************
 */
uint8_t MRFI_RxAddrIsFiltered(uint8_t * pAddr)
{
  uint8_t i;
  uint8_t addrByte;
  uint8_t filterAddrMatches;
  uint8_t broadcastAddrMatches;

  /* first check to see if filtering is even enabled */
  if (!mrfiRxFilterEnabled)
  {
    /*
     *  Filtering is not enabled, so by definition the address is
     *  not filtered.  Return zero to indicate address is not filtered.
     */
    return( 0 );
  }

  /* clear address byte match counts */
  filterAddrMatches    = 0;
  broadcastAddrMatches = 0;

  /* loop through address to see if there is a match to filter address of broadcast address */
  for (i=0; i<MRFI_ADDR_SIZE; i++)
  {
    /* get byte from address to check */
    addrByte = pAddr[i];

    /* compare byte to filter address byte */
    if (addrByte == mrfiRxFilterAddr[i])
    {
      filterAddrMatches++;
    }
    if (addrByte == mrfiBroadcastAddr[i])
    {
      broadcastAddrMatches++;
    }
  }

  /*
   *  If address is *not* filtered, either the "filter address match count" or
   *  the "broadcast address match count" will equal the total number of bytes
   *  in the address.
   */
  if ((broadcastAddrMatches == MRFI_ADDR_SIZE) || (filterAddrMatches == MRFI_ADDR_SIZE))
  {
    /* address *not* filtered, return zero */
    return( 0 );
  }
  else
  {
    /* address filtered, return non-zero */
    return( 1 );
  }
}


/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */

/*
 *  These asserts happen if there is extraneous compiler padding of arrays.
 *  Modify compiler settings for no padding, or, if that is not possible,
 *  comment out the offending asserts.
 */
BSP_STATIC_ASSERT(sizeof(mrfiLogicalChanTable) == ((sizeof(mrfiLogicalChanTable)/sizeof(mrfiLogicalChanTable[0])) * sizeof(mrfiLogicalChanTable[0])));
BSP_STATIC_ASSERT(sizeof(mrfiBroadcastAddr) == ((sizeof(mrfiBroadcastAddr)/sizeof(mrfiBroadcastAddr[0])) * sizeof(mrfiBroadcastAddr[0])));


/**************************************************************************************************
*/


