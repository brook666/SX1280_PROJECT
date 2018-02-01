/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
**  �����ſ�����
**  RTCʱ������
**  �汾��Rev1.0 
**  ���ڣ�2011-6-20
**  ��̳��www.openmcu.com
**  ��̳��www.openmcu.com  bbs.openmcu.com
**  �Ա���http://shop36995246.taobao.com/   
**  Copyright(c)@��������  Dream_etd@163.com  
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
#include "bsp.h"
#include "stm32_rtc.h"


ap_time_t  real_time;  //����һ��ʱ��ṹ�����

uint8_t init_time[7] = {15,5,27,0,0,0,3};
//�·����ݱ�    
const uint8_t table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5};       //���������ݱ�    
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};  //ƽ����·����ڱ�
/***************************************************************************
 * @fn          Init_RTC
 *     
 * @brief       ��ʼ��RTC
 *     
 * @data        2015��08��04��
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
  
                                  //ʹ��PWR��BKP����ʱ��      
  PWR_BackupAccessCmd(ENABLE);                  //ʹ��RTC�ͺ󱸼Ĵ������� 
  
  if(BKP_ReadBackupRegister(BKP_DR1)!=0x5555)         //��ָ���ĺ󱸼Ĵ����ж������ݣ��ж��Ƿ�Ϊ��һ������
  {                      
    BKP_DeInit();                       //������BKP��ȫ���Ĵ�������Ϊȱʡֵ   
//    RCC_LSEConfig(RCC_LSE_ON);                  //ʹ���ⲿ����ʱ�� 32.768KHz
//    while ((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)&& (StartUpCounter < 0x200000))     //���ָ����RCC��־λ�������,�ȴ����پ������
//    {
//    }
    RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);   //����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��     
    RCC_RTCCLKCmd(ENABLE);                    //ʹ��RTCʱ��  
    RTC_WaitForSynchro();                   //�ȴ�RTC�Ĵ���(RTC_CNT,RTC_ALR��RTC_PRL)��RTC APBʱ��ͬ��
    RTC_WaitForLastTask();                    //�ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_ITConfig(RTC_IT_SEC, ENABLE);             //ʹ��RTC���ж�
    RTC_WaitForLastTask();                    //�ȴ����һ�ζ�RTC�Ĵ�����д�������
    RTC_SetPrescaler(62499); //625                //10ms  //����RTCԤ��Ƶ��ֵ  RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
    RTC_WaitForLastTask();                    //�ȴ����һ�ζ�RTC�Ĵ�����д�������
    Time_Set(init_time,7);                          //ʱ������  
    BKP_WriteBackupRegister(BKP_DR1, 0x5555);         //��ָ���ĺ󱸼Ĵ�����д���û���������0X5555���жϱ�־                    
  }                                 
  else                              //���ǵ�һ������ ������ʱ
  {
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)      //���ָ����RCC��־λ�������:POR/PDR��λ
    {
//          printf("�ϵ縴λ������\r\n");
    }                     
    else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)    //���ָ����RCC��־λ�������:�ܽŸ�λ
    {
//          printf("�ⲿ��λ������\r\n");
    }
//      printf("����Ҫ���á�����\r\n");
    
    RTC_WaitForSynchro();                   //�ȴ����һ�ζ�RTC�Ĵ�����д�������
    
    RTC_ITConfig(RTC_IT_SEC, ENABLE);             //ʹ��RTC���ж�

    RTC_WaitForLastTask();                    //�ȴ����һ�ζ�RTC�Ĵ�����д�������
  }                    
  Time_Get(&real_time);                           //����ʱ�� 
	
  
  RCC_ClearFlag();                        //���RCC�ĸ�λ��־λ
  
                   
  return 0; //ok    
}


/***************************************************************************
 * @fn          Time_Update
 *     
 * @brief       ����ʱ�ӣ��������ʱ��ת��Ϊ���ӡ�
 *              ��1970��1��1��Ϊ��׼��1970~2099��Ϊ�Ϸ����
 * @data        2015��08��04��
 *     
 * @param       in_time - ����ʱ�䣬����Ϊap_time_t
 *     
 * @return      0    : �ɹ���
 *              ���� : ������롣
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
  
  for(t=1970;t<in_time.year;t++)                      //��������ݵ��������
  {
    if(Is_Leap_Year(t))
    {
      seccount+=31622400;           //�����������
    }
    else 
    {
      seccount+=31536000;                   //ƽ���������
    }
  }
  in_time.month -= 1;
  for(t=0;t<in_time.month;t++)                          //��ǰ���·ݵ����������
  {
    seccount+=(uint32_t)mon_table[t]*86400;           //�·����������
    if(Is_Leap_Year(in_time.year)&&t==1)
    {
      seccount+=86400;        //����2�·�����һ��������� 
    }
  }
  seccount+=(uint32_t)(in_time.day-1)*86400;                //��ǰ�����ڵ���������� 
  seccount+=(uint32_t)in_time.hour*3600;                  //Сʱ������
  seccount+=(uint32_t)in_time.minute*60;                    //����������
  seccount+=in_time.second;                         //�������Ӽ���ȥ
                          
  RTC_WaitForLastTask();                      //�ȴ����һ�ζ�RTC�Ĵ�����д�������
  RTC_SetCounter(seccount);                   //����RTC��������ֵ
  RTC_WaitForLastTask();                      //�ȴ����һ�ζ�RTC�Ĵ�����д�������   
  return 0;   
}


/***************************************************************************
 * @fn          Time_Get
 *     
 * @brief       ��ȡ��ǰʱ�䡣
 *     
 * @data        2015��08��04��
 *     
 * @param       void
 *     
 * @return      1   : �ɹ���
 *              ����: �������
 ***************************************************************************
 */ 
