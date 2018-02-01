#ifdef FREQUENCY_AGILITY

#ifndef NET_FREQ_AGILITY_H
#define NET_FREQ_AGILITY_H


uint8_t getBestChanInTheOther(void);
void update_chan(void);
uint8_t getBestChanInAll(void);
/* Frequency Agility helper functions */
void    changeChannel(void);
void  checkChangeChannel(void);
#endif

#endif




