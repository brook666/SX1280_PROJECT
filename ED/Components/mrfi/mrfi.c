/**************************************************************************************************
  Revised:        $Date: 2007-07-06 11:19:00 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

**************************************************************************************************/

/* ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 *   MRFI (Minimal RF Interface)
 *   Top-level code file.
 * ~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mrfi_defs.h"

/* ------------------------------------------------------------------------------------------------
 *                                       C Code Includes
 * ------------------------------------------------------------------------------------------------
 */

/* ----- Radio Family 1 or Radio Family 2----- */
#if (defined MRFI_RADIO_FAMILY1 || \
     defined MRFI_RADIO_FAMILY2)
#include "radios/family1/mrfi_radio.c"
#include "radios/family1/mrfi_spi.c"
#include "radios/common/mrfi_f1f2.c"
#include "bsp_external/mrfi_board.c"

/* ----- Radio Family 3 ----- */
#elif (defined MRFI_RADIO_FAMILY3)
#include "radios/family3/mrfi_radio.c"
#include "radios/common/mrfi_f1f2.c"

#else
#error "ERROR: Radio family is not defined."
#endif


/**************************************************************************************************
 */


