/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   stm32_flash.c
// Description: BSP.
//              STM32 FLASH��д��װ.
// Author:      Leidi
// Version:     1.0
// Date:        2014-11-22
// History:     2014-11-22  Leidi ��ʼ�汾����.�ο�����:http://www.51hei.com/mcu/2766.html
//              2014-12-02  Leidi ��������FLASH_EraseOnePage().
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "stm32_flash.h"
#include "flash_start_addr.h"
#include "smpl_config.h"
BootLoaderState bootloader_state ;

/**************************************************************************************************
 * @fn          FLASH_WriteByte
 *
 * @brief       ��ָ��FLASH��ַд����ֽ�����.
 *
 * @param       addr      - ��FLASH��д�����ݵ���ʼ��ַ.ע��:�õ�ַ����ҳ(page)�ϵ����ݽ��ᱻȫ������.
 *              pByte     - ��д������ĵ�ַ(������ż��)
 *              byte_num  - ��д��������ֽ�����������ż����
 *
 * @return      none
 **************************************************************************************************
 */
void FLASH_WriteByte(uint32_t addr , uint8_t *pByte , uint16_t byte_num)
{
  uint16_t halfWord;
  uint16_t halfWord_num = (byte_num+1)/2; 
  uint8_t is_odd_flag = 0;  
  uint8_t   intState;  

  if(byte_num%2)
  {
    is_odd_flag = 1;
  }
  else
  {
    is_odd_flag = 0; 
  }
  
  intState = OS_CPU_SR_Save();
  /* ����FLASH */
  FLASH_Unlock();
  
  /* ������й����־λ */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  
  /* ����FLASHҳ */
  FLASH_ErasePage(addr);
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif  
  while(halfWord_num--)
  {
    /* ���ɰ���. ע��:STM32ΪС��ģʽ,���ȴ洢���ֽ��ٴ洢���ֽ� */
    if((halfWord_num == 0) && (is_odd_flag == 1))
    {
      halfWord = *(pByte++); 
      halfWord |= 0x00ff << 8;      
    }
    else
    {
      halfWord = *(pByte++);
      halfWord |= *(pByte++) << 8;
    }
    
    /* д����� */
    FLASH_ProgramHalfWord(addr, halfWord);
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif    
    /* ����д�����ʼ��ַ */
    addr += 2;
  }
  
  /* ����FLASH */
  FLASH_Lock();
  OS_CPU_SR_Restore(intState);
}

/**************************************************************************************************
 * @fn          FLASH_ReadByte
 *
 * @brief       ��ָ��FLASH��ַ��ȡ���ֽ�����.
 *
 * @param       addr      - ��FLASH�ж�ȡ���ݵ���ʼ��ַ
 *              pByte     - ��ȡ��Ҫд������ĵ�ַ
 *              byte_num  - Ҫ��ȡ���ֽ���
 *
 * @return      none
 **************************************************************************************************
 */
void FLASH_ReadByte(uint32_t addr, uint8_t *pByte, uint16_t byte_num)
{
  while(byte_num--)
  {
    *(pByte++) = *((uint8_t *)addr++);
  }
}

/**************************************************************************************************
 * @fn          FLASH_EraseOnePage
 *
 * @brief       ����ָ��FLASH��ַ���ڵ�ҳ(page).ע��:�õ�ַ����ҳ(page)�ϵ����ݽ��ᱻȫ������.
 *
 * @param       addr  - Ҫ������ҳ���ڵĵ�ַ.
 *
 * @return      none
 **************************************************************************************************
 */
void FLASH_EraseOnePage(uint32_t addr)
{
  /* ����FLASH */
  FLASH_Unlock();
  
  /* ������й����־λ */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  
  /* ����FLASHҳ */
  FLASH_ErasePage(addr);
  
  /* ����FLASH */
  FLASH_Lock();
}

/***************************************************************************
 * @fn          Set_BootLoader_State
 *     
 * @brief       ����bootloader״̬�������Ƿ���й̼�����
 *     
 * @data        2016��03��04��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void Set_BootLoader_State(void)
{
  uint8_t   i,intState;
  uint16_t  flash_buf[3];
  
  flash_buf[0] =  bootloader_state.STATE;
  flash_buf[1] =  bootloader_state.LENGTH;
  flash_buf[2] =  bootloader_state.LENGTH>>16;
  
  intState = OS_CPU_SR_Save();
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
  FLASH_ErasePage(SWSN_APP_FRESH_FLAG);
  for(i=0;i<3;i++)
  {
    FLASH_ProgramHalfWord(SWSN_APP_FRESH_FLAG +i*2,flash_buf[i]);    
  }
  FLASH_Lock();  
  OS_CPU_SR_Restore(intState);    
}

/***************************************************************************
 * @fn          void Read_BootLoader_State(void)
 *     
 * @brief       ��ȡbootloader״̬
 *     
 * @data        2016��03��04��
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void Read_BootLoader_State(void)
{  
  bootloader_state.STATE  = *(u16 *)(SWSN_APP_FRESH_FLAG);
  bootloader_state.LENGTH = *(u32 *)(SWSN_APP_FRESH_FLAG+2);
}

/**************************************************************************************************
 */


