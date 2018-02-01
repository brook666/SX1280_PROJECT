#include"string.h"
#include "nwk_layer.h"
#include "stm32_usart2_gtway.h"
#include "bsp_stm32_defs.h"
#include "mrfi_spi.h"
#include "net_freq_agility.h"

// extern  globle extern
extern	uint8_t  uart_frm_rbuf1_vld ;
extern volatile uint8_t net_frm_cur_num;
extern volatile uint8_t net_ed_add_sem;
extern volatile uint8_t net_ed_num;

#ifdef FREQUENCY_AGILITY 
/*     ************** BEGIN interference detection support */
#define INTERFERNCE_THRESHOLD_DBM (-70)  //swsn provide
//#define INTERFERNCE_THRESHOLD_DBM (-95)
#define SSIZE    25
#define IN_A_ROW  3
 
static int8_t  sSample[SSIZE];
static uint8_t sChannel = 0;


int16_t chanrssi[4];
freqEntry_t curfreq;

#endif 


/***************************************************************************
 * @fn          update_chan
 *     
 * @brief       �����ŵ����ϵ�ʱ��ȡ������flash�е��ŵ����������ŵ�,���ŵ����
 *              û�и��ţ�������ڵ��������ͨ�С�Ȼ��ѡ��һ�������ŵ���������
 *              ��Ƶ(����ǰ������flash�е��ŵ��Ϸ��͹㲥֡)
 *     
 * @data        2015��08��07��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void  update_chan(void)
{
#ifdef FREQUENCY_AGILITY  
  freqEntry_t freq;
/***********�ϵ縴λ��һ��ʹ�õ��ŵ�*********************/
  sChannel = channr_num_from_flash();
  freq.logicalChan = sChannel;
  nwk_setChannel(&freq);
#ifdef DEBUG
  put_string_uart1("The prefer logic channel:\n");
  put_char_uart1(sChannel);
  put_string_uart1("chan num:0x");
  put_char_uart1(mrfiSpiReadReg(CHANNR));
#endif
/*******************************************************/
   sChannel=getBestChanInAll();
  
   freq.logicalChan=sChannel;
   SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_SET, &freq);
   channr_num_to_flash(sChannel);
#ifdef DEBUG
   put_string_uart1("after changechannel!!!\n ");
   put_string_uart1("The current logical channel: ");
   put_char_uart1(sChannel);
   put_string_uart1("chan num:0x");
   put_char_uart1(mrfiSpiReadReg(CHANNR));
#endif  
#endif 
  return;
}

/***************************************************************************
 * @fn          getBestChanInAll
 *     
 * @brief       ����������õ��ŵ�����ϵͳ�ܹ���4���ŵ�����ÿ���ŵ��϶�10��
 *              RSSIֵ��ѡ��RSSIֵƽ��ֵ��С��һ���ŵ���Ϊ�����ŵ������ϵ�ʱʹ�á�
 *     
 * @data        2015��08��07��
 *     
 * @param       void
 *     
 * @return      ������õ��ŵ��š�
 ***************************************************************************
 */ 
uint8_t getBestChanInAll(void)
{
#ifdef FREQUENCY_AGILITY   

  freqEntry_t freq;
  int8_t dbm=0;	
  uint8_t i=0;
  uint8_t j=0; 
  uint8_t chan_num;
  uint8_t udbm[4]={0,0,0,0};
  SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_GET, &curfreq);
  chan_num=curfreq.logicalChan ; 
#ifdef DEBUG
  put_string_uart1("getBestChanInAll!!!\n ");
#endif
  for(i=0;i<4;i++)
  {
    if (net_frm_cur_num || net_ed_add_sem ||uart_frm_rbuf1_vld )
    {
      nwk_setChannel(&curfreq);
      return chan_num;
    }
    freq.logicalChan = i;
    nwk_setChannel(&freq);
    //SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_SET, &freq);//�ظ������ι㲥��Ϣ
    SWSN_DELAY(1);
    for(j=0;j<10;j++)
    {
      SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RSSI, (void *)&dbm);
      chanrssi[i]+=dbm; 
    }
    chanrssi[i]/=10; 
    if(chanrssi[i]<chanrssi[chan_num])
      chan_num=i;   //4���ŵ�ѡһ��
  }
  nwk_setChannel(&curfreq);
#ifdef DEBUG
  put_string_uart1("every RSSI!!!\n ");
  for(i=0;i<4;i++)
  {
    udbm[i]=-chanrssi[i];
    put_char_uart1_10(udbm[i]);
  }
#endif
   
  return chan_num;
#else
  return 0;
#endif
  
}


