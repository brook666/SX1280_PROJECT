/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
**  启航号开发板
**  RTC时钟配置
**  版本：Rev1.0 
**  日期：2011-6-20
**  论坛：www.openmcu.com
**  论坛：www.openmcu.com  bbs.openmcu.com
**  淘宝：http://shop36995246.taobao.com/   
**  Copyright(c)@旺宝电子  Dream_etd@163.com  
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
#include "bsp.h"
#include "stm32_rtc.h"


ap_time_t  real_time;  //定义一个时间结构体变量

uint8_t init_time[7] = {15,5,27,0,0,0,3};
//月份数据表    
const uint8_t table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5};       //月修正数据表    
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};  //平年的月份日期表
/***************************************************************************
 * @fn          Init_RTC
 *     
 * @brief       初始化RTC
 *     
 * @data        2015年08月04日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
uint8_t Init_RTC(void)
{
  __IO uint32_t StartUpCounter = 0, LSEStatus = 0;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  
                                  //使能PWR和BKP外设时钟      
  PWR_BackupAccessCmd(ENABLE);                  //使能RTC和后备寄存器访问 
  
  if(BKP_ReadBackupRegister(BKP_DR1)!=0x5555)         //从指定的后备寄存器中读出数据，判断是否为第一次配置
  {                      
    BKP_DeInit();                       //将外设BKP的全部寄存器重设为缺省值   
//    RCC_LSEConfig(RCC_LSE_ON);                  //使能外部低速时钟 32.768KHz
//    while ((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)&& (StartUpCounter < 0x200000))     //检查指定的RCC标志位设置与否,等待低速晶振就绪
//    {
//    }
//		
//	RCC_LSICmd(ENABLE);
//	
//	/* Wait till LSE is ready */
//	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
//	{}
//	
//	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);				
		
    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);   //设置RTC时钟(RTCCLK),选择LSE作为RTC时钟     
    RCC_RTCCLKCmd(ENABLE);                    //使能RTC时钟  
    RTC_WaitForSynchro();                   //等待RTC寄存器(RTC_CNT,RTC_ALR和RTC_PRL)与RTC APB时钟同步
    RTC_WaitForLastTask();                    //等待最近一次对RTC寄存器的写操作完成
    RTC_ITConfig(RTC_IT_SEC, ENABLE);             //使能RTC秒中断
    RTC_WaitForLastTask();                    //等待最近一次对RTC寄存器的写操作完成
    RTC_SetPrescaler(62499); //625                //10ms  //设置RTC预分频的值  RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
    RTC_WaitForLastTask();                    //等待最近一次对RTC寄存器的写操作完成
    Time_Set(init_time,7);                          //时间设置  
    BKP_WriteBackupRegister(BKP_DR1, 0x5555);         //向指定的后备寄存器中写入用户程序数据0X5555做判断标志                                    
  }                                 
  else                              //不是第一次配置 继续计时
  {
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)      //检查指定的RCC标志位设置与否:POR/PDR复位
    {
//          printf("上电复位。。。\r\n");
    }                     
    else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)    //检查指定的RCC标志位设置与否:管脚复位
    {
//          printf("外部复位。。。\r\n");
    }
//      printf("不需要配置。。。\r\n");
    
    RTC_WaitForSynchro();                   //等待最近一次对RTC寄存器的写操作完成
    
    RTC_ITConfig(RTC_IT_SEC, ENABLE);             //使能RTC秒中断

    RTC_WaitForLastTask();                    //等待最近一次对RTC寄存器的写操作完成
  }                    
  Time_Get(&real_time);                           //更新时间 
	
  
  RCC_ClearFlag();                        //清除RCC的复位标志位
  
                   
  return 0; //ok    
}


/***************************************************************************
 * @fn          Time_Update
 *     
 * @brief       设置时钟，将输入的时钟转换为秒钟。
 *              以1970年1月1日为基准，1970~2099年为合法年份
 * @data        2015年08月04日
 *     
 * @param       in_time - 输入时间，类型为ap_time_t
 *     
 * @return      0    : 成功；
 *              其它 : 错误代码。
 ***************************************************************************
 */ 
