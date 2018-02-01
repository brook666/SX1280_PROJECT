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
app_net_para_t freq_proc(app_pkg_t *,uint8_t *, uint8_t *);     //Ƶ�ʵĶ�д
app_net_para_t chan_proc(app_pkg_t *,uint8_t *, uint8_t *);     //�ŵ��Ķ�д
app_net_para_t chanspc_proc(app_pkg_t *,uint8_t *, uint8_t *);  //�ŵ�����Ķ�д
app_net_para_t rate_proc(app_pkg_t *,uint8_t *, uint8_t *);   //���ʵĶ�д,
app_net_para_t manchter_proc(app_pkg_t *,uint8_t *, uint8_t *);  //����˹�ر���ʹ�ܵĶ�д
app_net_para_t deviatn_proc(app_pkg_t *,uint8_t *, uint8_t *);  //Ƶ��ƫ��Ķ�д
app_net_para_t modformat_proc(app_pkg_t *,uint8_t *, uint8_t *);//���Ʒ�ʽ�Ķ�д
app_net_para_t whiting_proc(app_pkg_t *,uint8_t *, uint8_t *);  //�׻�ʹ��
app_net_para_t txpw_proc(app_pkg_t *,uint8_t *, uint8_t *);     //���书�ʵĶ�д
app_net_para_t read_version(app_pkg_t *,uint8_t *, uint8_t *);   //��ȡоƬ�汾��
app_net_para_t read_rssi(app_pkg_t *,uint8_t *, uint8_t *);   //��ȡRSSI
app_net_para_t add_ap_rssi(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len);   //��Ӽ�����RSSIֵ����ȡ�ڵ�RSSI�ķ���֡��
app_net_para_t bandrate_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len);  //��ȡ�����ò�����
app_net_para_t device_type_proc(app_pkg_t *,uint8_t *, uint8_t *);   // ��ȡ�豸����    
app_net_para_t begin_firmware_update(app_pkg_t *,uint8_t *, uint8_t *);   //��ʼ�̼�����
app_net_para_t end_firmware_update(app_pkg_t *,uint8_t *, uint8_t *);   //�����̼�����
app_net_para_t set_my_device_address(app_pkg_t *,uint8_t *, uint8_t *);//���ñ�����ַ
app_net_para_t no_para_fun(app_pkg_t *, uint8_t *, uint8_t *);  


uint8_t  change_to_rate(uint8_t * ,uint32_t *);
uint8_t  rate_to_arry(uint32_t * bandrate,uint8_t * respond);
#endif


