#ifndef  __RTC_H
#define	 __RTC_H

#include "stm32f10x_rtc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_rcc.h"

typedef struct
{ 
  uint16_t year;
  uint8_t month; 
  uint8_t day; 
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint16_t msec;
  uint8_t week;
} ap_time_t;


uint8_t Init_RTC(void);
uint8_t Is_Leap_Year(uint16_t year);
uint8_t Time_Update(ap_time_t);
uint8_t Time_Get(ap_time_t *real_time);
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day);
uint16_t Usart_Scanf(uint32_t value,uint8_t count);
uint8_t Time_Set(uint8_t* in_time,uint8_t len);
int get_time_s(void);
uint8_t get_week(uint8_t week);
uint8_t set_week(uint8_t week);



#endif
