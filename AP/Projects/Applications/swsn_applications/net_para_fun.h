#ifndef NET_PARA_FUN_H
#define NET_PARA_FUN_H

#include "bsp.h"
#include "nwk_layer.h"
#include "mrfi_spi.h"
#include "bui_pkg_fun.h"

typedef enum app_net_para{
  APP_NET_PARA_SUCCESS,
  APP_FREQ_PROC_FAIL,
  APP_CHAN_PROC_FALL,
  APP_CHANSPC_PROC_FALL,
  APP_RATE_PROC_FALL,
  APP_MANCHTER_PROC_FALL,
  APP_DEVIATN_PROC_FALL,
  APP_MODFORMAT_PROC_FALL,
  APP_WHITING_PROC_FALL,
  APP_TXPW_PROC_FALL,
  APP_PARTNUM_READ_FALL,
  APP_VERSION_READ_FALL,
  APP_RSSI_READ_FALL,
  APP_RSSI_NO_ED_FIND,
  APP_BEGIN_FIRMWARE_UPDATE_FAIL,
  APP_END_FIRMWARE_UPDATE_FAIL,
  APP_SET_ADDRESS_FALL,
  NO_PAPA_FUN_FAIL,
} app_net_para_t;


app_status_t   net_para_proc(app_pkg_t *,uint8_t *,uint8_t *);
app_net_para_t freq_proc(app_pkg_t *,uint8_t *, uint8_t *);     //频率的读写
app_net_para_t chan_proc(app_pkg_t *,uint8_t *, uint8_t *);     //信道的读写
app_net_para_t chanspc_proc(app_pkg_t *,uint8_t *, uint8_t *);  //信道间隔的读写
app_net_para_t rate_proc(app_pkg_t *,uint8_t *, uint8_t *);   //速率的读写,
app_net_para_t manchter_proc(app_pkg_t *,uint8_t *, uint8_t *);  //曼切斯特编码使能的读写
app_net_para_t deviatn_proc(app_pkg_t *,uint8_t *, uint8_t *);  //频率偏差的读写
app_net_para_t modformat_proc(app_pkg_t *,uint8_t *, uint8_t *);//调制方式的读写
app_net_para_t whiting_proc(app_pkg_t *,uint8_t *, uint8_t *);  //白话使能
app_net_para_t txpw_proc(app_pkg_t *,uint8_t *, uint8_t *);     //发射功率的读写
app_net_para_t read_version(app_pkg_t *,uint8_t *, uint8_t *);   //读取芯片版本号
app_net_para_t read_rssi(app_pkg_t *,uint8_t *, uint8_t *);   //读取RSSI
app_net_para_t add_ap_rssi(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len);   //添加集中器RSSI值到读取节点RSSI的返回帧中
app_net_para_t bandrate_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len);  //读取与设置波特率
app_net_para_t device_type_proc(app_pkg_t *,uint8_t *, uint8_t *);   // 获取设备类型    
app_net_para_t begin_firmware_update(app_pkg_t *,uint8_t *, uint8_t *);   //开始固件更新
app_net_para_t end_firmware_update(app_pkg_t *,uint8_t *, uint8_t *);   //结束固件更新
app_net_para_t set_my_device_address(app_pkg_t *,uint8_t *, uint8_t *);//设置本机地址
app_net_para_t no_para_fun(app_pkg_t *, uint8_t *, uint8_t *);  


uint8_t  change_to_rate(uint8_t * ,uint32_t *);
uint8_t  rate_to_arry(uint32_t * bandrate,uint8_t * respond);
#endif


