#ifndef _88_FRM_PROC_H
#define _88_FRM_PROC_H


#include <string.h>
#include "delay.h"
#include "bui_pkg_fun.h"
#include "gtway_frm_proc.h"

void proc_88_frm(uint8_t *,uint8_t *,uint8_t *);
void proc_88_data_frm(uint8_t *,uint8_t *,uint8_t *);
void proc_88_command_frm(uint8_t *,uint8_t *,uint8_t *);
void proc_88_ack_frm(uint8_t *,uint8_t *,uint8_t *);
void proc_88_alarm_frm(uint8_t *,uint8_t *,uint8_t *);

app_status_t com_gtway_88_proc(app_pkg_t *,uint8_t *, uint8_t *);
app_status_t sw_gtway_88_proc(app_pkg_t *,uint8_t *, uint8_t *);
app_device_disg_t server_frm_proc(app_pkg_t *,uint8_t *,uint8_t *);
app_server_para_t check_heart_beat(app_pkg_t *,uint8_t *,uint8_t *);
app_server_para_t get_gtway_type_mac(app_pkg_t *,uint8_t *,uint8_t *);
//app_server_para_t get_router_ssid(app_pkg_t *,uint8_t *,uint8_t *);
app_server_para_t sync_gtway_time(app_pkg_t *,uint8_t *,uint8_t *);
app_server_para_t server_begin_firmware_update(app_pkg_t *,uint8_t *,uint8_t *);
app_server_para_t server_end_firmware_update(app_pkg_t *,uint8_t *,uint8_t *);
app_server_para_t check_wifi_heart_frm(app_pkg_t *,uint8_t *,uint8_t *);
app_server_para_t server_get_gtway_code_version(app_pkg_t *,uint8_t *,uint8_t *);

#endif




