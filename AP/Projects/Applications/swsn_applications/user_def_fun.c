#include <string.h>
#include "bsp.h"
#include "bsp_leds.h"

#include "mrfi_spi.h"
#include "bui_pkg_fun.h"
#include "user_def_fun.h"
#include "uart_frm_proc.h"
#include "nwk_layer.h"
#include "net_frm_proc.h"
#include "stm32_rtc.h"
#include "firmware_info_proc.h"
#include "flash_start_addr.h"
#include "68_frm_proc.h"

extern  linkID_t sLinkID1;
extern  uint8_t  bro_cast_sem;
extern ap_time_t  real_time;

int32_t remoter_press_num = 0;
/***************************************************************************
 * @fn          user_def_proc
 *     
 * @brief       �û��Զ�����Ϣ������ڣ�����DI2ȷ����DI2 = 0x80��
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SUCCESS - �ɹ�
 *              APP_ERR     - ʧ��
 ***************************************************************************
 */ 
app_status_t user_def_proc(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  
  app_user_def_t  user_rc= APP_USER_DEF_SUCCESS;
  uint8_t  msg_id_1 = in_pkg->msg[2];
  uint8_t alarm_info[4];
  
  switch(msg_id_1)
  {
    case 0x10:  user_rc = time_proc(in_pkg, out_pkg, len);
                break;
    default:    user_rc = no_user_fun(in_pkg, out_pkg, len);      //δʹ�õ��û�����
  }

    
  if(user_rc == APP_USER_DEF_SUCCESS)
  {  
    return APP_SUCCESS;
  }
  else
  {
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x02;     
    alarm_info[2] = 0x02;
    alarm_info[3] = 0x08;
    *len =bui_pkg_alarm(in_pkg, alarm_info, 4, out_pkg);       
    return APP_ERR;
  }
}

/**************************************************************************************************
 * @fn          no_user_fun
 *
 * @brief       δʹ�õ��û�����.
 *
 * @param       in_frame  - �����֡�ṹ��
 *              out_msg   - ������ֽ�����
 *              out_len   - ������ֽ����鳤��
 *
 * @return      NO_USER_FUN_FAIL     - �����û�����ʧ��
 **************************************************************************************************
 */
app_user_def_t no_user_fun(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_user_def_t rc= NO_USER_FUN_FAIL;
   
  return rc;
}

/***************************************************************************
 * @fn          time_proc
 *     
 * @brief       ʱ����Ϣ����DI1 = 0x11��DI1 = 0����ʾ��ȡ��ǰʱ����Ϣ��
 *              DI1 = 1ʱ����ʾ����ʱ�䡣��ʽ����A-G:
 *              A=Year(0~99),B=Month(1~12),C=Day(1~31),D=Hours(0~23),
 *              E=Minute(0~59),F=Seconds(0~59),G=Weekday. Weekday[6:0]
 *              ��LSB��ʼ�ֱ��Ӧ�����죬����һ����������
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SUCCESS - �ɹ�
 *              APP_ERR     - ʧ��
 ***************************************************************************
 */ 
app_user_def_t time_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_user_def_t  rc= APP_TIME_PROC_FAIL;
  uint8_t  msg_id_0 = in_pkg->msg[3];

  if(msg_id_0 == 0x01)
  {
    Time_Set(&in_pkg->msg[4],7);
    
    rc= APP_USER_DEF_SUCCESS;
    *len = bui_rep_pkg_command(in_pkg,NULL,0,out_pkg);    
  }
  else if(msg_id_0 == 0x00)
  {
    uint8_t  return_time[7];
      
    return_time[0] = real_time.year-2000;
    return_time[1] = real_time.month;
    return_time[2] = real_time.day;
    return_time[3] = real_time.hour;
    return_time[4] = real_time.minute;
    return_time[5] = real_time.second;
    return_time[6] = set_week(real_time.week);
    
    rc= APP_USER_DEF_SUCCESS;
    *len = bui_rep_pkg_command(in_pkg,return_time,7,out_pkg);    
  }
  else
  {
  }

  return rc;
  
}

/////////////////////////////////////////////////////////////




