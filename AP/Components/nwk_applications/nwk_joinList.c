/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   nwk_join_list.c
// Description: NWK APP.
//              本文件用于支持swsn的网络应用层Join过程中的许可列表.
// Author:      Leidi
// Version:     1.0
// Date:        2015-05-13
// History:     2015-05-13  Leidi 初始版本建立.
//              2015-08-31  Zengjia 修改一下节点:
//                          (1)Join许可列表的信息中添加节点IP地址
//                          (2)添加检测IP冲突函数nwk_joinList_IPConflict
//                          (3)添加分配IP地址函数nwk_joinList_assignIP
//                          (4)nwk_joinList_init中添加IP地址初始化
//                          (5)nwk_joinList_add中添加分配IP地址过程
//                          (6)nwk_joinList_save中添加向flash写入IP地址的过程                            
*****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "nwk_joinList.h"
#include "nwk.h"
#include "nwk_globals.h"
#include <string.h>
#include <stdlib.h>
/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
/* 已经许可Join的设备列表 */
static join_list_t join_list[JOIN_LIST_ITEMS_NUM];

/* 已经许可Join的设备数量 */
static uint8_t join_device_num;


/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static join_list_t *nwk_joinList_search(const addr_t *pDevivceAddr);


/**************************************************************************************************
 * @fn          nwk_joinList_init
 *
 * @brief       初始化许可Join的设备列表.初始化数据来自FLASH.
 *
 * @param       none.
 *
 * @return      none
 **************************************************************************************************
 */
void nwk_joinList_init(void)
{
  uint8_t i;
	uint8_t tmp[10] = {0};
  join_list_t *pRam_join_list = join_list;
  uint8_t *pFlash_join_list = (uint8_t *)JOIN_LIST_START_ADDR;
  addr_t uninit_addr = {0xff, 0xff, 0xff, 0xff};  //STM32 FLASH中未初始化的单元均为0xff
  
  /* 变量初始化 */
  memset(pRam_join_list, 0x00, sizeof(join_list_t) * JOIN_LIST_ITEMS_NUM);
  join_device_num = 0;
	
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"join list!"RTT_CTRL_RESET"\n");
#endif    
	
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    /* 如果FLASH中存储的内容不是未初始化的 */
    if(memcmp(&uninit_addr, (addr_t*)pFlash_join_list, NET_ADDR_SIZE))
    {
      /* 使用FLASH中存储的Join的设备列表初始化RAM变量 */
      memcpy(&(pRam_join_list->mac), pFlash_join_list, NET_MAC_ADDR_SIZE);
      memcpy(&(pRam_join_list->ip), pFlash_join_list+NET_MAC_ADDR_SIZE, NET_IP_ADDR_SIZE);
      pRam_join_list->state = JOIN_ADDR_IN_USE;
      join_device_num++;
#ifdef SEGGER_SYS_DEBUG    
			memcpy(tmp,pFlash_join_list,10);
			SEGGER_RTT_put_multi_char(tmp,10);	
			SWSN_DELAY(2);
#endif     
      pRam_join_list++;
    }
    pFlash_join_list += NET_MAC_ADDR_SIZE+NET_IP_ADDR_SIZE ;
  }
}

/**************************************************************************************************
 * @fn          nwk_joinList_deInit
 *
 * @brief       清空许可Join的设备列表.同时清空FLASH中的备份.
 *
 * @param       none.
 *
 * @return      none
 **************************************************************************************************
 */
void nwk_joinList_deInit(void)
{
  join_list_t *pRam_join_list = join_list;
  
  /* 全局变量初始化 */
  memset(pRam_join_list, 0x00, sizeof(join_list_t) * JOIN_LIST_ITEMS_NUM);
  join_device_num = 0;
  
  /* 清除FLASH中保存的Join的设备列表 */
  FLASH_EraseOnePage(JOIN_LIST_START_ADDR);
}

/**************************************************************************************************
 * @fn          nwk_joinList_add
 *
 * @brief       向许可Join的设备列表中添加一个设备地址.
 *
 * @param       pDevivceAddr - 指向待添加的设备地址
 *
 * @return      若执行此操作成功，返回许可Join的设备列表的可用容量。若执行失败，返回-1.
 **************************************************************************************************
 */
