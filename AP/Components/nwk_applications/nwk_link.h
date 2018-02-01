/***************************************************************************
** 文件名:  nwk_link.h
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  节点连接信息的处理函数
**          
** 版  本:  1.0
***************************************************************************/

#ifndef NWK_LINK_H
#define NWK_LINK_H

#include "nwk_types.h"

/* Macros needed for protocol backward compatibility */
#define LINK_LEGACY_MSG_LENGTH       8
#define LINK_REPLY_LEGACY_MSG_LENGTH 3


#define LINK_LISTEN_ON   0
#define LINK_LISTEN_OFF  1

/* application payload offsets */
/*    both */
#define LB_REQ_OS         0
#define LB_TID_OS         1

/*    link frame */
#define L_LINK_TOKEN_OS        2
#define L_RMT_PORT_OS          6
#define L_MY_RXTYPE_OS         7
#define L_PROTOCOL_VERSION_OS  8
#define L_CTR_OS               9
/*    link reply frame */
#define LR_RMT_PORT_OS         2
#define LR_MY_RXTYPE_OS        3
#define LR_ED_IPH_OS           4
#define LR_ED_IPL_OS           5
#define LR_AP_IPH_OS           6
#define LR_AP_IPL_OS           7
#define LR_CTR_OS              8

/*    unlink frame */
#define UL_RMT_PORT_OS        2
/*    unlink reply frame */
#define ULR_RESULT_OS         2

/* change the following as protocol developed */
#ifndef SMPL_SECURE
#define MAX_LINK_APP_FRAME      9
#else
#define MAX_LINK_APP_FRAME      13
#endif

/* frame sizes */
#ifndef SMPL_SECURE
#define LINK_FRAME_SIZE         9
#define LINK_REPLY_FRAME_SIZE   8
#else
#define LINK_FRAME_SIZE         13
#define LINK_REPLY_FRAME_SIZE   12
#endif
#define UNLINK_FRAME_SIZE       3
#define UNLINK_REPLY_FRAME_SIZE 3

/* link requests
 * NOTE: If aditional command codes are required do _not_ use the
 *       value LINK_REPLY_LEGACY_MSG_LENGTH. This numeral is used
 *       to guarantee that legacy Link frames (from before release
 *       1.0.6) work correctly. Don't ask.
 */

#define LINK_REQ_LINK       1
#define LINK_REQ_UNLINK     2

/* prototypes */
fhStatus_t   nwk_processLink(mrfiPacket_t *);
linkID_t     nwk_getLocalLinkID(void);
void         nwk_linkInit(void);
smplStatus_t nwk_link(linkID_t *);
smplStatus_t nwk_unlink(linkID_t);
void         nwk_setLinkToken(uint32_t);
void         nwk_getLinkToken(uint32_t *);
void         nwk_setListenContext(uint8_t);

void         nwk_setDuplicateLinkLid(linkID_t lid);
linkID_t     nwk_getDuplicateLinkLid(void);
void         nwk_setDuplicateLinkFlag(void);
void         nwk_clearDuplicateLinkFlag(void);
uint8_t      nwk_isExistDuplicateLinkED(void);
#endif


