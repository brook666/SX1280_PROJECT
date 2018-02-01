

#include <string.h>
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
#include "usart_share_info.h"
#include "stm32_usart2_gtway.h"
#include "app_globals.h"
#include "delay.h"
#include "bsp_config.h"
#include "bsp.h"
#include "gtway_frm_proc.h"


#define MAX_APP_MSGLEN 240

void Usart1_DMA_Configuration(void);	
void Iterates_usart1_buffer(void);
void read_Usart1_DMA_FIFO(usart_buffer_t* usart1_rbuf);
uint16_t bui_usart1_outnet_frm(uint8_t *pOut_msg, uint8_t *pInner_msg, uint16_t inner_msg_len);
app_usart_proc_t check_usart1_frame_head(uint16_t * pBegin, uint16_t * pEnd );
app_usart_proc_t check_usart1_frame_end_and_crc(uint16_t *beginIndex);
void usart1_err_proc(usart_buffer_t* usart_rbuf);
