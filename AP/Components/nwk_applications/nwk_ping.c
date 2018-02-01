/***************************************************************************
** 文件名:  nwk_ping.c
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  网络层ping命令相关函数
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
#include "nwk_ping.h"
#include "nwk_globals.h"
#include "nwk_api.h"
#include "nwk_freq.h"
#include "nwk_security.h"

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

static volatile uint8_t sTid = 0;

/******************************************************************************
 * LOCAL FUNCTIONS
 */
static void smpl_send_ping_reply(mrfiPacket_t *);
static void handlePingRequest(mrfiPacket_t *);

/******************************************************************************
 * GLOBAL VARIABLES
 */

/******************************************************************************
 * GLOBAL FUNCTIONS
 */

/******************************************************************************
 * @fn          nwk_pingInit
 *
 * @brief       Initialize Ping application.
 *
 * input parameters
 *
 * output parameters
 *
 * @return   void
 */
void nwk_pingInit(void)
{
  sTid = MRFI_RandomByte();

  return;
}

/******************************************************************************
 * @fn          nwk_ping
 *
 * @brief       Called from the application level to ping a peer. A small
 *              payload is sent that includes a tid to detect correct reply.
 *              Caller does not supply payload.
 *
 * input parameters
 * @param   lid     - Link ID representing peer to ping
 *
 * output parameters
 *
 * @return   SMPL_SUCCESS   valid reply received
 *           SMPL_TIMEOUT   no valid reply received
 *           SMPL_NO_CHANNEL  no channels returned on a scan
 */
smplStatus_t nwk_ping(linkID_t lid)
{
  connInfo_t  *pCInfo   = nwk_getConnInfo(lid);
  smplStatus_t rc       = SMPL_BAD_PARAM;
  uint8_t      done     = 0;
  uint8_t      repeatIt = 2;
  uint8_t      msg[MAX_PING_APP_FRAME];
  uint8_t      radioState = MRFI_GetRadioState();
  union
  {
    ioctlRawSend_t    send;
    ioctlRawReceive_t recv;
  } ioctl_info;

  if (!pCInfo || (SMPL_LINKID_USER_UUD == lid))
  {
    /* either link ID bogus or tried to ping the unconnected user datagram link ID. */
    return rc;
  }

  do
  {
#if defined(FREQUENCY_AGILITY) && !defined(ACCESS_POINT)
    uint8_t     i, numChan;
    freqEntry_t channels[NWK_FREQ_TBL_SIZE];

    if (repeatIt == 2)
    {
      /* If FA enabled, first time through set up so that the 'for'
       * loop checks the current channel. This saves time (no scan)
       * and is very likely to succeed. Populate the proper strucure.
       */
      SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_GET, channels);
      numChan = 1;
    }
    else
    {
      /* If we get here we must scan for the channel we're now on */
      if (!(numChan=nwk_scanForChannels(channels)))
      {
        return SMPL_NO_CHANNEL;
      }
    }
    /* Either we scan next time through or we're done */
    repeatIt--;

    /* this loop Pings on each channel (probably only 1) looking
     * for peer.
     */
    for (i=0; i<numChan && !done; ++i)
    {
      nwk_setChannel(&channels[i]);
#else
  {
      repeatIt = 0;
#endif  /* defined(FREQUENCY_AGILITY) && !defined(ACCESS_POINT) */

      ioctl_info.send.addr = (addr_t *)pCInfo->peerAddr;
      ioctl_info.send.msg  = msg;
      ioctl_info.send.len  = sizeof(msg);
      ioctl_info.send.port = SMPL_PORT_PING;

      /* fill in msg */
      msg[PB_REQ_OS] = PING_REQ_PING;
      msg[PB_TID_OS] = sTid;

      SMPL_Ioctl(IOCTL_OBJ_RAW_IO, IOCTL_ACT_WRITE, &ioctl_info.send);

      ioctl_info.recv.port = SMPL_PORT_PING;
      ioctl_info.recv.msg  = msg;
      ioctl_info.recv.addr = 0;

      NWK_CHECK_FOR_SETRX(radioState);
      NWK_REPLY_DELAY();
      NWK_CHECK_FOR_RESTORE_STATE(radioState);

     if (SMPL_SUCCESS == SMPL_Ioctl(IOCTL_OBJ_RAW_IO, IOCTL_ACT_READ, &ioctl_info.recv))
//       if(time_50_8_ms_count!=20)
      {
        repeatIt = 0;
        done     = 1;
        sTid++;   /* guard against duplicates */
      }
    }
  } while (repeatIt);

  return done ? SMPL_SUCCESS : SMPL_TIMEOUT;

}

