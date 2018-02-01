/***************************************************************************
** 文件名:  nwk_globals.c
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  网络层全局变量
**          
** 版  本:  1.0
***************************************************************************/


/******************************************************************************
 * INCLUDES
 */
#include <string.h>
#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_globals.h"

/******************************************************************************
 * MACROS
 */

/******************************************************************************
 * CONSTANTS AND DEFINES
 */

/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */
static const addr_t   sMyROMAddress = THIS_DEVICE_ADDRESS;
static addr_t         sAPAddress;
static addr_t         sMyRAMAddress;
static uint8_t        sRAMAddressIsSet = 0;

static const type_t   sMyROMType = THIS_DEVICE_TYPE;
static type_t         sMyRAMType;
static uint8_t        sRAMTypeIsSet = 0;

static ip_addr_t      sMyIPAddress ;
/* Version number set as a 4 byte quantity. Each byte is a revision number
 * in the form w.x.y.z. The subfields are each limited to values 0x0-0xFF.
 */
static const smplVersionInfo_t sVersionInfo = {
                                                0x02,  /* protocol version */
                                                0x01,  /* major revision number */
                                                0x01,  /* minor revision number */
                                                0x01,  /* maintenance release number */
                                                0x00   /* special release */
                                               };

/******************************************************************************
 * LOCAL FUNCTIONS
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */

/******************************************************************************
 * GLOBAL FUNCTIONS
 */

/******************************************************************************
 * @fn          nwk_globalsInit
 *
 * @brief       Initialization of global symbols
 *
 * input parameters
 *
 * output parameters
 *
 * @return   void
 */
void nwk_globalsInit(void)
{

  memset(&sAPAddress, 0x00, sizeof(addr_t));
  
  /* populate RAM address from ROM default if it hasn't laready been set
   * using the IOCTL interface.
   */
  if (!sRAMAddressIsSet)
  {
    memcpy(&sMyRAMAddress, &sMyROMAddress, sizeof(addr_t));
    sRAMAddressIsSet = 1;  /* RAM address is now valid */
  }
  /* populate RAM type from ROM default if it hasn't laready been set
   * using the IOCTL interface.
   */
  if (!sRAMTypeIsSet)
  {
    memcpy(&sMyRAMType, &sMyROMType, sizeof(type_t));
    sRAMTypeIsSet = 1;  /* RAM type is now valid */
  }

  /*set my IP*/
  sMyIPAddress.edAddr[0] = 0x0;
  sMyIPAddress.edAddr[1] = 0x0;
  sMyIPAddress.apAddr[0] = sMyRAMAddress.addr[0];
  sMyIPAddress.apAddr[1] = sMyRAMAddress.addr[1];
  
  return;
}

/******************************************************************************
 * @fn          nwk_getMyAddress
 *
 * @brief       Return a pointer to my address. It comes either from ROM as
 *              set in the configuration file or it was set using the IOCTL
 *              interface -- probably from random bytes.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   pointer to a constant address type object.
 */
addr_t const *nwk_getMyAddress(void)
{
  /* This call supports returning a valid pointer before either the
   * initialization or external setting of the address. But caller needs
   * to be careful -- if this routine is called immediately it will return
   * the ROM address. If the application then sets the address using the
   * IOCTL before doing the SMPL_Init() the original pointer is no longer
   * valid as it points to the wrong address.
   */
  return sRAMAddressIsSet ? &sMyRAMAddress : &sMyROMAddress;
}

/******************************************************************************
 * @fn          nwk_getMyType
 *
 * @brief       Return a pointer to my address. It comes either from ROM as
 *              set in the configuration file or it was set using the IOCTL
 *              interface -- probably from random bytes.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   pointer to a constant address type object.
 */
type_t const *nwk_getMyType(void)
{
  /* This call supports returning a valid pointer before either the
   * initialization or external setting of the address. But caller needs
   * to be careful -- if this routine is called immediately it will return
   * the ROM address. If the application then sets the address using the
   * IOCTL before doing the SMPL_Init() the original pointer is no longer
   * valid as it points to the wrong address.
   */
  return sRAMTypeIsSet ? &sMyRAMType : &sMyROMType;
}

/******************************************************************************
 * @fn          nwk_getFWVersion
 *
 * @brief       Return a pointer to the current firmware version string.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   pointer to a constant uint16_t object.
 */
