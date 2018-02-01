/***************************************************************************
** 文件名:  nwk_QMgmt.c
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  网络层帧队列管理
**          
** 版  本:  1.0
***************************************************************************/


/******************************************************************************
 * INCLUDES
 */
//#include <intrinsics.h>
#include "stm32f10x.h" 
#include "core_cm3.h"
#include <string.h>
#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk.h"
#include "nwk_frame.h"
#include "nwk_QMgmt.h"
#include "nwk_mgmt.h"     /* need offsets for poll frames */

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

#if SIZE_INFRAME_Q > 0
static frameInfo_t   sInFrameQ[SIZE_INFRAME_Q];
#else
static frameInfo_t  *sInFrameQ = NULL;
#endif  /* SIZE_INFRAME_Q > 0 */

static frameInfo_t   sOutFrameQ[SIZE_OUTFRAME_Q];

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
* @fn          nwk_QInit
* 
* @brief     Initialize the input and output frame queues to hold no packets. 
* 
* input parameters
* 
* output parameters
* 
* @return   void
*/
void nwk_QInit(void)
{
#if SIZE_INFRAME_Q > 0
  memset(sInFrameQ, 0, sizeof(sInFrameQ));
#endif  // SIZE_INFRAME_Q > 0
  memset(sOutFrameQ, 0, sizeof(sOutFrameQ));
}
 
/******************************************************************************
 * @fn          nwk_QfindSlot
 *
 * @brief       Finds a slot to use to retrieve the frame from the radio. It
 *              uses a LRU cast-out scheme. It is possible that this routine
 *              finds no slot. This can happen if the queue is of size 1 or 2
 *              and the Rx interrupt occurs during a retrieval call from an
 *              application. There are meta-states for frames as the application
 *              looks for the oldest frame on the port being requested.
 *
 *              This routine is running in interrupt context.
 *
 * input parameters
 * @param   which   - INQ or OUTQ to search
 *
 * output parameters
 *
 * @return      Pointer to oldest available frame in the queue
 */
frameInfo_t *nwk_QfindSlot(uint8_t which)
{
  frameInfo_t *pFI = NULL, *oldest= NULL, *newFI = NULL;
  uint8_t      i=0, num=0, newOrder = 0, orderTest=0;

  if (INQ == which)
  {
    pFI  = sInFrameQ;
    num  = SIZE_INFRAME_Q;
  }
  else
  {
    pFI  = sOutFrameQ;
    num  = SIZE_OUTFRAME_Q;
  }

  orderTest = num + 1;

  for (i=0; i<num; ++i, ++pFI)
  {
    /* if frame is available it's a candidate. */
    if (pFI->fi_usage != FI_AVAILABLE)
    {
      if (INQ == which)  /* TODO: do cast-out for Tx as well */
      {

        /* need to know the number of occupied slots so we know the age value
         * for the unoccupied slot (if there is one).
         */
        newOrder++;

        /* make sure nwk_retrieveFrame() is not processing this frame */
        if (FI_INUSE_TRANSITION == pFI->fi_usage)
        {
          continue;
        }
        /* is this frame older than any we've seen? */
        if (orderTest > pFI->orderStamp)
        {
          /* yes. */
          oldest    = pFI;
          orderTest = pFI->orderStamp;
        }
      }
    }
    else
    {
      if (OUTQ == which)  /* TODO: do cast-out for Tx as well */
      {
        return pFI;
      }
      newFI = pFI;
    }
  }

  /* did we find anything? */
  if (!newFI)
  {
    /* queue was full. cast-out happens here...unless... */
    if (!oldest)
    {
      /* This can happen if the queue is only of size 1 or 2 and all
       * the frames are in transition when the Rx interrupt occurs.
       */
      return (frameInfo_t *)0;
    }
    newFI = oldest;
    nwk_QadjustOrder(which, newFI->orderStamp);
    /*队列满了以后，如果直接将序号最小的空间直接用来存储新接收进来的数据，
      则会导致数据包的状态为FI_INUSE_UNTIL_DEL.程序必须要保证用来接收新
      数据的空间为FI_AVAILABLE状态。下面这一句为新增代码 。
    */
    newFI->fi_usage = FI_AVAILABLE;
    
    newFI->orderStamp = i;
  }
  else
  {
    /*防止误操作导致该slot的状态不是FI_AVAILABLE*/
    newFI->fi_usage = FI_AVAILABLE;
    
   /* mark the available slot. */
    newFI->orderStamp = ++newOrder;
  }

  return newFI;
}

