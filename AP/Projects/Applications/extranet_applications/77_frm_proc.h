#ifndef _77_FRM_PROC_H
#define _77_FRM_PROC_H


#include <string.h>
#include "bsp.h"
#include "delay.h"
#include "bui_pkg_fun.h"
#include "gtway_frm_proc.h"

void proc_77_frm(uint8_t *,uint8_t *,uint16_t *);
void proc_77_data_frm(uint8_t *,uint8_t *,uint16_t *);
void proc_77_command_frm(uint8_t *,uint8_t *,uint16_t *);
void proc_77_ack_frm(uint8_t *,uint8_t *,uint16_t *);
void proc_77_alarm_frm(uint8_t *,uint8_t *,uint16_t *);
void get_ap_ed_mac_ip(app_77_pkg_t *,uint8_t *,uint16_t *);
void get_alarm_info(app_77_pkg_t *,uint8_t *,uint16_t *);
void get_gtway_type_mac_77(app_77_pkg_t *,uint8_t *,uint16_t *);

#endif




