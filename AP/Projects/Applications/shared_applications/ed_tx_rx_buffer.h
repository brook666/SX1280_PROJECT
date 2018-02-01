#ifndef _ED_TX_RX_BUFFER_H
#define _ED_TX_RX_BUFFER_H

#include "nwk_layer.h"
#include "gtway_frm_proc.h"

 /* 操作节点缓冲区后的状态枚举 */
typedef enum {
  BUFFER_SUCCESS,
  BUFFER_IP_MODIFY_FAIL,
  BUFFER_FREE_FAIL,
}buffer_result_t;

/* 网关所连接的服务器IP地址 */
extern  addr_t sGtwayServerIP;
extern volatile uint8_t app_frm_num ;
extern volatile uint8_t tx_fail_frm_num ;
extern volatile uint8_t start_emergency_dispatch_in_rtc ;
extern volatile uint8_t emergency_dispatch_flag ; 

extern uint8_t Write_rx_ed_buffer(linkID_t lid);
extern void Iterates_rx_ed_buffer(void);
extern void Find_ed_info_in_rx_buffer(void);

extern void Write_tx_fail_ed_buffer(linkID_t lid,uint8_t *pIn_msg, uint8_t* pIn_len);
extern void Iterates_tx_fail_ed_buffer(void);
extern void Find_ed_info_in_tx_fail_buffer(void);

extern void Write_tx_ed_buffer(const gtway_pkg_t *pIn_gtway_pkg, uint8_t *pOut_msg, uint8_t* pOut_len);
extern void Iterates_tx_ed_buffer(void);
extern buffer_result_t Free_tx_ed_buffer(uint16_t ed_ip);
extern uint8_t isReadyToWriteGroupFrm(void);
extern buffer_result_t Modify_ip_in_tx_ed_buffer(uint16_t current_ed_ip, uint16_t modified_ed_ip);

#endif