/******************************************************************************
 * @fn          nwk_QadjustOrder
 *
 * @brief       Adjusts the age of everyone in the queue newer than the frame
 *              being removed.
 *
 * input parameters
 * @param   which   - INQ or OUTQ to adjust
 * @param   stamp   - value of frame being removed
 *
 * output parameters
 *
 * @return      void
 */
void nwk_QadjustOrder(uint8_t which, uint8_t stamp)
{
  frameInfo_t *pFI;
  uint8_t      i, num;
  bspIState_t  intState;

  if (INQ == which)
  {
    pFI  = sInFrameQ;
    num  = SIZE_INFRAME_Q;
  }
  else
  {
/*    pFI  = sOutFrameQ; */
/*    num  = SIZE_OUTFRAME_Q; */
    return;
  }

  BSP_ENTER_CRITICAL_SECTION(intState);

  for (i=0; i<num; ++i, ++pFI)
  {
    if ((pFI->fi_usage != FI_AVAILABLE) && (pFI->orderStamp > stamp))
    {
      pFI->orderStamp--;
    }
  }

  BSP_EXIT_CRITICAL_SECTION(intState);

  return;
}

/******************************************************************************
 * @fn          nwk_QfindOldest
 *
 * @brief       Look through frame queue and find the oldest available frame
 *              in the context in question. Supports connection-based (user),
 *              non-connection based (NWK applications), and the special case
 *              of store-and-forward.
 *
 * input parameters
 * @param   which      - INQ or OUTQ to adjust
 * @param   rcvContext - context information for finding the oldest
 * @param   usage      - normal usage or store-and-forward usage
 *
 * output parameters
 *
 * @return      Pointer to frame that is the oldsest on the requested port, or
 *              0 if there are none.
 */
