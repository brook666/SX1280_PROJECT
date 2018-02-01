/***************************************************************************
** 文件名:  nwk.h
** 创建人:  雷迪 
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  网络层基本函数
**          
** 版  本:  1.0
***************************************************************************/
// History:     2014-10-25  Leidi 添加函数nwk_getLinkIdByAddress().


#ifndef NWK_H
#define NWK_H
#include <stdint.h>
#include "nwk_types.h"
#include "nwk_layer.h"

/* well known ports*/
#define SMPL_PORT_PING          0x01
#define SMPL_PORT_LINK          0x02
#define SMPL_PORT_JOIN          0x03
#define SMPL_PORT_SECURITY      0x04
#define SMPL_PORT_FREQ          0x05
#define SMPL_PORT_MGMT          0x06

#define SMPL_PORT_NWK_BCAST     0x1F
#define SMPL_PORT_USER_BCAST    0x3F

/* Unconnected User Datagram Link ID */
//#define SMPL_LINKID_USER_UUD    ((linkID_t) ~0)

#define PORT_BASE_NUMBER        0x20

/* Reserve the top of the User port namespace below the broadcast
 * address for static allocation.
 */
#define PORT_USER_STATIC_NUM    1
#define SMPL_PORT_STATIC_MAX    0x3E
#define SMPL_PORT_USER_MAX      (SMPL_PORT_STATIC_MAX - PORT_USER_STATIC_NUM)


/* to check connection info sanity */
#define CHK_RX   0
#define CHK_TX   1

/* return types for validating a reply frame */
#define SMPL_MY_REPLY    0
#define SMPL_A_REPLY     1
#define SMPL_NOT_REPLY   2

/* when allocating local Rx port it depends on whether the allocation
 * is being done as a result of a link or a link reply
 */
#define LINK_SEND   1
#define LINK_REPLY  2

#define  CONNSTATE_FREE           (0x00)
#define  CONNSTATE_JOINED         (0x01)
#define  CONNSTATE_CONNECTED      (0x02)

typedef struct
{
  volatile uint8_t     connState;
  //用来测试节点当前的状态，如果不在网置零cur_state即可，
  //不要置零connState,否则节点在此加入网络时要重新加入
           uint8_t     cur_state;      
           uint8_t     hops2target;
#ifdef APP_AUTO_ACK
  volatile uint8_t     ackTID;
#endif
           uint8_t     peerAddr[NET_ADDR_SIZE];
           rxMetrics_t sigInfo;
           uint8_t     portRx;
           uint8_t     portTx;
           linkID_t    thisLinkID;
#ifdef SMPL_SECURE
           uint32_t    connTxCTR;
           uint32_t    connRxCTR;
#endif
} connInfo_t;

#define SYS_NUM_CONNECTIONS   (NUM_CONNECTIONS+1)
//******************************************************************************
/* This structure aggregates eveything necessary to save if we want to restore
 * the connection information later.
 */
typedef struct
{
        uint8_t    structureVersion; /* to dectect upgrades... */
        uint8_t    numConnections;   /* count includes the UUD port/link ID */
/* The next two are used to detect overlapping port assignments. When _sending_ a
 * link frame the local port is assigned from the top down. When sending a _reply_
 * the assignment is bottom up. Overlapping assignments are rejected. That said it
 * is extremely unlikely that this will ever happen. If it does the test implemented
 * here is overly cautious (it will reject assignments when it needn't). But we leave
 * it that way on the assumption that it will never happen anyway.
 */
        uint8_t    curNextLinkPort;
        uint8_t    curMaxReplyPort;
        uint8_t    net_ed_num;      
        linkID_t   nextLinkID;
        linkID_t   nextJoinID;
#ifdef ACCESS_POINT
        sfInfo_t   sSandFContext;
#endif
/* Connection table entries last... */
        connInfo_t connStruct[SYS_NUM_CONNECTIONS];
} persistentContext_t;


