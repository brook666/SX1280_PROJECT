#ifndef _GTWAY_FRM_PROC_H
#define _GTWAY_FRM_PROC_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
 #include"bsp.h"


#define MAX_APP_MSG1 2048

/* 网外通信帧（0x69）帧头与帧尾 */
//#define ED_FRAME_HEAD 0x69
//#define ED_FRAME_TAIL 0x17

/* 获取网外通信帧的传输方向 */
#define GET_ED_FRM_TRANS_DIRECTION(msg)  ((uint8_t)msg[2]&0x0f)


typedef struct
{
  uint8_t  frm_head;
  uint8_t  reserve1;
  uint16_t ctr_code;
  uint16_t ed_type;
  uint8_t  ed_addr[4];
  uint8_t  server_addr[4];
  uint8_t  mobile_addr[6];
  uint16_t pid_num;
  uint16_t data_len;     
  uint8_t  msg[MAX_APP_MSG1];   
  uint8_t  cs;
  uint8_t  frm_end;

} ed_pkg_t;

typedef struct
{
  uint8_t  type[2];
  uint8_t  addr[4];
} ed_addr_t;

void Send_ed_msg_to_uart(uint8_t *, uint16_t );
uint8_t bui_ed_ack(ed_pkg_t *,uint8_t * );
uint8_t bui_ed_alarm(ed_pkg_t *,uint8_t *,uint8_t ,uint8_t *);

uint8_t Bui_bcast_ed_msg_from_inner_msg(const uint8_t *pInner_msg, uint8_t inner_msg_len, uint8_t *pOut_msg);
void pro_ed_data_ack(ed_pkg_t *pIn_pkg);

uint16_t bui_outnet_frm(ed_pkg_t *, uint8_t *, uint16_t, uint8_t *);
void bui_ed_pkg(uint8_t *pIn_msg, ed_pkg_t *pOut_pkg);
uint16_t Reverse_trans_direction(uint16_t);
ed_addr_t * get_ed_addr(void);
#endif

