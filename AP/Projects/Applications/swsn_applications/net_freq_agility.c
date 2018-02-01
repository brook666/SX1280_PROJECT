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
 * @brief       更新信道。上电时读取保存在flash中的信道，并设置信道,此信道如果
 *              没有干扰，可以与节点进行正常通行。然后选择一个最有信道，并发起
 *              跳频(在先前保存在flash中的信道上发送广播帧)
 *     
 * @data        2015年08月07日
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
/***********上电复位上一次使用的信道*********************/
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
 * @brief       返回质量最好的信道。本系统总共有4个信道，在每个信道上读10次
 *              RSSI值，选择RSSI值平均值最小的一个信道作为最有信道。刚上电时使用。
 *     
 * @data        2015年08月07日
 *     
 * @param       void
 *     
 * @return      质量最好的信道号。
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
    //SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_SET, &freq);//重复发两次广播消息
    SWSN_DELAY(1);
    for(j=0;j<10;j++)
    {
      SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RSSI, (void *)&dbm);
      chanrssi[i]+=dbm; 
    }
    chanrssi[i]/=10; 
    if(chanrssi[i]<chanrssi[chan_num])
      chan_num=i;   //4个信道选一个
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
 * @brief       返回质量最好的信道(当前信道除外)。信道在通行无法正常进行时，
 *              当前信道已无法通信，故只需要在其余3个信道中选出一个最优信道即可。
 *     
 * @data        2015年08月07日
 *     
 * @param       void
 *     
 * @return      质量最好的信道号。
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
    if(i==curfreq.logicalChan) //此句是直接跳出原信道，不检测
    {
      chanrssi[i]=0;
      continue;
    }
    freq.logicalChan = i;
    nwk_setChannel(&freq);
    //SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_SET, &freq);//重复发两次广播消息
    SWSN_DELAY(1);
    for(j=0;j<10;j++)
    {
      SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RSSI, (void *)&dbm);
      chanrssi[i]+=dbm; 
    }
    chanrssi[i]/=10; 
   // if(chanrssi[i]<chanrssi[chan_num])
    //  chan_num=i;   //原来是4个信道选一个
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
 * @brief       选择一个最优信道，并广播发送给节点，最后改变集中器信道。
 *     
 * @data        2015年08月07日
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
 * @brief       信道检测，集中器发起跳频的依据之一。集中器在当前信道上进行
 *              5次RSSI值，如果存在连续的3次超过阈值-70dBm,则发起跳频。
 *     
 * @data        2015年08月07日
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