frameInfo_t *nwk_QfindOldest(uint8_t which, rcvContext_t *rcv, uint8_t fi_usage)
{
  uint8_t      i, oldest, num, port;
  uint8_t      uType, addr12Compare;
  bspIState_t  intState;
  frameInfo_t *fPtr = NULL, *wPtr = NULL;
  connInfo_t  *pCInfo = NULL;
  uint8_t     *pAddr1 = NULL, *pAddr2 = NULL, *pAddr3  = NULL;

  if (INQ == which)
  {
    wPtr   = sInFrameQ;
    num    = SIZE_INFRAME_Q;
    oldest = SIZE_INFRAME_Q+1;
  }
  else
  {
/*    pFI  = sOutFrameQ; */
/*    num  = SIZE_OUTFRAME_Q; */
    return 0;
  }

  if (RCV_APP_LID == rcv->type)
  {
    pCInfo = nwk_getConnInfo(rcv->t.lid);
    if (!pCInfo)
    {
      return (frameInfo_t *)0;
    }
    port   = pCInfo->portRx;
    pAddr2 = pCInfo->peerAddr;
  }
  else if (RCV_NWK_PORT == rcv->type)
  {
    port = rcv->t.port;
  }
#ifdef ACCESS_POINT
  else if (RCV_RAW_POLL_FRAME == rcv->type)
  {
    port   = *(MRFI_P_PAYLOAD(rcv->t.pkt)+F_APP_PAYLOAD_OS+M_POLL_PORT_OS);
    pAddr2 = MRFI_P_SRC_ADDR(rcv->t.pkt);
    pAddr3 = MRFI_P_PAYLOAD(rcv->t.pkt)+F_APP_PAYLOAD_OS+M_POLL_ADDR_OS;
  }
#endif
  else
  {
    return (frameInfo_t *)0;
  }

  uType = (USAGE_NORMAL == fi_usage) ? FI_INUSE_UNTIL_DEL : FI_INUSE_UNTIL_FWD;

  for (i=0; i<num; ++i, ++wPtr)
  {

    BSP_ENTER_CRITICAL_SECTION(intState);   /* protect the frame states */

    /* only check entries in use and waiting for this port */
    if (uType == wPtr->fi_usage)
    {
      wPtr->fi_usage = FI_INUSE_TRANSITION;

      BSP_EXIT_CRITICAL_SECTION(intState);  /* release hold */
      /* message sent to this device? */
      if (GET_FROM_FRAME(MRFI_P_PAYLOAD(&wPtr->mrfiPkt), F_PORT_OS) == port)
      {
        /* Port matches. If the port of interest is a NWK applicaiton we're a
         * match...the NWK applications are not connection-based. If it is a
         * NWK application we need to check the source address for disambiguation.
         * Also need to check source address if it's a raw frame lookup (S&F frame)
         */
        if (RCV_APP_LID == rcv->type)
        {
          if (SMPL_PORT_USER_BCAST == port)
          {
            /* guarantee a match... */
            pAddr1 = pCInfo->peerAddr;
          }
          else
          {
            pAddr1 = MRFI_P_SRC_ADDR(&wPtr->mrfiPkt);
          }
        }
#ifdef ACCESS_POINT
        else if (RCV_RAW_POLL_FRAME == rcv->type)
        {
          pAddr1 = MRFI_P_DST_ADDR(&wPtr->mrfiPkt);
        }
#endif

        addr12Compare = memcmp(pAddr1, pAddr2, NET_ADDR_SIZE);
        if (  (RCV_NWK_PORT == rcv->type) ||
              (!pAddr3 && !addr12Compare) ||
              (pAddr3 && !memcmp(pAddr3, MRFI_P_SRC_ADDR(&wPtr->mrfiPkt), NET_ADDR_SIZE))
           )
        {
          if (wPtr->orderStamp < oldest)
          {
            if (fPtr)
            {
              /* restore previous oldest one */
              fPtr->fi_usage = uType;
            }
            oldest = wPtr->orderStamp;
            fPtr   = wPtr;
            continue;
          }
          else
          {
            /* not oldest. restore state */
            wPtr->fi_usage = uType;
          }
        }
        else
        {
          /* not a match. restore state */
          wPtr->fi_usage = uType;
        }
      }
      else
      {
        /* wrong port. restore state */
        wPtr->fi_usage = uType;
      }
    }
    else
    {
      BSP_EXIT_CRITICAL_SECTION(intState);
    }
  }

  return fPtr;
}


/******************************************************************************
 * @fn          nwk_getQ
 *
 * @brief       Get location of teh specified frame queue.
 *
 * input parameters
 * @param   which   - INQ or OUTQ to get
 *
 * output parameters
 *
 * @return      Pointer to frame queue
 */
frameInfo_t *nwk_getQ(uint8_t which)
{
  return (INQ == which) ? sInFrameQ : sOutFrameQ;
}


/******************************************************************************
 * @fn          nwk_QfindAppFrame
 *
 * @brief       判断输入队列中是否有数据.不对网络层帧和广播帧进行判断。
 *
 * input parameters
 *
 * output parameters
 *
 * @return      1，有数据；0，没有数据。
 */
uint8_t nwk_QfindAppFrame(void)
{
  uint8_t      i, num, port;
  frameInfo_t *fPtr = NULL;
  
  fPtr   = sInFrameQ;
  num    = SIZE_INFRAME_Q;

  for (i=0; i<num; ++i, ++fPtr)
  {
    port = GET_FROM_FRAME(MRFI_P_PAYLOAD(&fPtr->mrfiPkt), F_PORT_OS);
    if((fPtr->fi_usage == FI_INUSE_UNTIL_DEL) && \
        (port >= PORT_BASE_NUMBER) && \
        (port <= SMPL_PORT_STATIC_MAX))
    {
      return 1;
    }
  }
  return 0;
}

