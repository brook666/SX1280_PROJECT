/***************************************************************************
** 文件名:  nwk_ioctl.c
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  网络层的IO操作
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
#include "nwk_frame.h"
#include "nwk.h"
#include "nwk_ioctl.h"
#include "nwk_globals.h"
#include "nwk_security.h"

#ifdef ACCESS_POINT
#include "nwk_join.h"
#endif

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

/******************************************************************************
 * LOCAL FUNCTIONS
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */
extern  persistentContext_t sPersistInfo;
/******************************************************************************
 * GLOBAL FUNCTIONS
 */


/******************************************************************************
 * @fn          nwk_rawSend
 *
 * @brief       Builds an outut frame based on information provided by the
 *              caller. This function allows a raw transmission to the target
 *              if the network address is known. this function is used a lot
 *              to support NWK applications.
 *
 * input parameters
 * @param   info    - pointer to strcuture containing info on how to build
 *                    the outgoing frame.
 * output parameters
 *
 * @return         SMPL_SUCCESS
 *                 SMPL_NOMEM       - no room in output frame queue
 *                 SMPL_TX_CCA_FAIL - CCA failure
 */
smplStatus_t nwk_rawSend(ioctlRawSend_t *info)
{
  frameInfo_t *pOutFrame;
  uint8_t      hops;

  /* If we know frame is going to or from the AP then we can reduce the hop
   * count.
   */
  switch (info->port)
  {
    case SMPL_PORT_JOIN:
    case SMPL_PORT_FREQ:
    case SMPL_PORT_MGMT:
      hops = MAX_HOPS_FROM_AP;
      break;

    default:
      hops = MAX_HOPS;
      break;
  }
  pOutFrame = nwk_buildFrame(info->port, info->msg, info->len, hops);
  if (pOutFrame)
  {
    memcpy(MRFI_P_DST_ADDR(&pOutFrame->mrfiPkt), info->addr, NET_ADDR_SIZE);
#ifdef SMPL_SECURE
    nwk_setSecureFrame(&pOutFrame->mrfiPkt, info->len, 0);
#endif  /* SMPL_SECURE */
    return nwk_sendFrame(pOutFrame, MRFI_TX_TYPE_CCA);
  }
  return SMPL_NOMEM;
}

/******************************************************************************
 * @fn          nwk_rawReceive
 *
 * @brief       Retriievs specified from from the input frame queue. Additional
 *              information such as source address and hop count may also be
 *              retrieved
 *
 * input parameters
 * @param   info    - pointer to structure containing info on what to retrieve
 *
 * output parameters - actually populated by nwk_retrieveFrame()
 *      info->msg      - application payload copied here
 *      info->len      - length of received application payload
 *      info->addr     - if non-NULL points to memory to be populated with
 *                       source address of retrieved frame.
 *      info->hopCount - if non-NULL points to memory to be populated with
 *                       hop count of retrieved frame.
 *
 * @return   Status of operation.
 */
smplStatus_t nwk_rawReceive(ioctlRawReceive_t *info)
{
  rcvContext_t rcv;

  rcv.type   = RCV_NWK_PORT;
  rcv.t.port = info->port;

  return nwk_retrieveFrame(&rcv, info->msg, &info->len, info->addr, &info->hopCount);
}

/******************************************************************************
 * @fn          nwk_radioControl
 *
 * @brief       Handle radio control functions.
 *
 * input parameters
 * @param   action   - radio operation to perform. currently suppoerted:
 *                         sleep/unsleep
 * output parameters
 *
 * @return   Status of operation.
 */