uint8_t const *nwk_getFWVersion()
{
  return sVersionInfo.fwVerString;
}

/******************************************************************************
 * @fn          nwk_getProtocolVersion
 *
 * @brief       Return the current protocol version.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   Protocol version.
 */
uint8_t nwk_getProtocolVersion(void)
{
  return sVersionInfo.protocolVersion;
}

/******************************************************************************
 * @fn          nwk_setMyAddress
 *
 * @brief       Set my address object if it hasn't already been set. This call
 *              is referenced by the IOCTL support used to change the device
 *              address. It is effective only if the address has not already
 *              been set.
 *
 * input parameters
 *
 * @param   addr  - pointer to the address object to be used to set my address.
 *
 * output parameters
 *
 * @return   Returns non-zero if request is respected, otherwise returns 0.
 */
uint8_t nwk_setMyAddress(addr_t *addr)
{
  uint8_t rc = 0;

  if (!sRAMAddressIsSet)
  {
    memcpy(&sMyRAMAddress, addr, sizeof(addr_t));
    sRAMAddressIsSet = 1;  /* RAM address is now valid */
    rc = 1;
  }

  return rc;
}

/******************************************************************************
 * @fn          nwk_setMyType
 *
 * @brief       Set my address object if it hasn't already been set. This call
 *              is referenced by the IOCTL support used to change the device
 *              address. It is effective only if the address has not already
 *              been set.
 *
 * input parameters
 *
 * @param   addr  - pointer to the address object to be used to set my address.
 *
 * output parameters
 *
 * @return   Returns non-zero if request is respected, otherwise returns 0.
 */
uint8_t nwk_setMyType(type_t *type)
{
  uint8_t rc = 0;

  if (!sRAMTypeIsSet)
  {
    memcpy(&sMyRAMType, type, sizeof(type_t));
    sRAMTypeIsSet = 1;  /* RAM address is now valid */
    rc = 1;
  }

  return rc;
}



/******************************************************************************
 * @fn          nwk_setAPAddress
 *
 * @brief       Set the AP's address. Called after the AP address is gleaned
 *              from the Join reply.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   void
 */
void nwk_setAPAddress(addr_t *addr)
{

  memcpy((void *)&sAPAddress, (void *)addr, NET_ADDR_SIZE);

  return;
}

/******************************************************************************
 * @fn          nwk_getAPAddress
 *
 * @brief       Get the AP's address.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   Pointer to a constant address object or null if the address has not
 *           yet been set.
 */
addr_t const *nwk_getAPAddress(void)
{
  addr_t addr;

  memset(&addr, 0x0, sizeof(addr));

  return !memcmp(&sAPAddress, &addr, NET_ADDR_SIZE) ? 0 : &sAPAddress;
}

/******************************************************************************
 * @fn          nwk_getBCastAddress
 *
 * @brief       Get the network broadcast address.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   Pointer to a constant address object.
 */
addr_t const *nwk_getBCastAddress(void)
{
  return (addr_t const *)mrfiBroadcastAddr;
}


/******************************************************************************
 * @fn          nwk_getRemoterAddress
 *
 * @brief       Get the network remoter address.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   Pointer to a constant address object.
 */
addr_t const *nwk_getRemoterAddress(void)
{
  return (addr_t const *)mrfiRemoterAddr;
}


/******************************************************************************
 * @fn          nwk_getMyIPAddress
 *
 * @brief       Return a pointer to my IP address. It comes either from ROM as
 *              set in the configuration file or it was set using the IOCTL
 *              interface -- probably from random bytes.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   pointer to a constant address type object.
 */
ip_addr_t const *nwk_getMyIPAddress(void)
{
  return &sMyIPAddress;
}


/******************************************************************************
 * @fn          nwk_setMyIPAddress
 *
 * @brief       Set my address object if it hasn't already been set. This call
 *              is referenced by the IOCTL support used to change the device
 *              address. It is effective only if the address has not already
 *              been set.
 *
 * input parameters
 *
 * @param   addr  - pointer to the address object to be used to set my address.
 *
 * output parameters
 *
 * @return   Returns non-zero if request is respected, otherwise returns 0.
 */
uint8_t nwk_setMyIPAddress(ip_addr_t *IPAddr)
{

  memcpy(&sMyIPAddress, IPAddr, sizeof(ip_addr_t));

  return 1;
}