uint8_t Time_Update(ap_time_t  in_time)
{
  uint16_t t;
  uint32_t seccount=0;
  if(in_time.year<1970||in_time.year>2099)
  {
    return 1; 
  } 
  
  for(t=1970;t<in_time.year;t++)                      //把所有年份的秒钟相加
  {
    if(Is_Leap_Year(t))
    {
      seccount+=31622400;           //闰年的秒钟数
    }
    else 
    {
      seccount+=31536000;                   //平年的秒钟数
    }
  }
  in_time.month -= 1;
  for(t=0;t<in_time.month;t++)                          //把前面月份的秒钟数相加
  {
    seccount+=(uint32_t)mon_table[t]*86400;           //月份秒钟数相加
    if(Is_Leap_Year(in_time.year)&&t==1)
    {
      seccount+=86400;        //闰年2月份增加一天的秒钟数 
    }
  }
  seccount+=(uint32_t)(in_time.day-1)*86400;                //把前面日期的秒钟数相加 
  seccount+=(uint32_t)in_time.hour*3600;                  //小时秒钟数
  seccount+=(uint32_t)in_time.minute*60;                    //分钟秒钟数
  seccount+=in_time.second;                         //最后的秒钟加上去
                          
  RTC_WaitForLastTask();                      //等待最近一次对RTC寄存器的写操作完成
  RTC_SetCounter(seccount);                   //设置RTC计数器的值
  RTC_WaitForLastTask();                      //等待最近一次对RTC寄存器的写操作完成   
  return 0;   
}


/***************************************************************************
 * @fn          Time_Get
 *     
 * @brief       获取当前时间。
 *     
 * @data        2015年08月04日
 *     
 * @param       void
 *     
 * @return      1   : 成功；
 *              其它: 错误代码
 ***************************************************************************
 */ 
uint8_t Time_Get(ap_time_t *real_time)
{
  static uint16_t daycnt=0;
  uint32_t timecount=0; 
  uint32_t temp=0;
  uint16_t temp1=0;   
   
  timecount = RTC_GetCounter();                 //获得 RTC 计数器值(秒钟数)      
  temp=timecount/86400;   //得到天数(秒钟数对应的)
  if(daycnt!=temp)//超过一天了
  {   
    daycnt=temp;
    temp1=1970; //从1970年开始
    while(temp>=365)
    {        
      if(Is_Leap_Year(temp1))//是闰年
      {
        if(temp>=366)
        {
          temp-=366;//闰年的秒钟数
        }
        else 
        {
          temp1++;
          break;
        }  
      }
      else 
      {
        temp-=365;    //平年 
      }
      temp1++;  
    }   
    real_time->year=temp1;//得到年份
    temp1=0;
    while(temp>=28)//超过了一个月
    {
      if(Is_Leap_Year(real_time->year)&&temp1==1)//当年是不是闰年/2月份
      {
        if(temp>=29)
        {
          temp-=29;//闰年的秒钟数
        }
        else 
        {
          break;
        }
      }
      else 
      {
        if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
        else break;
      }
      temp1++;  
    }
    real_time->month=temp1+1;//得到月份
    real_time->day=temp+1;  //得到日期 
  }
  temp=timecount%86400;     //得到秒钟数       
  real_time->hour=temp/3600;     //小时
  real_time->minute=(temp%3600)/60; //分钟 
  real_time->second=(temp%3600)%60; //秒钟
  real_time->week=RTC_Get_Week(real_time->year,real_time->month,real_time->day);//获取星期   
  return 1;
}

//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号    
/***************************************************************************
 * @fn          RTC_Get_Week
 *     
 * @brief       输入公历日期得到星期(只允许1901-2099年)
 *     
 * @data        2015年08月05日
 *     
 * @param       year  - 公历年
 *              month - 公历月
 *              day   - 公历日
 *
 * @return      星期号，0,1,2,3,4，5,6分别表示星期天，星期一...星期六
 ***************************************************************************
 */ 

uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{ 
  uint16_t temp2;
  uint8_t yearH,yearL;
  
  yearH=year/100;
  yearL=year%100;    
  if (yearH>19)
  {
    yearL+=100;// 如果为21世纪,年份数加100 
  }
  
  temp2=yearL+yearL/4;    // 所过闰年数只算1900年之后的
  temp2=temp2%7; 
  temp2=temp2+day+table_week[month-1];
  if (yearL%4==0&&month<3)
  {
    temp2--;
  }
  return(temp2%7);
} 


/***************************************************************************
 * @fn          Is_Leap_Year
 *     
 * @brief       判断是否是闰年
 *     
 * @data        2015年08月05日
 *     
 * @param       year - 输入年份
 *     
 * @return      1 , 是
 *              0 , 不是
 ***************************************************************************
 */ 

uint8_t Is_Leap_Year(uint16_t year)
{       
  if(year%4==0) //必须能被4整除
  { 
    if(year%100==0) 
    { 
      if(year%400==0)
        return 1;//如果以00结尾,还要能被400整除      
      else 
        return 0;   
    }
    else 
      return 1;   
  }
  else return 0;  
}
/*****************************************************************************
** 函数名称: Time_Set
** 功能描述: 通过键盘设置时间
          包括年月日时分秒
** 作  　者: Dream
** 日　  期: 2011年6月20日
*****************************************************************************/
/***************************************************************************
 * @fn          Time_Set
 *     
 * @brief       设置时间
 *     
 * @data        2015年08月05日
 *     
 * @param       pInTime - 输入的时间常量，week常量需要转换
 *              len     - 有效数据的长度
 *     
 * @return      void
 ***************************************************************************
 */ 

uint8_t Time_Set(uint8_t* pInTime,uint8_t len)
{
  bspIState_t intState;
  if(len != 7)
    return 0;
  
  BSP_ENTER_CRITICAL_SECTION(intState);  
  real_time.year   = 2000+pInTime[0];
  real_time.month  = pInTime[1];
  real_time.day    = pInTime[2];
  real_time.hour   = pInTime[3];
  real_time.minute = pInTime[4];
  real_time.second = pInTime[5];
  real_time.week   = get_week(pInTime[6]);

  
  Time_Update(real_time);

  RTC_ClearITPendingBit(RTC_IT_SEC);    //清除溢出，秒钟中断标志                       
  RTC_WaitForLastTask();          //等待RTC寄存器操作完成
  BSP_EXIT_CRITICAL_SECTION(intState); 

	return 1;
}

/***************************************************************************
 * @fn          get_time_s
 *     
 * @brief       获取当前时间的秒数
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      返回当前时间的秒数
 ***************************************************************************
 */ 
int get_time_s(void)
{
  return RTC_GetCounter() ;
}


/***************************************************************************
 * @fn          get_week
 *     
 * @brief       将bit位转换成星期几
 *     
 * @data        2015年08月05日
 *     
 * @param       week - [6:0]从低位开始 依次表示星期天，星期一...星期六
 *     
 * @return      0,1,2,3,4，5,6分别表示星期天，星期一...星期六
 ***************************************************************************
 */ 

uint8_t get_week(uint8_t week)
{
  uint8_t i = 0;
  for(i=0;i<6;i++)
  {
    if(week&0x01)
    {
      break;
    }
    else
    {
      week = week>>1;
    }
  }
  return i;
}

/***************************************************************************
 * @fn          set_week
 *     
 * @brief       将星期几转换成bit位
 *     
 * @data        2015年08月05日
 *     
 * @param       week - 0,1,2,3,4，5,6分别表示星期天，星期一...星期六
 *     
 * @return      [6:0]从低位开始依次表示星期天，星期一...星期六
 ***************************************************************************
 */ 

uint8_t set_week(uint8_t week)
{
  if(week>6)
    return week;
  return (1<<week);
}