smplStatus_t nwk_radioControl(ioctlAction_t action, void *val)
{
  smplStatus_t rc = SMPL_SUCCESS;

  if (IOCTL_ACT_RADIO_SLEEP == action)
  {
    /* go to sleep mode. */
    MRFI_Ready();
    MRFI_Sleep();
  }
  else if (IOCTL_ACT_RADIO_AWAKE == action)
  {
    MRFI_WakeUp();

#if !defined( END_DEVICE )
    MRFI_RxOn();
#endif

  }
  else if (IOCTL_ACT_RADIO_SIGINFO == action)
  {
    ioctlRadioSiginfo_t *pSigInfo = (ioctlRadioSiginfo_t *)val;
    connInfo_t          *pCInfo   = nwk_getConnInfo(pSigInfo->lid);

    if (!pCInfo)
    {
      return SMPL_BAD_PARAM;
    }
    memcpy(&pSigInfo->sigInfo, &pCInfo->sigInfo, sizeof(pCInfo->sigInfo));
  }
  else if (IOCTL_ACT_RADIO_RSSI == action)
  {
    *((rssi_t *)val) = MRFI_Rssi();
  }
  else if (IOCTL_ACT_RADIO_RXON == action)
  {
    MRFI_RxOn();
  }
  else if (IOCTL_ACT_RADIO_RXIDLE == action)
  {
    MRFI_Ready();
  }
#ifdef EXTENDED_API
  else if (IOCTL_ACT_RADIO_SETPWR == action)
  {
    uint8_t idx;

    switch (*(ioctlLevel_t *)val)
    {
      case IOCTL_LEVEL_2:
        idx = 2;
        break;

      case IOCTL_LEVEL_1:
        idx = 1;
        break;

      case IOCTL_LEVEL_0:
        idx = 0;
        break;

      default:
        return SMPL_BAD_PARAM;
    }
    MRFI_SetRFPwr(idx);
    return SMPL_SUCCESS;
  }
#endif  /* EXTENDED_API */
  else
  {
    rc = SMPL_BAD_PARAM;
  }
  return rc;
}

/******************************************************************************
 * @fn          nwk_joinContext
 *
 * @brief       For Access Points we need a way to support changing the Join
 *              context. This will allow arbitration bewteen potentially nearby
 *              Access Points when a new device is joining.
 *
 * input parameters
 * @param   action  - Join context is either on or off.
 *
 * output parameters
 *
 * @return   Status of operation. Currently always succeeds.
 */
#ifdef ACCESS_POINT
smplStatus_t nwk_joinContext(ioctlAction_t action)
{
  nwk_setJoinContext((IOCTL_ACT_ON == action) ? JOIN_CONTEXT_ON : JOIN_CONTEXT_OFF);

  return SMPL_SUCCESS;
}
#endif

/******************************************************************************
 * @fn          nwk_deviceAddress
 *
 * @brief       Set or Get this device address. The Set must be done before
 *              SMPL_Init() for it to take effect. The Get is always legal but
 *              the value could be invalid if it is called before a valid set
 *              call is made.
 *
 * input parameters
 * @param   action  - Gte or Set
 * @param   addr    - pointer to address object containing value on Set
 *
 * output parameters
 * @param   addr    - pointer to address object to receive value on Get.
 *
 * @return   SMPL_SUCCESS
 *           SMPL_BAD_PARAM  Action request illegal or a Set request
 *                           was not respected.
 */
smplStatus_t nwk_deviceAddress(ioctlAction_t action, addr_t *addr)
{
  smplStatus_t rc = SMPL_BAD_PARAM;

  if (IOCTL_ACT_GET == action)
  {
    memcpy(addr, nwk_getMyAddress(), sizeof(addr_t));
    rc = SMPL_SUCCESS;
  }
  else if (IOCTL_ACT_SET == action)
  {
    if (nwk_setMyAddress(addr))
    {
      rc = SMPL_SUCCESS;
    }
  }

  return rc;
}

/******************************************************************************
 * @fn          nwk_deviceType
 *
 * @brief       Set or Get this device address. The Set must be done before
 *              SMPL_Init() for it to take effect. The Get is always legal but
 *              the value could be invalid if it is called before a valid set
 *              call is made.
 *
 * input parameters
 * @param   action  - Gte or Set
 * @param   addr    - pointer to address object containing value on Set
 *
 * output parameters
 * @param   addr    - pointer to address object to receive value on Get.
 *
 * @return   SMPL_SUCCESS
 *           SMPL_BAD_PARAM  Action request illegal or a Set request
 *                           was not respected.
 */