int8_t nwk_joinList_add(const mac_addr_t *pDeviceAddr)
{
  join_list_t *pJoin_list_item;
  ip_addr_t deviceIPAddr = {0}; 
  uint8_t i;
  int8_t ret = -1;

  /* 在Join的设备列表中搜索 */
  pJoin_list_item = nwk_joinList_search(&pDeviceAddr->macAddr);

  if(pJoin_list_item == NULL)//未找到对应的设备
  {
    /* 从头开始寻找空位 */
    pJoin_list_item = join_list;
    
    for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
    {
      if(pJoin_list_item->state == JOIN_ADDR_NULL) //空位
      {
        /*分配IP地址*/
        nwk_joinList_assignIP(&pDeviceAddr->macAddr,&deviceIPAddr);
        /* 将新信息写入此位置 */
        memcpy(&(pJoin_list_item->mac), pDeviceAddr, NET_MAC_ADDR_SIZE);
        memcpy(&(pJoin_list_item->ip), &deviceIPAddr, NET_IP_ADDR_SIZE);
        pJoin_list_item->state = JOIN_ADDR_IN_USE;
        join_device_num++;
        break;
      }
      pJoin_list_item++;
    }
    
    if(i < JOIN_LIST_ITEMS_NUM)
    {
      ret = join_device_num; //添加成功，返回设备列表的可用容量
    }
    else
    {
      ret = -1; //Join的设备列表已满
    }
  }
  else//要添加的设备已经存在
  {
    ret = -1;
  }
  
  return ret;
}

/**************************************************************************************************
 * @fn          nwk_joinList_del
 *
 * @brief       从许可Join的设备列表中删除一个设备地址.
 *
 * @param       pDevivceAddr - 指向待删除的设备地址
 *
 * @return      若执行此操作成功，返回许可Join的设备列表的可用容量。若执行失败，返回-1.
 **************************************************************************************************
 */
int8_t nwk_joinList_del(const addr_t *pDeviceAddr)
{
  join_list_t *pJoin_list_item;
  uint8_t i;
  int8_t ret = -1;

  /* 从头开始寻找空位 */
  pJoin_list_item = join_list;
  
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    if(pJoin_list_item->state == JOIN_ADDR_IN_USE) //正在使用的列表项
    {
      /* 找到了待删除的设备MAC */
      if(!memcmp(&(pJoin_list_item->mac.macAddr), pDeviceAddr, NET_ADDR_SIZE))
      {
        pJoin_list_item->state = JOIN_ADDR_NULL;
        join_device_num--;
        break;
      }
    }
    pJoin_list_item++;
  }
  
  if(i < JOIN_LIST_ITEMS_NUM)
  {
    ret = join_device_num; //删除成功，返回设备列表的可用容量
  }
  else
  {
    ret = -1; //在列表中未找到要删除的设备
  }
  
  return ret;
}

/**************************************************************************************************
 * @fn          nwk_joinList_save
 *
 * @brief       将RAM中的许可Join的设备列表保持到FLASH.
 *
 * @param       none.
 *
 * @return      none
 **************************************************************************************************
 */
void nwk_joinList_save(void)
{
  uint8_t i = 0, j = 0;
  uint16_t byte_num = 0;
  join_list_t *pRam_join_list = join_list;
  uint8_t temp_addr[JOIN_LIST_ITEMS_NUM*(NET_MAC_ADDR_SIZE+NET_IP_ADDR_SIZE)];
	
  /* STM32 FLASH中未初始化的单元均为0xFF(STM8为0x00) */
  memset(temp_addr, 0xff, sizeof(temp_addr));
  
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    if(pRam_join_list->state == JOIN_ADDR_IN_USE)  //只保存正在使用的设备地址
    {
              /* 将新信息写入此位置 */
//      memcpy(&temp_addr[byte_num], &(pRam_join_list->mac), NET_MAC_ADDR_SIZE);
//      byte_num += NET_MAC_ADDR_SIZE;
//      memcpy(&temp_addr[byte_num], &(pRam_join_list->ip), NET_IP_ADDR_SIZE);
//      byte_num += NET_IP_ADDR_SIZE;
      for(j=0; j<NET_ADDR_SIZE;j++)
      {
        temp_addr[byte_num++] = pRam_join_list->mac.macAddr.addr[j];
      }
      for(j=0; j<NET_MAC_TYPE_SIZE;j++)
      {
        temp_addr[byte_num++] = pRam_join_list->mac.macType.type[j];
      }
      for(j=0; j<NET_ED_IP_ADDR_SIZE;j++)
      {
        temp_addr[byte_num++] = pRam_join_list->ip.edAddr[j];
      }
      for(j=0; j<NET_AP_IP_ADDR_SIZE;j++)
      {
        temp_addr[byte_num++] = pRam_join_list->ip.apAddr[j];
      }
    }
    pRam_join_list++;
  }
  /* 将已经添加到设备列表的节点的MAC地址和IP地址写入FLASH */
  FLASH_WriteByte(JOIN_LIST_START_ADDR, (uint8_t *)temp_addr, byte_num);
}

