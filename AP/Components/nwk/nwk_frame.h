/***************************************************************************
** 文件名:  nwk_frame.h
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  网络层帧处理相关函数
**          
** 版  本:  1.0
***************************************************************************/


#ifndef NWK_FRAME_H
#define NWK_FRAME_H

#include <stdint.h>
#include "mrfi.h"
#include "nwk_types.h"

/* Frame field defines and masks. Mask name must be field name with '_MSK' appended
 * so the GET and PUT macros work correctly -- they use token pasting. Offset values
 * are with respect to the MRFI payload and not the entire frame.
 */
#define F_PORT_OS         0
#define F_PORT_OS_MSK     (0x3F)
#define F_ENCRYPT_OS      0
#define F_ENCRYPT_OS_MSK  (0x40)
#define F_FWD_FRAME       0
#define F_FWD_FRAME_MSK   (0x80)
#define F_RX_TYPE         1
#define F_RX_TYPE_MSK     (0x40)
#define F_ACK_REQ         1
#define F_ACK_REQ_MSK     (0x80)
#define F_ACK_RPLY        1
#define F_ACK_RPLY_MSK    (0x08)
#define F_TX_DEVICE       1
#define F_TX_DEVICE_MSK   (0x30)
#define F_HOP_COUNT       1
#define F_HOP_COUNT_MSK   (0x07)
#define F_TRACTID_OS      2
#define F_TRACTID_OS_MSK  (0xFF)


#define SMPL_NWK_HDR_SIZE 3

#ifdef SMPL_SECURE

#define F_SECURE_OS       3

#define F_SEC_CTR_OS      3       /* counter hint */
#define F_SEC_CTR_OS_MSK  (0xFF)
#define F_SEC_ICHK_OS     4       /* Message integrity check */
#define F_SEC_ICHK_OS_MSK (0xFF)
#define F_SEC_MAC_OS      5       /* Message authentication code */
#define F_SEC_MAC_OS_MSK  (0xFF)

#else

#define F_SECURE_OS       0

#endif  /* SMPL_SECURE */

#define F_APP_PAYLOAD_OS  (SMPL_NWK_HDR_SIZE+F_SECURE_OS)

/* sub field details. they are in the correct bit locations (already shifted) */
#define F_RX_TYPE_USER_CTL       0x00    /* does not poll... */
#define F_RX_TYPE_POLLS          0x40    /* polls for held messages */

#define F_ACK_REQ_TYPE           0x80
#define F_ACK_RPLY_TYPE          0x08
#define F_FRAME_FWD_TYPE         0x80
#define F_FRAME_ENCRYPT_TYPE     0x40

/* device type fields */
#define F_TX_DEVICE_ED           0x00    /* End Device */
#define F_TX_DEVICE_RE           0x10    /* Range Extender */
#define F_TX_DEVICE_AP           0x20    /* Access Point */

/* macro to get a field from a frame buffer */
#define GET_FROM_FRAME(b,f)  ((b)[f] & (f##_MSK))

/* Macro to put a value 'v' into a frame buffer 'b'. 'v' value must already be shifted
 * if necessary. 'b' is a byte array
 */
#define PUT_INTO_FRAME(b,f,v)  do {(b)[f] = ((b)[f] & ~(f##_MSK)) | (v); } while(0)


/*       ****   frame information objects
 * info kept on each frame object
 */
#define   FI_AVAILABLE         0   /* entry available for use */
#define   FI_INUSE_UNTIL_DEL   1   /* in use. will be explicitly reclaimed */
#define   FI_INUSE_UNTIL_TX    2   /* in use. will be reclaimed after Tx */
#define   FI_INUSE_UNTIL_FWD   3   /* in use until forwarded by AP */
#define   FI_INUSE_TRANSITION  4   /* being retrieved. do not delete in Rx ISR thread. */

typedef struct
{
  uint8_t   rssi;
  uint8_t   lqi;
} sigInfo_t;

typedef struct
{
  volatile uint8_t      fi_usage;
           uint8_t      orderStamp;
           mrfiPacket_t mrfiPkt;
} frameInfo_t;


/* prototypes */
frameInfo_t  *nwk_buildFrame(uint8_t, uint8_t *msg, uint8_t len, uint8_t hops);
#ifdef APP_AUTO_ACK
frameInfo_t  *nwk_buildAckReqFrame(uint8_t, uint8_t *, uint8_t, uint8_t, volatile uint8_t *);
#endif
void          nwk_receiveFrame(void);
void          nwk_frameInit(uint8_t (*)(linkID_t,uint8_t));
smplStatus_t  nwk_retrieveFrame(rcvContext_t *, uint8_t *, uint8_t *, addr_t *, uint8_t *);
smplStatus_t  nwk_sendFrame(frameInfo_t *, uint8_t txOption);
frameInfo_t  *nwk_getSandFFrame(mrfiPacket_t *, uint8_t);
uint8_t       nwk_getMyRxType(void);
void          nwk_SendEmptyPollRspFrame(mrfiPacket_t *);
#ifdef APP_AUTO_ACK
void          nwk_sendAckReply(mrfiPacket_t *, uint8_t);
#endif

#ifndef END_DEVICE
/* only APs and REs repeat frames */
void  nwk_replayFrame(frameInfo_t *);
void nwk_replayRmoterFrame(frameInfo_t *pFrameInfo);
uint8_t nwk_getFrameReceiveCount(void);
#endif


#endif