smplStatus_t nwk_deviceType(ioctlAction_t action, type_t *type)
{
  smplStatus_t rc = SMPL_BAD_PARAM;

  if (IOCTL_ACT_GET == action)
  {
    memcpy(type, nwk_getMyType(), sizeof(type_t));
    rc = SMPL_SUCCESS;
  }
  else if (IOCTL_ACT_SET == action)
  {
    if (nwk_setMyType(type))
    {
      rc = SMPL_SUCCESS;
    }
  }

  return rc;
}

/******************************************************************************
 * @fn          nwk_deviceIPAddress
 *
 * @brief       Set or Get this device address. The Set must be done before
 *              SMPL_Init() for it to take effect. The Get is always legal but
 *              the value could be invalid if it is called before a valid set
 *              call is made.
 *
 * input parameters
 * @param   action  - Gte or Set
 * @param   addr    - pointer to address object containing value on Set
 *
 * output parameters
 * @param   addr    - pointer to address object to receive value on Get.
 *
 * @return   SMPL_SUCCESS
 *           SMPL_BAD_PARAM  Action request illegal or a Set request
 *                           was not respected.
 */
smplStatus_t nwk_deviceIPAddress(ioctlAction_t action, ip_addr_t *addr)
{
  smplStatus_t rc = SMPL_BAD_PARAM;

  if (IOCTL_ACT_GET == action)
  {
    memcpy(addr, nwk_getMyIPAddress(), sizeof(ip_addr_t));
    rc = SMPL_SUCCESS;
  }
  else if (IOCTL_ACT_SET == action)
  {
    if (nwk_setMyIPAddress(addr))
    {
      rc = SMPL_SUCCESS;
    }
  }

  return rc;
}
/******************************************************************************
 * @fn          nwk_connectionControl
 *
 * @brief       Access to connection table. Currently supports only deleting
 *              a connection from the table.
 *
 * input parameters
 * @param   action  - Connection control action (only delete is curently valid).
 * @param   val     - pointer to Link ID of connection on which to operate.
 *
 * output parameters
 *
 * @return   SMPL_SUCCESS
 *           SMPL_BAD_PARAM  Action is not delete
 *                           Link ID is the UUD Link ID
 *                           No connection table info for Link ID
 */
smplStatus_t nwk_connectionControl(ioctlAction_t action, void *val)
{
  uint8_t i=0; 
  connInfo_t *pCInfo;
  linkID_t    lid = *((linkID_t *)val);

  if (IOCTL_ACT_DELETE != action)
  {
    return SMPL_BAD_PARAM;
  }
  pCInfo=nwk_getConnInfo(lid);
  if ((SMPL_LINKID_USER_UUD == lid) ||(!pCInfo))
  {
    return SMPL_BAD_PARAM;
  }

  /*调整被删除节点之后的节点的LID,LID大于该lid的均减一*/
  for(i=1;i<sPersistInfo.nextLinkID;i++)
  {
    pCInfo=nwk_getConnInfo(i);
		if(i == lid)
		{
			nwk_freeConnection(pCInfo);
			memset(pCInfo,0x0,sizeof(connInfo_t));
		}
		else if(i>lid)
		{
			if(pCInfo &&(pCInfo->connState == CONNSTATE_CONNECTED))
			{
				if((pCInfo->thisLinkID) > lid)
				{
					pCInfo->thisLinkID--;//之后的节点LID顺次减1
				}
			}			
		}
		else
		{
		}
  }	
	
	sPersistInfo.nextLinkID--;//要分配的下一个LID
//  sPersistInfo.nextJoinID--;
  sPersistInfo.net_ed_num--;//有效节点数.定义于nwk.h
  
  return SMPL_SUCCESS;
}