/**************************************************************************************************
 * @fn          nwk_joinList_isFound
 *
 * @brief       判断提供的设备地址是否在许可Join的设备列表中.
 *
 * @param       pDevivceAddr - 指向提供的设备地址
 *
 * @return      若此地址在列表中，返回1.否则返回0.
 **************************************************************************************************
 */
uint8_t nwk_joinList_isFound(const addr_t *pDeviceAddr)
{
  join_list_t *pJoin_list_item;
  uint8_t ret = 0;

  /* 在Join的设备列表中搜索 */
  pJoin_list_item = nwk_joinList_search(pDeviceAddr);

  if(pJoin_list_item != NULL)//找到对应的设备
  {
    ret = 1;
  }
  
  return ret;
}

/**************************************************************************************************
 * @fn          nwk_joinList_get
 *
 * @brief       获取整个许可Join的设备列表.
 *
 * @param       none.
 *
 * @return      指向许可Join的设备列表的指针.
 **************************************************************************************************
 */
const join_list_t *nwk_joinList_get(void)
{
  return join_list;
}

/**************************************************************************************************
 * @fn          nwk_joinList_search
 *
 * @brief       使用给定的设备地址在许可Join的设备列表中查找对应的列表项.
 *
 * @param       待查找的设备地址.
 *
 * @return      指向对应的列表项的指针.若未找到，返回NULL.
 **************************************************************************************************
 */
static join_list_t *nwk_joinList_search(const addr_t *pDeviceAddr)
{
  uint8_t i;
  join_list_t *pJoin_list_item = join_list;
  
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    if(pJoin_list_item->state == JOIN_ADDR_IN_USE) //正在使用的列表项
    {
      if(!memcmp(&(pJoin_list_item->mac.macAddr), pDeviceAddr, NET_ADDR_SIZE))//此设备已经存在
      {
        return pJoin_list_item;
      }
    }
    pJoin_list_item++;
  }
  
  return NULL;
}


/**************************************************************************************************
 * @fn          nwk_joinList_assignIP
 *
 * @brief       分配IP地址.默认为MAC地址的后两位。如果冲突，以10为步长依次累加，
 *              直到不冲突为止。网络至少可以容纳6553个节点。            
 *
 * @param       pDevivceAddr  - 指向提供的设备地址
 *              pDeviceIPAddr - 指向提供设备的IP地址
 *
 * @return      成功，返回1.否则返回0.
 **************************************************************************************************
 */
uint8_t nwk_joinList_assignIP(const addr_t *pDeviceAddr,ip_addr_t *pDeviceIPAddr)
{
  uint8_t i;
  volatile uint16_t ip_addr_u16 = 0;
  ip_addr_t const *myDeviceIP = nwk_getMyIPAddress();
  
  ip_addr_u16 = pDeviceAddr->addr[0] | (pDeviceAddr->addr[1]<<8);
  for(i=0; i<JOIN_LIST_ITEMS_NUM;i++)
  {
    ip_addr_u16 += i*10;
    pDeviceIPAddr->edAddr[0] = ip_addr_u16;
    pDeviceIPAddr->edAddr[1] = (ip_addr_u16>>8);
    if(!nwk_joinList_IPConflict(pDeviceIPAddr))
    {
      break;
    }
  }
  pDeviceIPAddr->apAddr[0] = myDeviceIP->apAddr[0];
  pDeviceIPAddr->apAddr[1] = myDeviceIP->apAddr[1];
  
  return 1;
}

/**************************************************************************************************
 * @fn          nwk_joinList_IPConflict
 *
 * @brief       判断IP地址是否冲突。            
 *
 * @param       pDeviceList - 指向提供的设备的IP地址
 *
 * @return      冲突，返回1.否则返回0.
 **************************************************************************************************
 */
