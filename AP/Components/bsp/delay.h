#ifndef __DELAY
#define __DELAY

#include "stm32f10x.h"

void delay_init(u8 SYSCLK);
void delay_ms(u32 nms);
void delay_us(u32 nus);
void delay_s(u32 ns);
#endif
