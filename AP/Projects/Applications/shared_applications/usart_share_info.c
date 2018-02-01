#include "usart_share_info.h"



/***************************************************************************
 * @fn          check_crc
 *     
 * @brief       ���Ӧ�ò�����֡��CRC
 *     
 * @data        2015��08��05��
 *     
 * @param       frm - ����
 *              len - ���ݳ���
 *     
 * @return      1 , CRC��ȷ
 *              0 , CRC����
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
  if(crc_sum == *frm)    //�ж��ǲ���У����
    return 1;      //��
  else
    return 0;          //����
}

/**************************************************************************************************
 * @fn          DMA_ResetCurrDataCounter
 *
 * @brief       ����DMAͨ��X�Ĵ��������Ĵ�����DMAͨ��X��ʼ����ʼ��ַ���ջ�������
 *
 * @param       DMA_Channelx   - DMAͨ��X
 *              cnt            - ���ô��������Ĵ�����ֵ
 *
 * @return      ��
 **************************************************************************************************
 */
void DMA_ResetCurrDataCounter(DMA_Channel_TypeDef*DMA_Channelx,uint16_t cnt)
{ 
  DMA_Cmd(DMA_Channelx, DISABLE); //�ı仺������Сǰ��Ҫ��ֹͨ������
  DMA_Channelx->CNDTR=cnt; //DMAx,����������
  DMA_Cmd(DMA_Channelx, ENABLE);   
}




