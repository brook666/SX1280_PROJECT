/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   nwk_join_list.c
// Description: NWK APP.
//              ���ļ�����֧��swsn������Ӧ�ò�Join�����е�����б�.
// Author:      Leidi
// Version:     1.0
// Date:        2015-05-13
// History:     2015-05-13  Leidi ��ʼ�汾����.
//              2015-08-31  Zengjia �޸�һ�½ڵ�:
//                          (1)Join����б����Ϣ����ӽڵ�IP��ַ
//                          (2)��Ӽ��IP��ͻ����nwk_joinList_IPConflict
//                          (3)��ӷ���IP��ַ����nwk_joinList_assignIP
//                          (4)nwk_joinList_init�����IP��ַ��ʼ��
//                          (5)nwk_joinList_add����ӷ���IP��ַ����
//                          (6)nwk_joinList_save�������flashд��IP��ַ�Ĺ���                            
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
/* �Ѿ����Join���豸�б� */
static join_list_t join_list[JOIN_LIST_ITEMS_NUM];

/* �Ѿ����Join���豸���� */
static uint8_t join_device_num;


/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static join_list_t *nwk_joinList_search(const addr_t *pDevivceAddr);


/**************************************************************************************************
 * @fn          nwk_joinList_init
 *
 * @brief       ��ʼ�����Join���豸�б�.��ʼ����������FLASH.
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
  addr_t uninit_addr = {0xff, 0xff, 0xff, 0xff};  //STM32 FLASH��δ��ʼ���ĵ�Ԫ��Ϊ0xff
  
  /* ������ʼ�� */
  memset(pRam_join_list, 0x00, sizeof(join_list_t) * JOIN_LIST_ITEMS_NUM);
  join_device_num = 0;
	
