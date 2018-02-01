/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   stm32_flash.c
// Description: BSP.
//              STM32 FLASH读写封装.
// Author:      Leidi
// Version:     1.0
// Date:        2014-11-22
// History:     2014-11-22  Leidi 初始版本建立.参考资料:http://www.51hei.com/mcu/2766.html
//              2014-12-02  Leidi 新增函数FLASH_EraseOnePage().
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
 * @brief       向指定FLASH地址写入多字节数据.
 *
 * @param       addr      - 向FLASH中写入数据的起始地址.注意:该地址所在页(page)上的数据将会被全部擦除.
 *              pByte     - 被写入变量的地址(必须是偶数)
 *              byte_num  - 被写入变量的字节数（必须是偶数）
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
  /* 解锁FLASH */
  FLASH_Unlock();
  
  /* 清除所有挂起标志位 */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  
  /* 擦除FLASH页 */
  FLASH_ErasePage(addr);
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif  
  while(halfWord_num--)
  {
    /* 生成半字. 注意:STM32为小端模式,即先存储低字节再存储高字节 */
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
    
    /* 写入半字 */
    FLASH_ProgramHalfWord(addr, halfWord);
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif    
    /* 调整写入的起始地址 */
    addr += 2;
  }
  
  /* 锁定FLASH */
  FLASH_Lock();
  OS_CPU_SR_Restore(intState);
}

/**************************************************************************************************
 * @fn          FLASH_ReadByte
 *
 * @brief       从指定FLASH地址读取多字节数据.
 *
 * @param       addr      - 从FLASH中读取数据的起始地址
 *              pByte     - 读取后要写入变量的地址
 *              byte_num  - 要读取的字节数
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
 * @brief       擦除指定FLASH地址所在的页(page).注意:该地址所在页(page)上的数据将会被全部擦除.
 *
 * @param       addr  - 要擦除的页所在的地址.
 *
 * @return      none
 **************************************************************************************************
 */
void FLASH_EraseOnePage(uint32_t addr)
{
  /* 解锁FLASH */
  FLASH_Unlock();
  
  /* 清除所有挂起标志位 */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  
  /* 擦除FLASH页 */
  FLASH_ErasePage(addr);
  
  /* 锁定FLASH */
  FLASH_Lock();
}

/***************************************************************************
 * @fn          Set_BootLoader_State
 *     
 * @brief       设置bootloader状态，决定是否进行固件更新
 *     
 * @data        2016年03月04日
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
 * @brief       读取bootloader状态
 *     
 * @data        2016年03月04日
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


