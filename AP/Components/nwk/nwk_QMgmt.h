/***************************************************************************
** 文件名:  nwk_QMgmt.h
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  网络层帧队列管理
**          
** 版  本:  1.0
***************************************************************************/

#ifndef NWK_QMGMT_H
#define NWK_QMGMT_H

#include "nwk_types.h"
#include "nwk_frame.h"

#define  INQ   1
#define  OUTQ  2

#define  USAGE_NORMAL  1
#define  USAGE_FWD     2

/* prototypes */
void              nwk_QInit(void);
frameInfo_t *nwk_QfindSlot(uint8_t);
void         nwk_QadjustOrder(uint8_t, uint8_t);
frameInfo_t *nwk_QfindOldest(uint8_t, rcvContext_t *, uint8_t);
frameInfo_t *nwk_getQ(uint8_t);
uint8_t nwk_QfindAppFrame(void);

#endif  /* NWK_QMGMT_H */



