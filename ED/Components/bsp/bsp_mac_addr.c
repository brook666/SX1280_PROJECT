/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   addr_control_fun.c
// Description: User APP.
//              �豸��ַ����ӿ�.
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
/* FLASH�б������Ϣλ��(ע�ⵥƬ���ͺ�) */
#define MY_MAC_ADDR_PTR       0x0801a000   //MAC��ַ(4�ֽ�)
#define MOUDLE_INFO_PTR       0x0801a004   //ģ����Ϣ(2�ֽ�)

/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8_t sSwsnMacAddr[MAC_ADDR_SIZE] = THIS_DEVICE_ADDRESS;
/**************************************************************************************************
 * @fn          Load_MacAddress
 *
 * @brief       ����FLASH�е�MAC��ַ.���FLASH�е�IP��ַ������,���޲���.�����ڸ�������ʼ��ǰ����.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void Load_MacAddress(void)
{
  if(MacAddressInFlashIsValid())//FLASH�еı�����ַ��Ч
  {
    UseMacAddressInFlash();//ʹ��FLASH�еı�����ַ
  }
}

/**************************************************************************************************
 * @fn          MacAddressInFlashIsValid
 *
 * @brief       �ж�FLASH�еı�����ַ�Ƿ���Ч.
 *
 * @param       none
 *
 * @return      0 - ��Ч����ַ������
 *              1 - ��Ч��ַ
 **************************************************************************************************
 */
uint8_t MacAddressInFlashIsValid(void)
{
  uint8_t addr[MAC_ADDR_SIZE];
  /* ������ַ����Ϊ0x00��0xFFFFFFFF */
  
  if((*(uint32_t *)MY_MAC_ADDR_PTR == 0x00) || *(uint32_t *)MY_MAC_ADDR_PTR == 0xFFFFFFFF)
  {
     return 0;
  }

  memset(&addr, 0x0, sizeof(addr));

  return memcmp((const uint8_t *)MY_MAC_ADDR_PTR, &addr, MAC_ADDR_SIZE);//FLASH�еı�����ַ��Ϊ0����Ч
}

/**************************************************************************************************
 * @fn          UseMacAddressInFlash
 *
 * @brief       ����FLASH�еı�����ַ.
 *
 * @param       none
 *
 * @return      1 - �����ɹ�;2 - ����ʧ��
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
 * @brief       ��FLASH��д�뱾����ַ.
 *
 * @param       ָ��Ҫд��ı�����ַ��ָ��
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
 * @brief       ���FLASH�еı�����ַ.
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
 * @brief       ��FLASH��д�뱾ģ����Ϣ.
 *
 * @param       ָ��Ҫд��ı�ģ���ָ��
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
 * @brief       ���FLASH�еı�����ַ.
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
 * @brief       ��ȡ��ģ����Ϣ.
 *
 * @param       0,  - ��ȡ��˾����
 *              1,  - ��ȡģ������
 *
 * @return      ��ȡ����Ϣֵ
 **************************************************************************************************
 */
uint8_t GetMyModuleInfo(uint8_t item)
{
  uint8_t ret = 0;
  uint8_t company, type;
  
  company    = *(uint8_t *)MOUDLE_INFO_PTR;
  type       = *(uint8_t *)(MOUDLE_INFO_PTR + 1);
  
  if(0 == item)//��ȡ��˾����
  {
    ret = company;
  }
  else if(1 == item)//��ȡģ������
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
