/***************************************************************************
** 文件名:  nwk_join.h
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  节点加入网络的基本信息处理
**          
** 版  本:  1.0
***************************************************************************/
#include "nwk_types.h"

#ifndef NWK_JOIN_H
#define NWK_JOIN_H

#define JOIN_CONTEXT_ON  (0x01)
#define JOIN_CONTEXT_OFF (0x02)

/* Macros needed for protocol backward compatibility */
#define JOIN_LEGACY_MSG_LENGTH        7
#define JOIN_REPLY_LEGACY_MSG_LENGTH  6

/* place holder... */
#define SEC_CRYPT_KEY_SIZE  0

/* application payload offsets */
/*    both */
#define JB_REQ_OS                0
#define JB_TID_OS                1
/*    join frame */
#define J_JOIN_TOKEN_OS          2
#define J_NUMCONN_OS             6
#define J_PROTOCOL_VERSION_OS    7
/*    join reply frame */
#define JR_LINK_TOKEN_OS         2
#define JR_CRYPTKEY_SIZE_OS      6
#define JR_CRYPTKEY_OS           7

/* change the following as protocol developed */
#define MAX_JOIN_APP_FRAME    (JR_CRYPTKEY_OS + SEC_CRYPT_KEY_SIZE)

/* set out frame size */
#define JOIN_FRAME_SIZE         8
#define JOIN_REPLY_FRAME_SIZE   MAX_JOIN_APP_FRAME

/* join requests
 * NOTE: If aditional command codes are required do _not_ use the
 *       value JOIN_REPLY_LEGACY_MSG_LENGTH. This numeral is used
 *       to guarantee that legacy Join frames (from before release
 *       1.0.6) work correctly. Don't ask.
 */
#define JOIN_REQ_JOIN       1

/* prototypes */
void            nwk_joinInit(uint8_t (*)(linkID_t,uint8_t));
smplStatus_t    nwk_join(void);
fhStatus_t      nwk_processJoin(mrfiPacket_t *);
void            nwk_getJoinToken(uint32_t *);
void            nwk_setJoinContext(uint8_t);
void            nwk_setJoinToken(uint32_t);
void            nwk_getJoinToken(uint32_t *);
#ifdef ACCESS_POINT
sfClientInfo_t *nwk_isSandFClient(uint8_t *, uint8_t *);
#endif

#endif