/******************************************************************************
 * @fn          smpl_send_ping_reply
 *
 * @brief       Send a reply to a ping request.
 *
 * input parameters
 * @param   frame     - pointer to frame containing request
 *
 * output parameters
 *
 * @return   void
 */
static void smpl_send_ping_reply(mrfiPacket_t *frame)
{
  frameInfo_t *pOutFrame;

  /* Build the reply frame. The application payload is the one included in the
   * received frame payload.
   */
  pOutFrame = nwk_buildFrame(SMPL_PORT_PING, \
                             MRFI_P_PAYLOAD(frame)+F_APP_PAYLOAD_OS, \
                             MRFI_GET_PAYLOAD_LEN(frame)-F_APP_PAYLOAD_OS, \
                             MAX_HOPS);
  if (pOutFrame)
  {
    /* destination address is the source adddress of the received frame. */
    memcpy(MRFI_P_DST_ADDR(&pOutFrame->mrfiPkt), MRFI_P_SRC_ADDR(frame), NET_ADDR_SIZE);

    /* turn on the reply bit in the application payload */
    *(MRFI_P_PAYLOAD(&pOutFrame->mrfiPkt)+F_APP_PAYLOAD_OS+PB_REQ_OS) |= NWK_APP_REPLY_BIT;
#ifdef SMPL_SECURE
    nwk_setSecureFrame(&pOutFrame->mrfiPkt, MRFI_GET_PAYLOAD_LEN(frame)-F_APP_PAYLOAD_OS, 0);
#endif  /* SMPL_SECURE */
    nwk_sendFrame(pOutFrame, MRFI_TX_TYPE_FORCED);
  }
}

/******************************************************************************
 * @fn          nwk_processPing
 *
 * @brief       Ping network application frame handler.
 *
 * input parameters
 * @param   frame   - pointer to frame in question
 *
 * output parameters
 *
 * @return    Keep frame for application, release frame, or replay frame.
 */
fhStatus_t nwk_processPing(mrfiPacket_t *frame)
{
  fhStatus_t   rc;
  uint8_t      replyType;

  /* If we sent this then this is the reply. Validate the
   * packet for reception by client app. If we didn't send
   * it then we are the target. Send the reply.
   */
  replyType = nwk_isValidReply(frame, sTid, PB_REQ_OS, PB_TID_OS);
  if (SMPL_MY_REPLY == replyType)
  {
    /* It's a match and it's a reply. Validate the received packet by
     * returning a 1 so it can be received by the client app.
     */
    MRFI_PostKillSem();
    rc = FHS_KEEP;
  }
#if !defined( END_DEVICE )
  else if (SMPL_A_REPLY == replyType)
  {
    /* no match. If I'm not an ED this is a reply that should be passed on. */
    rc = FHS_REPLAY;
  }
#endif  /* !END_DEVICE */
  else
  {
    /* No, we didn't send it. Send reply assuming it's a Ping intended for us. */
    handlePingRequest(frame);

    rc = FHS_RELEASE;
  }

  return rc;
}

/******************************************************************************
 * @fn          handlePingRequest
 *
 * @brief       Dispatches handler for specfic Ping request
 *
 * input parameters
 *
 * @param   frame - Ping frame received
 *
 * output parameters
 *
 * @return   void
 */
static void handlePingRequest(mrfiPacket_t *frame)
{
  switch (*(MRFI_P_PAYLOAD(frame)+F_APP_PAYLOAD_OS))
  {
    case PING_REQ_PING:
      smpl_send_ping_reply(frame);
      break;

    default:
      break;
  }

  return;
}