/****************************************************************************************
 *                         Application Payload TID support
 *
 * Sometimes the receiving application uses a payload TID to determine if either it
 * missed frames (received TID > (last-seen TID+1)) or is seeing a duplicate (received
 * TID <= last-seen TID). Typically the TID simply increments for each successive frame.
 * But when the count wraps there is a problem. The received TID should always be one
 * more than the last TID. If it's equal, it's a duplicate. If it's less it's late. If
 * it's one more it's the one we expect. If it's more than 1 more then we missed frames.
 * Simple increments work for the wrap arithmetically. If the received TID is 0 and the last
 * seen ID is the biggest number in the world -- 0xFF... depending on type we can detect
 * the wrap. But if the receiver misses the 0 TID value for any reason or the biggest
 * number in the world TID is missed then susbequent testing for missed or duplicate
 * frames can get confused. We resolve this by allowing some leeway in the wrap testing.
 * this testing is assisted by the following macros. Setting TID_VALID_WINDOW to 0
 * will enforce a no leniency policy. In this case you'd better not miss either the
 * biggest number or the 0. The CHECK_TID_WRAP macro is only needed if the received
 * TID is less than the last-seen TID. The DUP_TID_AFTER_WRAP macro is only needed if the
 * received TID is greater than 1 more than the last-seen TID.
 ***************************************************************************************/
#define MAX_APT           ((appPTid_t)~0)    /* max value of application payload TID type */
#define TID_VALID_WINDOW  2                  /* window around max and 0 */

#define CHECK_TID_WRAP(lastTID, newTID)   ((lastTID >= (MAX_APT - TID_VALID_WINDOW)) && (newTID <= TID_VALID_WINDOW))
#define DUP_TID_AFTER_WRAP(lastTID, newTID)     ((newTID >= (MAX_APT - TID_VALID_WINDOW)) &&(lastTID <= TID_VALID_WINDOW))

/* prototypes */
smplStatus_t  nwk_nwkInit(uint8_t (*)(linkID_t,uint8_t));
connInfo_t   *nwk_getNextConnection(void);
void          nwk_freeConnection(connInfo_t *);
uint8_t       nwk_getNextClientPort(void);
connInfo_t   *nwk_getConnInfo(linkID_t port);
connInfo_t   *nwk_isLinkDuplicate(uint8_t *, uint8_t);
uint8_t       nwk_findAddressMatch(mrfiPacket_t *);
linkID_t      nwk_getLinkIdByAddress(addr_t *addr); //Add By Leidi 2014/10/25
smplStatus_t  nwk_checkConnInfo(connInfo_t *, uint8_t);
uint8_t       nwk_isConnectionValid(mrfiPacket_t *, linkID_t *);
uint8_t       nwk_allocateLocalRxPort(uint8_t, connInfo_t *);
uint8_t       nwk_isValidReply(mrfiPacket_t *, uint8_t, uint8_t, uint8_t);
connInfo_t   *nwk_findPeer(addr_t *, uint8_t);
smplStatus_t  nwk_NVObj(ioctlAction_t, ioctlNVObj_t *);


uint8_t       nwk_checkAppMsgTID(appPTid_t, appPTid_t);
void          nwk_getNumObjectFromMsg(void *, void *, uint8_t);
void          nwk_putNumObjectIntoMsg(void *, void *, uint8_t);
#ifdef ACCESS_POINT
sfInfo_t     *nwk_getSFInfoPtr(void);
#ifdef AP_IS_DATA_HUB
uint8_t       nwk_saveJoinedDevice(mrfiPacket_t *);
connInfo_t   *nwk_findAlreadyJoined(mrfiPacket_t *);
#endif
#endif

uint8_t nwk_resetUnlinkedED(void);
uint8_t nwk_resetlinkedED(void);
uint8_t nwk_getEDState(uint8_t *pInNetED,uint8_t *pOutNetED);
addr_t *nwk_getAddressByLinkId(linkID_t linkid);
uint8_t nwk_getEDbasicInfo(linkID_t linkID,uint8_t * ed_info);
#endif






