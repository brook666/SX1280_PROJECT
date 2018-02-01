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
 * @brief       用户自定义信息处理入口，根据DI2确定，DI2 = 0x80。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度     
 *     
 * @return      APP_SUCCESS - 成功
 *              APP_ERR     - 失败
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
    default:    user_rc = no_user_fun(in_pkg, out_pkg, len);      //未使用的用户函数
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
 * @brief       未使用的用户函数.
 *
 * @param       in_frame  - 输入的帧结构体
 *              out_msg   - 输出的字节数组
 *              out_len   - 输出的字节数组长度
 *
 * @return      NO_USER_FUN_FAIL     - 处理用户参数失败
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
 * @brief       时间信息处理，DI1 = 0x11。DI1 = 0，表示读取当前时间信息；
 *              DI1 = 1时，表示设置时间。格式如下A-G:
 *              A=Year(0~99),B=Month(1~12),C=Day(1~31),D=Hours(0~23),
 *              E=Minute(0~59),F=Seconds(0~59),G=Weekday. Weekday[6:0]
 *              从LSB开始分别对应星期天，星期一…星期六。
 *     
 * @data        2015年08月07日
 *     
 * @param       in_pkg  - 已打包好的结构体数据
 *              out_pkg - 数组数据
 *              len     - 输出数据长度     
 *     
 * @return      APP_SUCCESS - 成功
 *              APP_ERR     - 失败
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




