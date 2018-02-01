/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   addr_control_fun.c
// Description: User APP.
//              设备地址管理接口.
// Author:      Leidi
// Version:     3.0
// Date:        2012-9-30
// History:     
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp_mac_addr.h"
#include "smpl_config.h"
#include <string.h>


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
/* FLASH中保存的信息位置(注意单片机型号) */
#define MY_MAC_ADDR_PTR       0x0801a000   //MAC地址(4字节)
#define MOUDLE_INFO_PTR       0x0801a004   //模块信息(2字节)

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8_t sSwsnMacAddr[MAC_ADDR_SIZE] = THIS_DEVICE_ADDRESS;
/**************************************************************************************************
 * @fn          Load_MacAddress
 *
 * @brief       加载FLASH中的MAC地址.如果FLASH中的IP地址不可用,则无操作.必须在个域网初始化前运行.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Load_MacAddress(void)
{
  if(MacAddressInFlashIsValid())//FLASH中的本机地址有效
  {
    UseMacAddressInFlash();//使用FLASH中的本机地址
  }
}

/**************************************************************************************************
 * @fn          MacAddressInFlashIsValid
 *
 * @brief       判断FLASH中的本机地址是否有效.
 *
 * @param       none
 *
 * @return      0 - 无效，地址不存在
 *              1 - 有效地址
 **************************************************************************************************
 */
uint8_t MacAddressInFlashIsValid(void)
{
  uint8_t addr[MAC_ADDR_SIZE];
  /* 本机地址不能为0x00或0xFFFFFFFF */
  
  if((*(uint32_t *)MY_MAC_ADDR_PTR == 0x00) || *(uint32_t *)MY_MAC_ADDR_PTR == 0xFFFFFFFF)
  {
     return 0;
  }

  memset(&addr, 0x0, sizeof(addr));

  return memcmp((const uint8_t *)MY_MAC_ADDR_PTR, &addr, MAC_ADDR_SIZE);//FLASH中的本机地址若为0则无效
}

/**************************************************************************************************
 * @fn          UseMacAddressInFlash
 *
 * @brief       加载FLASH中的本机地址.
 *
 * @param       none
 *
 * @return      1 - 操作成功;2 - 操作失败
 **************************************************************************************************
 */
uint8_t UseMacAddressInFlash(void)
{
  int i = 0;

  for(i=0;i<MAC_ADDR_SIZE;i++)
  {
    sSwsnMacAddr[i] = *((uint8_t *)MY_MAC_ADDR_PTR+i);
  }
  return 1;
}

/**************************************************************************************************
 * @fn          WriteMacAddressToFlash
 *
 * @brief       在FLASH中写入本机地址.
 *
 * @param       指向要写入的本机地址的指针
 *
 * @return      none
 **************************************************************************************************
 */
void WriteMacAddressToFlash(const uint8_t *myAddr)
{
  
  FLASH_WriteByte(MY_MAC_ADDR_PTR,(uint8_t *)myAddr,MAC_ADDR_SIZE);
}

/**************************************************************************************************
 * @fn          CleanMacAddressInFlash
 *
 * @brief       清除FLASH中的本机地址.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void CleanMacAddressInFlash(void)
{
  uint8_t addr[MAC_ADDR_SIZE];
  
  memset(&addr, 0x0, sizeof(addr));
    
  FLASH_WriteByte(MY_MAC_ADDR_PTR,addr,MAC_ADDR_SIZE);
}

/**************************************************************************************************
 * @fn          WriteModuleInfoToFlash
 *
 * @brief       在FLASH中写入本模块信息.
 *
 * @param       指向要写入的本模块的指针
 *
 * @return      none
 **************************************************************************************************
 */
void WriteModuleInfoToFlash(const uint8_t *myModuleInfo)
{
  FLASH_WriteByte(MOUDLE_INFO_PTR,(uint8_t *)myModuleInfo,MAC_ADDR_SIZE+2);
}

/**************************************************************************************************
 * @fn          CleanModuleInfoInFlash
 *
 * @brief       清除FLASH中的本机地址.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void CleanModuleInfoInFlash(void)
{
  uint8_t myModuleInfo[6] = {0};
    
  FLASH_WriteByte(MOUDLE_INFO_PTR,myModuleInfo,MAC_ADDR_SIZE+2);
}
/**************************************************************************************************
 * @fn          GetMyModuleInfo
 *
 * @brief       获取本模块信息.
 *
 * @param       0,  - 获取公司代号
 *              1,  - 获取模块类型
 *
 * @return      获取的信息值
 **************************************************************************************************
 */
uint8_t GetMyModuleInfo(uint8_t item)
{
  uint8_t ret = 0;
  uint8_t company, type;
  
  company    = *(uint8_t *)MOUDLE_INFO_PTR;
  type       = *(uint8_t *)(MOUDLE_INFO_PTR + 1);
  
  if(0 == item)//获取公司代号
  {
    ret = company;
  }
  else if(1 == item)//获取模块类型
  {
    ret = type;
  }
  
  return ret;
}


uint8_t *bsp_getMacAddr(void)
{
  return sSwsnMacAddr;
}
/**************************************************************************************************
 */