/***************************************************************************
 * @fn          getBestChanInTheOther
 *     
 * @brief       ����������õ��ŵ�(��ǰ�ŵ�����)���ŵ���ͨ���޷���������ʱ��
 *              ��ǰ�ŵ����޷�ͨ�ţ���ֻ��Ҫ������3���ŵ���ѡ��һ�������ŵ����ɡ�
 *     
 * @data        2015��08��07��
 *     
 * @param       void
 *     
 * @return      ������õ��ŵ��š�
 ***************************************************************************
 */ 
uint8_t getBestChanInTheOther(void)
{
#ifdef FREQUENCY_AGILITY  

  freqEntry_t freq;
  int8_t dbm=0;
  uint8_t i=0; 
  uint8_t j=0;
  uint8_t chan_num;
  uint8_t udbm[4]={0,0,0,0};
  int16_t chanrssi_temp=0;
  SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_GET, &curfreq);
  chan_num=curfreq.logicalChan ;
#ifdef DEBUG
       put_string_uart1("getBestChanInTheOther!!!\n ");
#endif
  for(i=0;i<4;i++)
  {
    if (net_frm_cur_num || net_ed_add_sem ||uart_frm_rbuf1_vld )
    {
      nwk_setChannel(&curfreq);
      return chan_num;
    }
    if(i==curfreq.logicalChan) //�˾���ֱ������ԭ�ŵ��������
    {
      chanrssi[i]=0;
      continue;
    }
    freq.logicalChan = i;
    nwk_setChannel(&freq);
    //SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_SET, &freq);//�ظ������ι㲥��Ϣ
    SWSN_DELAY(1);
    for(j=0;j<10;j++)
    {
      SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RSSI, (void *)&dbm);
      chanrssi[i]+=dbm; 
    }
    chanrssi[i]/=10; 
   // if(chanrssi[i]<chanrssi[chan_num])
    //  chan_num=i;   //ԭ����4���ŵ�ѡһ��
    if(chanrssi_temp==0)
    {
      chanrssi_temp=chanrssi[i];
      chan_num=i;
    }
    else
    {
      if(chanrssi[i]<=chanrssi_temp)
      {
        chanrssi_temp=chanrssi[i];
        chan_num=i;
      }
    } 
  }
  nwk_setChannel(&curfreq);
#ifdef DEBUG
  put_string_uart1("every RSSI!!!\n ");
  for(i=0;i<4;i++)
  {
    udbm[i]=-chanrssi[i];
    put_char_uart1_10(udbm[i]);
  }
#endif
   
  return chan_num;
#else
  return 0;
#endif
  
}


/***************************************************************************
 * @fn          changeChannel
 *     
 * @brief       ѡ��һ�������ŵ������㲥���͸��ڵ㣬���ı伯�����ŵ���
 *     
 * @data        2015��08��07��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void changeChannel(void)
{
#ifdef FREQUENCY_AGILITY  
  freqEntry_t freq;
#ifdef DEBUG
  put_string_uart1("changechannel!!!\n ");
#endif
  
  sChannel=getBestChanInTheOther();
  
  freq.logicalChan=sChannel;
  SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_SET, &freq);
  channr_num_to_flash(sChannel);
#ifdef DEBUG
  put_string_uart1("after changechannel!!!\n ");
  put_string_uart1("logic channel:\n ");
  put_char_uart1(sChannel);
  put_string_uart1("chan num:0x");
  put_char_uart1(mrfiSpiReadReg(CHANNR));
#endif  
#endif 
  return;
}

/***************************************************************************
 * @fn          checkChangeChannel
 *     
 * @brief       �ŵ���⣬������������Ƶ������֮һ���������ڵ�ǰ�ŵ��Ͻ���
 *              5��RSSIֵ���������������3�γ�����ֵ-70dBm,������Ƶ��
 *     
 * @data        2015��08��07��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void  checkChangeChannel(void)
{
#ifdef FREQUENCY_AGILITY
  int8_t dbm, inARow = 0;

  uint8_t i;

  memset(sSample, 0x0, SSIZE);
  for (i=0; i<SSIZE; ++i)
  {
    /* quit if we need to service an app frame */
    if (net_frm_cur_num || net_ed_add_sem || uart_frm_rbuf1_vld)
    {
      return;
    }
    SWSN_DELAY(1);
    SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RSSI, (void *)&dbm);
    sSample[i] = dbm;
#ifdef DEBUG
        //utemp=-dbm;
        //put_char_uart1_10(utemp);
#endif
    if (dbm > INTERFERNCE_THRESHOLD_DBM)
    {
      if (++inARow == IN_A_ROW)
      {
        changeChannel();
        break;
      }
    }
    else
    {
      inARow = 0;
    }
  }
//#ifdef DEBUG
//       put_string_uart1("check over!!!\n ");
//       put_string_uart1("logic channel:\n ");
//       put_char_uart1(curfreq.logicalChan);
//       put_string_uart1("chan num:0x");
//       put_char_uart1(mrfiSpiReadReg(CHANNR));
//#endif
#endif
  return;
}



