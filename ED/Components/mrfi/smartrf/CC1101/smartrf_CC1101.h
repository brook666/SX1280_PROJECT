/***************************************************************
 *  SmartRF Studio(tm) Export
 *
 *  Radio register settings specifed with C-code
 *  compatible #define statements.
 *
 ***************************************************************/

#ifndef SMARTRF_CC1101_H
#define SMARTRF_CC1101_H

#define SMARTRF_RADIO_CC1101

#define SMARTRF_SETTING_IOCFG2           0x29
#define SMARTRF_SETTING_IOCFG1           0x2E
#define SMARTRF_SETTING_IOCFG0           0x06
#define SMARTRF_SETTING_FIFOTHR          0x07
#define SMARTRF_SETTING_SYNC1            0xD3
#define SMARTRF_SETTING_SYNC0            0x91
#define SMARTRF_SETTING_PKTLEN           0xFF
#define SMARTRF_SETTING_PKTCTRL1         0x04
#define SMARTRF_SETTING_PKTCTRL0         0x05
#define SMARTRF_SETTING_ADDR             0x00
//#define SMARTRF_SETTING_CHANNR           0x60 //xiaowei
//#define SMARTRF_SETTING_CHANNR           0x30 //weiwu
#define SMARTRF_SETTING_CHANNR           0x10 //ÍâÍø
#define SMARTRF_SETTING_FSCTRL1          0x08
#define SMARTRF_SETTING_FSCTRL0          0x00
#define SMARTRF_SETTING_FREQ2            0x10
#define SMARTRF_SETTING_FREQ1            0xA7
#define SMARTRF_SETTING_FREQ0            0x62

#if   defined (DATARATE_1_2KBAUD)
#define SMARTRF_SETTING_MDMCFG4          0xF5  //1.2KBaud£¬RX filter BW = 58KHz
#define SMARTRF_SETTING_MDMCFG3          0x83
#define SMARTRF_SETTING_DEVIATN          0x15  //ÆµÆ«5.2KHz

#elif defined (DATARATE_38_4KBAUD)
#define SMARTRF_SETTING_MDMCFG4          0xCA  //38.4KBaud£¬RX filter BW = 100KHz
#define SMARTRF_SETTING_MDMCFG3          0x83
#define SMARTRF_SETTING_DEVIATN          0x34  //ÆµÆ«20KHz

#elif defined (DATARATE_100KBAUD)
#define SMARTRF_SETTING_MDMCFG4          0x5B  //100KBaud£¬RX filter BW = 325KHz
#define SMARTRF_SETTING_MDMCFG3          0xF8
#define SMARTRF_SETTING_DEVIATN          0x47  //ÆµÆ«47.6KHz

#elif   defined (DATARATE_250KBAUD) 
#define SMARTRF_SETTING_MDMCFG4          0x2D   //250KBaud£¬RX filter BW = 540KHz
#define SMARTRF_SETTING_MDMCFG3          0x3B
#define SMARTRF_SETTING_DEVIATN          0x62  //ÆµÆ«127KHz

#endif


#define SMARTRF_SETTING_MDMCFG2          0x13

#ifdef FEC_ENABLE
#define SMARTRF_SETTING_MDMCFG1          0xa2
#else
#define SMARTRF_SETTING_MDMCFG1          0x22
#endif

#define SMARTRF_SETTING_MDMCFG0          0xF8

#define SMARTRF_SETTING_MCSM2            0x07
#define SMARTRF_SETTING_MCSM1            0x30
#define SMARTRF_SETTING_MCSM0            0x18
#define SMARTRF_SETTING_FOCCFG           0x1D
#define SMARTRF_SETTING_BSCFG            0x1C
#define SMARTRF_SETTING_AGCCTRL2         0xC7
#define SMARTRF_SETTING_AGCCTRL1         0x00
#define SMARTRF_SETTING_AGCCTRL0         0xB2
#define SMARTRF_SETTING_WOREVT1          0x87
#define SMARTRF_SETTING_WOREVT0          0x6B
#define SMARTRF_SETTING_WORCTRL          0xFB
#define SMARTRF_SETTING_FREND1           0xB6
#define SMARTRF_SETTING_FREND0           0x10
#define SMARTRF_SETTING_FSCAL3           0xEA
#define SMARTRF_SETTING_FSCAL2           0x2A
#define SMARTRF_SETTING_FSCAL1           0x00
#define SMARTRF_SETTING_FSCAL0           0x1F
#define SMARTRF_SETTING_RCCTRL1          0x41
#define SMARTRF_SETTING_RCCTRL0          0x00
#define SMARTRF_SETTING_FSTEST           0x59
#define SMARTRF_SETTING_PTEST            0x7F
#define SMARTRF_SETTING_AGCTEST          0x3F
#define SMARTRF_SETTING_TEST2            0x88
#define SMARTRF_SETTING_TEST1            0x31
#define SMARTRF_SETTING_TEST0            0x09
#define SMARTRF_SETTING_PARTNUM          0x00
#define SMARTRF_SETTING_VERSION          0x04
#define SMARTRF_SETTING_FREQEST          0x00
#define SMARTRF_SETTING_LQI              0x00
#define SMARTRF_SETTING_RSSI             0x00
#define SMARTRF_SETTING_MARCSTATE        0x00
#define SMARTRF_SETTING_WORTIME1         0x00
#define SMARTRF_SETTING_WORTIME0         0x00
#define SMARTRF_SETTING_PKTSTATUS        0x00
#define SMARTRF_SETTING_VCO_VC_DAC       0x00
#define SMARTRF_SETTING_TXBYTES          0x00
#define SMARTRF_SETTING_RXBYTES          0x00
#define SMARTRF_SETTING_RCCTRL1_STATUS   0x00
#define SMARTRF_SETTING_RCCTRL0_STATUS   0x00

#endif