uint8_t nwk_joinList_IPConflict(const ip_addr_t*pDeviceIPAddr)
{
  uint8_t i = 0;
  join_list_t *pJoin_list_item = join_list;    
  
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    if(pJoin_list_item->state == JOIN_ADDR_IN_USE) //正在使用的列表项
    {
      //判断是否是输入设备
      if(!memcmp(pDeviceIPAddr->edAddr,pJoin_list_item->ip.edAddr,NET_ED_IP_ADDR_SIZE))
      {
          return 1;
      }
    }
    pJoin_list_item++;
  }  
  return 0;
}

/**************************************************************************************************
 * @fn          nwk_joinList_searchIP
 *
 * @brief       使用给定的设备地址在许可Join的设备列表中查找对应设备的IP地址.
 *
 * @param       pDeviceAddr   - 指向待查找的设备地址.
 *              pDeviceIPAddr - 指向返回的IP地址
 *
 * @return      成功返回1，失败返回0.
 **************************************************************************************************
 */
uint8_t nwk_joinList_searchIP(const addr_t *pDeviceAddr,ip_addr_t *pDeviceIPAddr)
{
  uint8_t i;
  join_list_t *pJoin_list_item = join_list;

  if(pDeviceAddr == NULL || pDeviceIPAddr == NULL)
    return 0;
  
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    if(pJoin_list_item->state == JOIN_ADDR_IN_USE) //正在使用的列表项
    {
      if(!memcmp(&(pJoin_list_item->mac), pDeviceAddr, NET_ADDR_SIZE))//此设备已经存在
      {
        memcpy((uint8_t*)pDeviceIPAddr, (uint8_t*)&pJoin_list_item->ip, NET_IP_ADDR_SIZE);
        return 1;
      }
    }
    pJoin_list_item++;
  }
  return 0;
}
/**************************************************************************************************
 */


/***************************************************************************
 * @fn          nwk_getAPandTotalEDInfor
 *     
 * @brief       获取集中器地址、所有节点的IP地址、MAC地址及状态，
 *              并依次存放到数组里面
 *     
 * @data        2016年02月24日
 *     
 * @param       pOutMsg - 存放AP和ED信息的数组
 *     
 * @return      
 ***************************************************************************
 */ 
//#define NEW_VERSION
uint16_t nwk_getAPandTotalEDInfor(uint8_t *pOutMsg)
{
  uint8_t i,j;
  uint8_t node_num = 0;
  uint16_t msg_len  = 0;
  addr_t const *  myromaddr = nwk_getMyAddress(); 
  const join_list_t * pJoinList = nwk_joinList_get();
  

#ifdef NEW_VERSION 
/*新版本程序，集中器 地址两个字节，接着是节点总数*/  
  for(i=0; i<2; i++)
     pOutMsg[i] = myromaddr->addr[i];//集中器地址
  
  pOutMsg[2] = 0;//节点总数
  msg_len += 3;   
#else
/*旧版本程序，集中器地址1个字节，接着是节点总数*/    
  for(i=0; i<1; i++)
     pOutMsg[i] = myromaddr->addr[i];//集中器地址
  
  pOutMsg[1] = 0;//节点总数
  msg_len += 2;
#endif  

  for(i=0; i<JOIN_LIST_ITEMS_NUM;i++)
  {
    if(pJoinList[i].state == JOIN_ADDR_IN_USE)
    {
      /* 节点MAC地址 */
      for(j=0; j<NET_ADDR_SIZE; j++)
      {
         pOutMsg[j + msg_len] = pJoinList[i].mac.macAddr.addr[j];
      } 
      msg_len += NET_ADDR_SIZE;
#ifdef NEW_VERSION      
      for(j=0; j<NET_MAC_TYPE_SIZE; j++)
      {
         pOutMsg[j + msg_len] = pJoinList[i].mac.macType.type[j];
      } 
      msg_len += NET_MAC_TYPE_SIZE;
#endif     
      /* 节点IP地址 */
      for(j=0; j<NET_ED_IP_ADDR_SIZE; j++)
      {
         pOutMsg[j+ msg_len] = pJoinList[i].ip.edAddr[j];
      }   
      msg_len += NET_ED_IP_ADDR_SIZE;
      /* 节点状态，默认为在线、不冲突 */
      pOutMsg[msg_len] = 0X02;
      
      msg_len ++;
      
      node_num ++;
    }
      
  }
#ifdef NEW_VERSION 
  pOutMsg[2] = node_num;//节点总数
#else
  pOutMsg[1] = node_num;//节点总数
#endif  
  return msg_len;
}


