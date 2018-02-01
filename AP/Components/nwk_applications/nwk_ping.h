/***************************************************************************
** 文件名:  nwk_ping.h
** 创建人:  雷迪
** 日  期:  2016年01月17日
** 修改人:  
** 日  期:  
** 描  述:  网络层ping命令相关函数
**          
** 版  本:  1.0
***************************************************************************/


#ifndef NWK_PING_H
#define NWK_PING_H

/* change the following as protocol developed */
#define MAX_PING_APP_FRAME    2

/* application payload offsets */
/*    both */
#define PB_REQ_OS     0
#define PB_TID_OS     1


/* ping requests */
#define PING_REQ_PING       1

/* prototypes */
fhStatus_t   nwk_processPing(mrfiPacket_t *);
void         nwk_pingInit(void);
smplStatus_t nwk_ping(linkID_t);


#endif