uint8_t Time_Get(ap_time_t *real_time)
{
  static uint16_t daycnt=0;
  uint32_t timecount=0; 
  uint32_t temp=0;
  uint16_t temp1=0;   
   
  timecount = RTC_GetCounter();                 //��� RTC ������ֵ(������)      
  temp=timecount/86400;   //�õ�����(��������Ӧ��)
  if(daycnt!=temp)//����һ����
  {   
    daycnt=temp;
    temp1=1970; //��1970�꿪ʼ
    while(temp>=365)
    {        
      if(Is_Leap_Year(temp1))//������
      {
        if(temp>=366)
        {
          temp-=366;//�����������
        }
        else 
        {
          temp1++;
          break;
        }  
      }
      else 
      {
        temp-=365;    //ƽ�� 
      }
      temp1++;  
    }   
    real_time->year=temp1;//�õ����
    temp1=0;
    while(temp>=28)//������һ����
    {
      if(Is_Leap_Year(real_time->year)&&temp1==1)//�����ǲ�������/2�·�
      {
        if(temp>=29)
        {
          temp-=29;//�����������
        }
        else 
        {
          break;
        }
      }
      else 
      {
        if(temp>=mon_table[temp1])temp-=mon_table[temp1];//ƽ��
        else break;
      }
      temp1++;  
    }
    real_time->month=temp1+1;//�õ��·�
    real_time->day=temp+1;  //�õ����� 
  }
  temp=timecount%86400;     //�õ�������       
  real_time->hour=temp/3600;     //Сʱ
  real_time->minute=(temp%3600)/60; //���� 
  real_time->second=(temp%3600)%60; //����
  real_time->week=RTC_Get_Week(real_time->year,real_time->month,real_time->day);//��ȡ����   
  return 1;
}

//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//������������������� 
//����ֵ�����ں�    
/***************************************************************************
 * @fn          RTC_Get_Week
 *     
 * @brief       ���빫�����ڵõ�����(ֻ����1901-2099��)
 *     
 * @data        2015��08��05��
 *     
 * @param       year  - ������
 *              month - ������
 *              day   - ������
 *
 * @return      ���ںţ�0,1,2,3,4��5,6�ֱ��ʾ�����죬����һ...������
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
    yearL+=100;// ���Ϊ21����,�������100 
  }
  
  temp2=yearL+yearL/4;    // ����������ֻ��1900��֮���
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
 * @brief       �ж��Ƿ�������
 *     
 * @data        2015��08��05��
 *     
 * @param       year - �������
 *     
 * @return      1 , ��
 *              0 , ����
 ***************************************************************************
 */ 

uint8_t Is_Leap_Year(uint16_t year)
{       
  if(year%4==0) //�����ܱ�4����
  { 
    if(year%100==0) 
    { 
      if(year%400==0)
        return 1;//�����00��β,��Ҫ�ܱ�400����      
      else 
        return 0;   
    }
    else 
      return 1;   
  }
  else return 0;  
}
/*****************************************************************************
** ��������: Time_Set
** ��������: ͨ����������ʱ��
          ����������ʱ����
** ��  ����: Dream
** �ա�  ��: 2011��6��20��
*****************************************************************************/
/***************************************************************************
 * @fn          Time_Set
 *     
 * @brief       ����ʱ��
 *     
 * @data        2015��08��05��
 *     
 * @param       pInTime - �����ʱ�䳣����week������Ҫת��
 *              len     - ��Ч���ݵĳ���
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

  RTC_ClearITPendingBit(RTC_IT_SEC);    //�������������жϱ�־                       
  RTC_WaitForLastTask();          //�ȴ�RTC�Ĵ����������
  BSP_EXIT_CRITICAL_SECTION(intState); 

	return 1;
}

/***************************************************************************
 * @fn          get_time_s
 *     
 * @brief       ��ȡ��ǰʱ�������
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      ���ص�ǰʱ�������
 ***************************************************************************
 */ 
int get_time_s(void)
{
  return RTC_GetCounter() ;
}


/***************************************************************************
 * @fn          get_week
 *     
 * @brief       ��bitλת�������ڼ�
 *     
 * @data        2015��08��05��
 *     
 * @param       week - [6:0]�ӵ�λ��ʼ ���α�ʾ�����죬����һ...������
 *     
 * @return      0,1,2,3,4��5,6�ֱ��ʾ�����죬����һ...������
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
 * @brief       �����ڼ�ת����bitλ
 *     
 * @data        2015��08��05��
 *     
 * @param       week - 0,1,2,3,4��5,6�ֱ��ʾ�����죬����һ...������
 *     
 * @return      [6:0]�ӵ�λ��ʼ���α�ʾ�����죬����һ...������
 ***************************************************************************
 */ 

uint8_t set_week(uint8_t week)
{
  if(week>6)
    return week;
  return (1<<week);
}



