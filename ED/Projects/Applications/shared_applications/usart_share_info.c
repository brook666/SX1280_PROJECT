#include "usart_share_info.h"



/***************************************************************************
 * @fn          check_crc
 *     
 * @brief       检测应用层数据帧的CRC
 *     
 * @data        2015年08月05日
 *     
 * @param       frm - 数据
 *              len - 数据长度
 *     
 * @return      1 , CRC正确
 *              0 , CRC错误
 ***************************************************************************
 */ 
uint8_t check_crc(uint8_t *frm, uint16_t len)
{
  uint8_t crc_sum=0;
  uint16_t i;
  for( i =0; i < len-1; i++)
  {
    crc_sum = *frm + crc_sum;
    frm++;
  }
  if(crc_sum == *frm)    //判断是不是校验码
    return 1;      //是
  else
    return 0;          //不是
}

/**************************************************************************************************
 * @fn          DMA_ResetCurrDataCounter
 *
 * @brief       重置DMA通道X的传输数量寄存器，DMA通道X开始从起始地址接收或发送数据
 *
 * @param       DMA_Channelx   - DMA通道X
 *              cnt            - 设置传输数量寄存器的值
 *
 * @return      空
 **************************************************************************************************
 */
void DMA_ResetCurrDataCounter(DMA_Channel_TypeDef*DMA_Channelx,uint16_t cnt)
{ 
  DMA_Cmd(DMA_Channelx, DISABLE); //改变缓冲区大小前先要禁止通道工作
  DMA_Channelx->CNDTR=cnt; //DMAx,传输数据量
  DMA_Cmd(DMA_Channelx, ENABLE);   
}




