#ifndef _NET_FRM_PROC_H
#define _NET_FRM_PROC_H

#include "bsp.h"
#include "nwk_layer.h"

typedef struct
{
  linkID_t lid;
  uint8_t  isBack;
  uint8_t  frm_count;
}net_frm_back_t;

uint8_t net_frm_proc(void);
uint8_t sCB(linkID_t,uint8_t);
uint8_t net_ed_link(void);
uint8_t net_frm_proc(void);
void proc_net_msg(linkID_t lid,uint8_t *,uint8_t);
void proc_bcast_frm(void);
void send_broad_time_sync(void);
uint8_t write_net_frm_to_rx_buffer(linkID_t);
#endif