#ifdef SEGGER_DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"join list!"RTT_CTRL_RESET"\n");
#endif    
	
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    /* ���FLASH�д洢�����ݲ���δ��ʼ���� */
    if(memcmp(&uninit_addr, (addr_t*)pFlash_join_list, NET_ADDR_SIZE))
    {
      /* ʹ��FLASH�д洢��Join���豸�б��ʼ��RAM���� */
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
 * @brief       ������Join���豸�б�.ͬʱ���FLASH�еı���.
 *
 * @param       none.
 *
 * @return      none
 **************************************************************************************************
 */
void nwk_joinList_deInit(void)
{
  join_list_t *pRam_join_list = join_list;
  
  /* ȫ�ֱ�����ʼ�� */
  memset(pRam_join_list, 0x00, sizeof(join_list_t) * JOIN_LIST_ITEMS_NUM);
  join_device_num = 0;
  
  /* ���FLASH�б����Join���豸�б� */
  FLASH_EraseOnePage(JOIN_LIST_START_ADDR);
}

/**************************************************************************************************
 * @fn          nwk_joinList_add
 *
 * @brief       �����Join���豸�б������һ���豸��ַ.
 *
 * @param       pDevivceAddr - ָ�����ӵ��豸��ַ
 *
 * @return      ��ִ�д˲����ɹ����������Join���豸�б�Ŀ�����������ִ��ʧ�ܣ�����-1.
 **************************************************************************************************
 */
int8_t nwk_joinList_add(const mac_addr_t *pDeviceAddr)
{
  join_list_t *pJoin_list_item;
  ip_addr_t deviceIPAddr = {0}; 
  uint8_t i;
  int8_t ret = -1;

  /* ��Join���豸�б������� */
  pJoin_list_item = nwk_joinList_search(&pDeviceAddr->macAddr);

  if(pJoin_list_item == NULL)//δ�ҵ���Ӧ���豸
  {
    /* ��ͷ��ʼѰ�ҿ�λ */
    pJoin_list_item = join_list;
    
    for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
    {
      if(pJoin_list_item->state == JOIN_ADDR_NULL) //��λ
      {
        /*����IP��ַ*/
        nwk_joinList_assignIP(&pDeviceAddr->macAddr,&deviceIPAddr);
        /* ������Ϣд���λ�� */
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
      ret = join_device_num; //��ӳɹ��������豸�б�Ŀ�������
    }
    else
    {
      ret = -1; //Join���豸�б�����
    }
  }
  else//Ҫ��ӵ��豸�Ѿ�����
  {
    ret = -1;
  }
  
  return ret;
}

/**************************************************************************************************
 * @fn          nwk_joinList_del
 *
 * @brief       �����Join���豸�б���ɾ��һ���豸��ַ.
 *
 * @param       pDevivceAddr - ָ���ɾ�����豸��ַ
 *
 * @return      ��ִ�д˲����ɹ����������Join���豸�б�Ŀ�����������ִ��ʧ�ܣ�����-1.
 **************************************************************************************************
 */
int8_t nwk_joinList_del(const addr_t *pDeviceAddr)
{
  join_list_t *pJoin_list_item;
  uint8_t i;
  int8_t ret = -1;

  /* ��ͷ��ʼѰ�ҿ�λ */
  pJoin_list_item = join_list;
  
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    if(pJoin_list_item->state == JOIN_ADDR_IN_USE) //����ʹ�õ��б���
    {
      /* �ҵ��˴�ɾ�����豸MAC */
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
    ret = join_device_num; //ɾ���ɹ��������豸�б�Ŀ�������
  }
  else
  {
    ret = -1; //���б���δ�ҵ�Ҫɾ�����豸
  }
  
  return ret;
}

/**************************************************************************************************
 * @fn          nwk_joinList_save
 *
 * @brief       ��RAM�е����Join���豸�б��ֵ�FLASH.
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
	
  /* STM32 FLASH��δ��ʼ���ĵ�Ԫ��Ϊ0xFF(STM8Ϊ0x00) */
  memset(temp_addr, 0xff, sizeof(temp_addr));
  
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    if(pRam_join_list->state == JOIN_ADDR_IN_USE)  //ֻ��������ʹ�õ��豸��ַ
    {
              /* ������Ϣд���λ�� */
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
  /* ���Ѿ���ӵ��豸�б�Ľڵ��MAC��ַ��IP��ַд��FLASH */
  FLASH_WriteByte(JOIN_LIST_START_ADDR, (uint8_t *)temp_addr, byte_num);
}

/**************************************************************************************************
 * @fn          nwk_joinList_isFound
 *
 * @brief       �ж��ṩ���豸��ַ�Ƿ������Join���豸�б���.
 *
 * @param       pDevivceAddr - ָ���ṩ���豸��ַ
 *
 * @return      ���˵�ַ���б��У�����1.���򷵻�0.
 **************************************************************************************************
 */
uint8_t nwk_joinList_isFound(const addr_t *pDeviceAddr)
{
  join_list_t *pJoin_list_item;
  uint8_t ret = 0;

  /* ��Join���豸�б������� */
  pJoin_list_item = nwk_joinList_search(pDeviceAddr);

  if(pJoin_list_item != NULL)//�ҵ���Ӧ���豸
  {
    ret = 1;
  }
  
  return ret;
}

/**************************************************************************************************
 * @fn          nwk_joinList_get
 *
 * @brief       ��ȡ�������Join���豸�б�.
 *
 * @param       none.
 *
 * @return      ָ�����Join���豸�б��ָ��.
 **************************************************************************************************
 */
const join_list_t *nwk_joinList_get(void)
{
  return join_list;
}

/**************************************************************************************************
 * @fn          nwk_joinList_search
 *
 * @brief       ʹ�ø������豸��ַ�����Join���豸�б��в��Ҷ�Ӧ���б���.
 *
 * @param       �����ҵ��豸��ַ.
 *
 * @return      ָ���Ӧ���б����ָ��.��δ�ҵ�������NULL.
 **************************************************************************************************
 */
static join_list_t *nwk_joinList_search(const addr_t *pDeviceAddr)
{
  uint8_t i;
  join_list_t *pJoin_list_item = join_list;
  
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    if(pJoin_list_item->state == JOIN_ADDR_IN_USE) //����ʹ�õ��б���
    {
      if(!memcmp(&(pJoin_list_item->mac.macAddr), pDeviceAddr, NET_ADDR_SIZE))//���豸�Ѿ�����
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
 * @brief       ����IP��ַ.Ĭ��ΪMAC��ַ�ĺ���λ�������ͻ����10Ϊ���������ۼӣ�
 *              ֱ������ͻΪֹ���������ٿ�������6553���ڵ㡣            
 *
 * @param       pDevivceAddr  - ָ���ṩ���豸��ַ
 *              pDeviceIPAddr - ָ���ṩ�豸��IP��ַ
 *
 * @return      �ɹ�������1.���򷵻�0.
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
 * @brief       �ж�IP��ַ�Ƿ��ͻ��            
 *
 * @param       pDeviceList - ָ���ṩ���豸��IP��ַ
 *
 * @return      ��ͻ������1.���򷵻�0.
 **************************************************************************************************
 */
uint8_t nwk_joinList_IPConflict(const ip_addr_t*pDeviceIPAddr)
{
  uint8_t i = 0;
  join_list_t *pJoin_list_item = join_list;    
  
  for(i = 0; i < JOIN_LIST_ITEMS_NUM; i++)
  {
    if(pJoin_list_item->state == JOIN_ADDR_IN_USE) //����ʹ�õ��б���
    {
      //�ж��Ƿ��������豸
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
 * @brief       ʹ�ø������豸��ַ�����Join���豸�б��в��Ҷ�Ӧ�豸��IP��ַ.
 *
 * @param       pDeviceAddr   - ָ������ҵ��豸��ַ.
 *              pDeviceIPAddr - ָ�򷵻ص�IP��ַ
 *
 * @return      �ɹ�����1��ʧ�ܷ���0.
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
    if(pJoin_list_item->state == JOIN_ADDR_IN_USE) //����ʹ�õ��б���
    {
      if(!memcmp(&(pJoin_list_item->mac), pDeviceAddr, NET_ADDR_SIZE))//���豸�Ѿ�����
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
 * @brief       ��ȡ��������ַ�����нڵ��IP��ַ��MAC��ַ��״̬��
 *              �����δ�ŵ���������
 *     
 * @data        2016��02��24��
 *     
 * @param       pOutMsg - ���AP��ED��Ϣ������
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
/*�°汾���򣬼����� ��ַ�����ֽڣ������ǽڵ�����*/  
  for(i=0; i<2; i++)
     pOutMsg[i] = myromaddr->addr[i];//��������ַ
  
  pOutMsg[2] = 0;//�ڵ�����
  msg_len += 3;   
#else
/*�ɰ汾���򣬼�������ַ1���ֽڣ������ǽڵ�����*/    
  for(i=0; i<1; i++)
     pOutMsg[i] = myromaddr->addr[i];//��������ַ
  
  pOutMsg[1] = 0;//�ڵ�����
  msg_len += 2;
#endif  

  for(i=0; i<JOIN_LIST_ITEMS_NUM;i++)
  {
    if(pJoinList[i].state == JOIN_ADDR_IN_USE)
    {
      /* �ڵ�MAC��ַ */
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
      /* �ڵ�IP��ַ */
      for(j=0; j<NET_ED_IP_ADDR_SIZE; j++)
      {
         pOutMsg[j+ msg_len] = pJoinList[i].ip.edAddr[j];
      }   
      msg_len += NET_ED_IP_ADDR_SIZE;
      /* �ڵ�״̬��Ĭ��Ϊ���ߡ�����ͻ */
      pOutMsg[msg_len] = 0X02;
      
      msg_len ++;
      
      node_num ++;
    }
      
  }
#ifdef NEW_VERSION 
  pOutMsg[2] = node_num;//�ڵ�����
#else
  pOutMsg[1] = node_num;//�ڵ�����
#endif  
  return msg_len;
}


