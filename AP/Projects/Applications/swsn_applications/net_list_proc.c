/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_list_proc.c
// Description: User APP.
//              �ڵ���Ϣ������.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-25
// History:     2014-10-25  Leidi ��Ӻ���Modify_IP_by_MAC().
//              2014-10-28  Leidi �޸ĺ���add_net_list(),�����м������ӵĽڵ�IP�Ƿ��ͻ,������status�ֶ�.
//                                �޸ĺ���init_list(),head_list_to_flash(),flash_to_head_list(),status�ֶδ�FLASH�д�ȡ.
//                                ɾ�����ú���pdel_ip(),modify_ip().
//                                ʹ�ú���Modify_IP_by_MAC()�޸�IPʱ,���status�ֶ��е�IP��ͻ��־λ.
//                                ʹ�ú���pdel_mac()ɾ���ڵ�ʱ,���ú���Free_tx_ed_buffer()�ͷ���Ӧ�Ľڵ㷢�ͻ�����.
//              2014-10-29  Leidi ��Ӻ���detect_ip_conflict().���������add_net_list(),pdel_mac(),Modify_IP_by_MAC().
//              2014-11-03  Leidi �ں���init_list()�е���detect_ip_conflict().�Խ��ÿ�ο�����IP��ͻ��־λ����ȷ������.
//              2014-11-24  Leidi �ڳ�ʼ��������init_list()��,��д���ŵ��ŵĺ���channr_num_to_flash()ʹ��FREQUENCY_AGILITY
//                                ��������.ʹ֮�ڲ�����FREQUENCY_AGILITYʱ����д���ŵ���.
//              2015-09-01  Zengjia �޸�add_net_list(),����IP��ַ�������join���б���䡣 
********************************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdlib.h>
#include <string.h>
#include "bsp.h"
#include "nwk_layer.h"
#include "net_list_proc.h"
#include "ed_tx_rx_buffer.h"
#include "flash_start_addr.h"
/* ------------------------------------------------------------------------------------------------
 *                                            Global Variables
 * ------------------------------------------------------------------------------------------------
 */
//uint16_t sys_data[1024];
//uint16_t head_data[500];
uint16_t list_ind_data;
uint16_t flash_ed_num;

extern  uint8_t net_ed_num;


struct ip_mac_id *head_list = NULL;
struct ip_mac_id *tail_list = NULL;

/* ------------------------------------------------------------------------------------------------
 *                                            Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static uint8_t detect_ip_conflict(void);
static uint8_t compare_listNode_insertNode(struct ip_mac_id *,addr_t *,ip_addr_t ,linkID_t );

//��ʼ������
/***************************************************************************
 * @fn          init_list
 *     
 * @brief       ��ʼ������falsh��û������ʱ���������еĳ�ʼ������ ��Ϣд��
 *              flash��flash��������ʱ����flash�ж������ݣ����ø����ݳ�ʼ��
 *              ����,�����ʼ�����֮�󣬶Խڵ�IP��ַ���г�ͻ��⡣
 *     
 * @data        2015��08��07��
 *     
 * @param       void
 *     
 * @return      1
 ***************************************************************************
 */ 
uint8_t init_list(void)
{ 
  uint8_t i;
  struct ip_mac_id *p=head_list;
  head_list=(struct ip_mac_id *)malloc(sizeof(struct ip_mac_id));
  tail_list=(struct ip_mac_id *)malloc(sizeof(struct ip_mac_id));  
//  tail_list = pnext;
	
#ifdef DEBUG
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_RED"Init List!"RTT_CTRL_RESET"\n");
#endif

  if( *(u16 *) LIST_IND_PTR !=0x5a5a)
  { 
    list_ind_data =0x5a5a;
    flash_ed_num = 2;
    for(i=0;i<4;i++)
    {
      head_list->ip[i] = 0xaa;
      head_list->mac[i] = 0x00;
      head_list->id = 0x00;
      head_list->status = 0x00;
    }
    p=head_list;  

    for(i=0;i<4;i++)
    {
      tail_list->ip[i] = 0xff;
      tail_list->mac[i] = 0x00;
      tail_list->id = 0xff;
      tail_list->status = 0x00;
    }
    p->next=tail_list; 
    p=tail_list; 
    p->next=NULL;
   
  //  p=head_list;

#ifdef FREQUENCY_AGILITY
    channr_num_to_flash(0x00);
#endif
    sysinfo_to_flash();
    head_list_to_flash(head_list);
  }
  else
  {
    struct ip_mac_id *pnew = NULL ;
    list_ind_data =0x5a5a;
    
    /* ��FLASH�ж�ȡ������Ϣ */
    sysinfo_from_flash();
    
    /* ��FLASH�ж�ȡ�ڵ���Ϣ���� */
    flash_to_head_list(head_list);
    
    /* ��������е�IP��ͻ��� */
    detect_ip_conflict();
    
    /* �ҵ������βָ�� */
    pnew = head_list;
    while(pnew!=NULL)
    {
      if(pnew->next == NULL)
        tail_list = pnew;
        pnew = pnew->next;
    }
   }
#ifdef SEGGER_SYS_DEBUG
  SEGGER_RTT_print_all_ed_info(head_list);
	SEGGER_RTT_print_sysinfo();
#endif
	 
  return 1;
}


/***************************************************************************
 * @fn          ip_comp
 *     
 * @brief       �Ƚ�IP��ַ
 *     
 * @data        2015��08��07��
 *     
 * @param       node - �����нڵ���Ϣ
 *              ip   - ֡�нڵ�IP
 *     
 * @return      1 , ƥ��ɹ�
 *              0 , ƥ��ʧ��
 ***************************************************************************
 */ 
uint8_t ip_comp(struct ip_mac_id *node,uint16_t *ip)
{
  uint8_t i;
  uint8_t ip_addr[2];
  ip_addr[0] = *ip;
  ip_addr[1] = *ip >>8;
  for(i=0;i<2;i++)
    if(node->ip[i]!=ip_addr[i])
      return 0;
  if(i==2)
    return 1;
  else
    return 0;
}

/***************************************************************************
 * @fn          mac_comp(���ڲ���)
 *     
 * @brief       �Ƚ�MAC��ַ
 *     
 * @data        2015��08��07��
 *     
 * @param       node - �����нڵ���Ϣ
 *              mac  - ֡�нڵ�MAC
 *     
 * @return      
 ***************************************************************************
 */ 
uint8_t mac_comp(struct ip_mac_id *node,uint8_t *mac)
{
  uint8_t i;
  for(i=0;i<4;i++)
    if(node->mac[i]!=*mac++)
      return 0;
  if(i==4)
    return 1;
  else 
    return 0;
}

/***************************************************************************
 * @fn          add_net_list
 *     
 * @brief       ������β����������ڵ���Ϣ����д��flash��
 *     
 * @data        2015��08��07��
 *     
 * @param       linkid - �ڵ�ID��
 *     
 * @return      1
 ***************************************************************************
 */ 
uint8_t add_net_list(uint8_t linkid)
{
  uint8_t i = 0;
  struct ip_mac_id *pnew;
  struct ip_mac_id *pnext;
  
  ip_addr_t edIPAddr = {0};
  
  addr_t *ed_info= nwk_getAddressByLinkId(linkid); 
  
#ifdef DEBUG
    put_string_uart1("add_net_list!!!\n ");
#endif
  if(linkid == 0)
    return 0;
  
	//�����������Ƿ���ڸýڵ����Ϣ��������ڽڵ���Ϣ�����޸Ľڵ���Ϣ
	if(psearch_mac(ed_info->addr, &pnew))
	{
		nwk_joinList_searchIP(ed_info,&edIPAddr);
		for(i=0;i<4;i++)
		{
			if(i<2)
			{
				pnew->ip[i] = edIPAddr.edAddr[i];
			}
			else
			{
				pnew->ip[i] = edIPAddr.apAddr[i-2];
			}
			pnew->mac[i] = ed_info->addr[i];
		}
		pnew->id = linkid; 
		pnew->status = 0;
	}//��������û���ҵ��ýڵ����Ϣ������ӽڵ�
	else
	{
		
		flash_ed_num++;
		
		nwk_joinList_searchIP(ed_info,&edIPAddr);
		
		pnew=head_list;
		
		/* �ҵ������β�� */
		while(pnew->next != NULL)
		{
			pnew=pnew->next;
		}
		
		/* �����µ�����ڵ� */
		pnext=(struct ip_mac_id *)malloc(sizeof(struct ip_mac_id)); 
		for(i=0;i<4;i++)
		{
			if(i<2)
			{
				pnext->ip[i] = edIPAddr.edAddr[i];
			}
			else
			{
				pnext->ip[i] = edIPAddr.apAddr[i-2];
			}
			pnext->mac[i] = ed_info->addr[i];
		}
		pnext->id = linkid; 
		pnext->status = 0;
		
		/* ���µ�����ڵ���ӵ�����β�� */
		pnew->next=pnext;
		pnew->next->next=NULL;
		tail_list = pnext;
	}
  
  /* ��������е�IP��ͻ��� */
  detect_ip_conflict();
  
  head_list_to_flash(head_list);
  sysinfo_to_flash();
  
  return 1;
}

/***************************************************************************
 * @fn          pdel_mac
 *     
 * @brief       ͨ��MAC��ַɾ���ڵ�
 *     
 * @data        2015��08��07��
 *     
 * @param       mac - �ڵ�MAC��ַ
 *     
 * @return      1 , �ɹ�
 *              0 , ʧ��
 ***************************************************************************
 */ 
uint8_t pdel_mac(uint8_t *mac)
{
  struct ip_mac_id  *p  =  NULL;
  struct ip_mac_id  *p_temp  =  NULL;
  uint16_t ip_addr;
	uint8_t tmpLinkId = 0;
  
  if(!psearch_mac(mac, &p))
  {
    return 0;
  }
  else
  { 
    if(p== head_list) // it never happen
    {
      return 0;
    }
    else
    { 
			tmpLinkId = p->id;
      p_temp = head_list;

      /* �ҵ�Ҫɾ���Ľڵ��ǰһ���ڵ� */
      while(p_temp->next != p)
      {
        p_temp = p_temp->next;
      }
      
      /* �Ͽ�Ҫɾ���Ľڵ� */
      p_temp->next = p->next;
      
      /* �ҵ�Ҫɾ���Ľڵ�ĺ�һ���ڵ� */
      p_temp = p_temp->next;
      
//      /* ֮��Ľڵ��LID����1 */
//      while(p_temp!= NULL)
//      {
//        p_temp->id--;
//        p_temp = p_temp->next;
//      }
      /* ����LID,�����д��ڸ�lid�ľ���һ�������е�ǰ��������*/
			p_temp = head_list;
			p_temp = p_temp->next;
			p_temp = p_temp->next;
			
      while(p_temp!= NULL)
      {
				if(p_temp->id > tmpLinkId)
				{
					p_temp->id--;
				}
        p_temp = p_temp->next;
      }      
      /* FLASH��������Ч��־ */
      list_ind_data =0x5a5a;
      
      /* �ڵ�������1 */
      flash_ed_num--;
    }
    
    
    /* �ͷŸýڵ㷢�ͻ����� */
    ip_addr = (p->ip[1] << 8) | p->ip[0];
    Free_tx_ed_buffer(ip_addr);
    
    /* �ͷ�Ҫɾ���ڵ�ռ�õĿռ� */
    free(p);
    p = NULL;
    
    /* ��������е�IP��ͻ��� */
    detect_ip_conflict();
    
    /* ���ڵ���Ϣ����д��FLASH */  
    head_list_to_flash(head_list); 
    
    return 1;
  }
}

/***************************************************************************
 * @fn          pdel_listNode
 *     
 * @brief       ɾ�������еĽڵ㡣��Ҫ����ɾ��һЩ��Ч������Ϣ,��Ҫ����ʹ�á�
 *              ��Ϊ����������������Ϣ��Ϊ������Ϣ��ֻ����һЩ���������ʹ�ã�
 *              ��ɾ����ϢʱMCU�����������˲�����Ϣ��
 *     
 * @data        2015��08��07��
 *     
 * @param       pNode - �����е���Ч��Ϣ��
 *     
 * @return      1 , �ɹ�
 *              0 , ʧ��
 ***************************************************************************
 */ 
uint8_t pdel_listNode(struct ip_mac_id *pNode)
{
  struct ip_mac_id  *p_temp  =  NULL;
  
	if(pNode == NULL)
		return 0;
	
	if(pNode== head_list) // it never happen
	{
		return 0;
	}
	else
	{ 
		p_temp = head_list;

		/* �ҵ�Ҫɾ���Ľڵ��ǰһ���ڵ� */
		while(p_temp->next != pNode)
		{
			p_temp = p_temp->next;
		}
		
		/* �Ͽ�Ҫɾ���Ľڵ� */
		p_temp->next = pNode->next;
			
	}
	
	/* �ͷ�Ҫɾ���ڵ�ռ�õĿռ� */
	free(pNode);
	pNode = NULL;
	return 1;
}
/***************************************************************************
 * @fn          print_all_ed_info
 *     
 * @brief       ��ӡ�ڵ���Ϣ
 *     
 * @data        2015��08��07��
 *     
 * @param       head - �ڵ�����ͷָ��
 *     
 * @return      void
 ***************************************************************************
 */ 
void print_all_ed_info(struct ip_mac_id *head)
{
  struct ip_mac_id *temp;
  temp=head;
  if(temp == NULL)
  {
#ifdef DEBUG
    put_string_uart1("there no endpoint !!!\n ");
#endif
  }
#ifdef DEBUG  
  put_string_uart1("the infomation of ed as following !!!\n ");
#endif
  while(temp!=NULL)
  {
#ifdef DEBUG
    put_multi_char_uart1(temp->ip, 4);
    put_multi_char_uart1(temp->mac, 4);
    put_char_uart1(temp->id);
    temp=temp->next;
    put_string_uart1("\n");
#endif
  }
} 

void SEGGER_RTT_print_all_ed_info(struct ip_mac_id *head)
{
#ifdef SEGGER_SYS_DEBUG
  struct ip_mac_id *temp;
	uint8_t i = 0;
	uint8_t printfTmp[9] = {0};
  temp=head;
  if(temp == NULL)
  {
		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"there no endpoint..."RTT_CTRL_RESET"\n");	
  }
	
  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"the infomation of ed as following !!!"RTT_CTRL_RESET"\n");	
	
  while(temp!=NULL)
  {
		for(i=0;i<4;i++)
		{
			printfTmp[i] = temp->ip[i];
			printfTmp[4+i] = temp->mac[i];
		}
		printfTmp[8] = temp->id;
		SEGGER_RTT_put_multi_char(printfTmp,9);
		temp=temp->next;
    SWSN_DELAY(2);		
  }
#endif
} 
/***************************************************************************
 * @fn          psearch_ip
 *     
 * @brief       ͨ��֡�и�����IP��ַ��Ϣ���������нڵ���Ϣ��
 *     
 * @data        2015��08��07��
 *     
 * @param       ip     - ֡��IP��ַ
 *              p_list - ������Ϣ
 *     
 * @return      1 , ���ҵ��ڵ���Ϣ
 *              0 , ���Ҳ����ڵ���Ϣ
 ***************************************************************************
 */ 
uint8_t psearch_ip(uint16_t *ip, struct ip_mac_id **p_list)
{

  struct ip_mac_id *p=head_list;
#ifdef DEBUG
  put_string_uart1("enter psearch_ip!!!\n ");
#endif
  while(p != NULL)
  {
    if( ip_comp(p,ip))
    {
      *p_list = p;
      return 1;
    }
    p= p->next;
  }
#ifdef DEBUG
       put_string_uart1("ip is not exsist!!!\n ");
#endif
  return 0;
}

/***************************************************************************
 * @fn          psearch_mac
 *     
 * @brief       ͨ��֡�и�����MAC��ַ��Ϣ���������нڵ���Ϣ��
 *     
 * @data        2015��08��07��
 *     
 * @param       mac    - ֡��mac��ַ
 *              p_list - ������Ϣ
 *     
 * @return      1 , ���ҵ��ڵ���Ϣ
 *              0 , ���Ҳ����ڵ���Ϣ
 ***************************************************************************
 */ 
uint8_t psearch_mac(uint8_t *mac, struct ip_mac_id **p_list)
{
  struct ip_mac_id *p=head_list;
  
  while(p != NULL)
  {
    if( mac_comp(p,mac))
    {
      *p_list = p;
      return 1;
    }
    p= p->next;
  }
      
  return 0;
}

//���ҽڵ�ͨ��id��
/***************************************************************************
 * @fn          psearch_id
 *     
 * @brief       ͨ��ID��Ϣ���������нڵ���Ϣ��
 *     
 * @data        2015��08��07��
 *     
 * @param       id     - ID��
 *              p_list - ������Ϣ
 *     
 * @return      1 , ���ҵ��ڵ���Ϣ
 *              0 , ���Ҳ����ڵ���Ϣ
 ***************************************************************************
 */ 

uint8_t psearch_id(uint8_t id, struct ip_mac_id **p_list)
{
  struct ip_mac_id *p=head_list;
  
  while(p != NULL)
  {
    if( p->id == id)
    {
      *p_list = p;
      return 1;
    }
    p= p->next;
  }
      
  return 0;
}

/**************************************************************************************************
 * @fn          Modify_IP_by_MAC
 *
 * @brief       ����MAC��ַ�޸Ķ�Ӧ��IP��ַ.
 *
 * @param       new_ip  - �µ�IP��ַ
 *              mac     - �ڵ��MAC��ַ
 *
 * @return      0 - �ṩ��MAC��ַ�����ڣ��޸�ʧ��
 *              1 - �޸ĳɹ�
 **************************************************************************************************
 */
uint8_t Modify_IP_by_MAC(const addr_t *mac, uint16_t new_ip)
{
  struct ip_mac_id *p = head_list;
  uint16_t old_ip;
  
  while(p != NULL)
  {
    /* �ҵ�ƥ���MAC��ַ */
    if(!memcmp(p->mac, mac, MAC_ADDR_SIZE))
    {
      break;
    }
    p= p->next;
  }
  
  if(p == NULL)
  {
    return 0;
  }
  
  /* ��ȡ�޸�ǰ��IP��ַ */
  old_ip = (p->ip[1] << 8) | p->ip[0];
  
  /* �޸������е�IP��ַ(Ŀǰֻʹ��2�ֽ�) */
  p->ip[0] = new_ip;
  p->ip[1] = new_ip >> 8;
  
  /* ��������е�IP��ͻ��� */
  detect_ip_conflict();
  
  /* �޸Ľڵ㷢�ͻ����������ڱ�ʶ��ͬ�ڵ��IP */  
  Modify_ip_in_tx_ed_buffer(old_ip, new_ip);
  

  /* ���޸ĺ������д��FLASH */
  head_list_to_flash(head_list);

  return 1;
}

/**************************************************************************************************
 * @fn          detect_ip_conflict
 *
 * @brief       ���ڵ���Ϣ�����д��ڵ�IP��ַ��ַ��ͻ,���ڳ�ͻ�Ľڵ���IP��ͻ��־λ.
 *
 * @param       none
 *
 * @return      ����IP��ͻ�Ľڵ����.
 **************************************************************************************************
 */
uint8_t detect_ip_conflict(void)
{
  uint8_t conflict_num;
  struct ip_mac_id *p, *q;

  conflict_num = 0;

  /* �Ƚ������еĽڵ��IP��ͻ״̬��־��� */
  p = head_list;
  while(p != NULL)
  {
    p->status &= ~0x01;
    p = p->next;
  }
  
  /* p���ڱ�������,��������IP��ͻ״̬��־ */
  p = head_list->next;
  while(p != NULL)
  {  
    q = p;
    
    /* q����p����Ľ�㣬����p��IP�Ƚ� */
    
    while(q->next != NULL)
    {
      /* ���IP��ͬ,��IP��ͻ��־λ */
      if (!memcmp(&q->next->ip[0], &p->ip[0], 2))
      {
        q->next->status |= 0x01;
        conflict_num ++;
      }
      q = q->next;
    }
    p = p->next;
  }

  return conflict_num;
}



/***************************************************************************
 * @fn          channr_num_to_flash
 *     
 * @brief       ���ŵ���Ϣд��flash
 *     
 * @data        2015��08��07��
 *     
 * @param       chan_num - �ŵ���
 *     
 * @return      void
 ***************************************************************************
 */ 
void channr_num_to_flash( uint8_t chan_num)
{
  bspIState_t intState;
#ifdef DEBUG
  put_string_uart1("channr num to flash \n ");
#endif  
  BSP_ENTER_CRITICAL_SECTION(intState);
  
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
  FLASH_ErasePage(CHANNR_PTR);
  FLASH_ProgramHalfWord(CHANNR_PTR ,(u16)chan_num);
  FLASH_Lock();
  BSP_EXIT_CRITICAL_SECTION(intState);
}

/***************************************************************************
 * @fn          channr_num_from_flash
 *     
 * @brief       ��flash�ж����ŵ���Ϣ
 *     
 * @data        2015��08��07��
 *     
 * @param       void
 *     
 * @return      �ŵ���
 ***************************************************************************
 */ 
uint8_t channr_num_from_flash(void)
{
  uint8_t chan_num;
#ifdef DEBUG
  put_string_uart1("channr num from flash \n ");
#endif  
  chan_num=*(u16 *)CHANNR_PTR;
  return chan_num;
}

/***************************************************************************
 * @fn          head_list_to_flash
 *     
 * @brief       ���ڵ��������Ϣд��flash
 *     
 * @data        2015��08��07��
 *     
 * @param       head_list - ����ָ��
 *     
 * @return      ת��Ϊuint16_t�����ĳ���
 ***************************************************************************
 */ 
int head_list_to_flash( struct ip_mac_id *head_list)
{
  struct ip_mac_id *head = head_list;
  uint16_t i = 0, len, count = 0;
  bspIState_t intState;
  uint16_t head_data[2048];
#ifdef DEBUG
    put_string_uart1("write head_list to flash!!!\n ");
#endif
  memset(head_data,0x0,sizeof(head_data));
  
  head_data[0] = list_ind_data; //������Ч��־
  head_data[1] = flash_ed_num;  //�ڵ�����
  head_data[3] = 0xffff;
  i=4;
  
  while(head != NULL)
  {
     head_data[i  ] = head->ip[0] | (head->ip[1] << 8);
     head_data[i+1] = head->ip[2] | (head->ip[3] << 8);
     head_data[i+2] = head->mac[0]| (head->mac[1] << 8);
     head_data[i+3] = head->mac[2]| (head->mac[3] << 8);
     head_data[i+4] = head->id | (head->status << 8);
     i+=5;
     head=head->next;
  }
  len=i;
  head_data[2] = len-3;
  
  BSP_ENTER_CRITICAL_SECTION(intState);
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif    
  FLASH_ErasePage(LIST_IND_PTR);

	//��LIST_IND_PTR��־λ��Ϣ���д��,ȷ����Ϣ��Ϣд����ϡ�  
	count = 1;
  while(count < len)
  {
#ifdef IWDG_START 
		IWDG_ReloadCounter();  
#endif    
    FLASH_ProgramHalfWord((LIST_IND_PTR +count*2),head_data[count]);
    count++;
  } 
	count = 0;
	FLASH_ProgramHalfWord(LIST_IND_PTR ,head_data[0]);
	
  FLASH_Lock(); 
  count = 0;
  BSP_EXIT_CRITICAL_SECTION(intState);
  return len;
}

/***************************************************************************
 * @fn          flash_to_head_list
 *     
 * @brief       ��flash�ж����ڵ���Ϣ����ת��Ϊ����
 *     
 * @data        2015��08��07��
 *     
 * @param       head_list - ����
 *     
 * @return      0
 ***************************************************************************
 */ 
int flash_to_head_list( struct ip_mac_id *head_list)
{
  int i = 0, len = 0;
  struct ip_mac_id *pnew;
  struct ip_mac_id *pnext;
  uint16_t head_data[2048];
#ifdef DEBUG
    put_string_uart1("read flash data to head_list!!!\n ");
#endif
  memset(head_data,0x0,sizeof(head_data));
  
  len = (*(u16 *)LEN_PTR)-1;
  
  for(i = 0; i <len; i++)
  {
    head_data[i]=*(u16 *)(HEAD_LIST_PTR + i*2);
  }
  pnew=head_list;
  pnew->ip[0]   =  head_data[0] &0xffff;
  pnew->ip[1]   = (head_data[0] &0xffff)>>8;
  pnew->ip[2]   =  head_data[1] &0xffff;
  pnew->ip[3]   = (head_data[1] &0xffff)>>8;
  pnew->mac[0]  =  head_data[2] &0xffff;
  pnew->mac[1]  = (head_data[2] &0xffff)>>8;
  pnew->mac[2]  =  head_data[3] &0xffff;
  pnew->mac[3]  = (head_data[3] &0xffff)>>8;
  pnew->id      =  head_data[4] &0xffff;
  pnew->status  = (head_data[4] &0xffff)>>8;
  
  for(i=5;i<len;i+=5)
  {
    pnext=(struct ip_mac_id *)malloc(sizeof(struct ip_mac_id));
    
    pnext->ip[0]  =  head_data[i+0] &0xffff;
    pnext->ip[1]  = (head_data[i+0] &0xffff)>>8;
    pnext->ip[2]  =  head_data[i+1] &0xffff;
    pnext->ip[3]  = (head_data[i+1] &0xffff)>>8;
    pnext->mac[0] =  head_data[i+2] &0xffff;
    pnext->mac[1] = (head_data[i+2] &0xffff)>>8;
    pnext->mac[2] =  head_data[i+3] &0xffff;
    pnext->mac[3] = (head_data[i+3] &0xffff)>>8;
    pnext->id     =  head_data[i+4] &0xffff;
    pnew->status  = (head_data[i+4] &0xffff)>>8;
    
    pnew->next = pnext;
    pnew=pnext;
  }
  pnew->next=NULL;
#ifdef SEGGER_DEBUG 
//  SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"USART2 SEND:"RTT_CTRL_RESET"\n");        
//  SEGGER_RTT_put_multi_char(pSendData,len);
#endif    
  return 0; 
}



/***************************************************************************
 * @fn          SWSN_deleteInnetED
 *     
 * @brief       ɾ�������ڵ����Ϣ
 *     
 * @data        2016��02��24��
 *     
 * @param       mac - �ڵ��MAC��ַ
 *              lid - �ڵ��ID��
 *     
 * @return      1��ɾ���ɹ���0��ɾ��ʧ��
 ***************************************************************************
 */ 
uint8_t SWSN_deleteInnetED(addr_t mac_addr, linkID_t lid)
{
//  if(pdel_mac(mac_addr.addr))//ɾ�������еĽڵ�ɹ�
//  {
//    net_ed_num--;//��Ч�ڵ���.������net_frm_proc.c
//    SWSN_DeleteEDByLid(&lid);
//    sysinfo_to_flash( ); //����������Ϣ
//    
//    /* �����Join���豸�б���ɾ����Ӧ���豸�������� */
//    if(-1 != nwk_joinList_del(&mac_addr))
//    {
//      nwk_joinList_save();
//    }
//    return 1;
//  }      
	if(-1 != nwk_joinList_del(&mac_addr))
	{
		/* �����Join���豸�б���ɾ����Ӧ���豸�������� */
		nwk_joinList_save();
		
		pdel_mac(mac_addr.addr);//ɾ�������еĽڵ�ɹ�
		net_ed_num--;//��Ч�ڵ���.������net_frm_proc.c
		SWSN_DeleteEDByLid(&lid);
		sysinfo_to_flash( ); //����������Ϣ
	
		return 1;
	}
	

	
  return 0;
}

/***************************************************************************
 * @fn          SWSN_checkDuplicateLinkEdIPList
 *     
 * @brief       ����ظ��������صĽڵ�·�ɱ��Ƿ�����쳣������쳣������޸�
 *     
 * @data        2016��02��24��
 *     
 * @param       mac - �ڵ��MAC��ַ
 *              lid - �ڵ��ID��
 *     
 * @return      1��ɾ���ɹ���0��ɾ��ʧ��
 ***************************************************************************
 */ 
void SWSN_checkDuplicateLinkEdIPList(void)
{
	uint8_t i = 0;
	linkID_t lid;
	addr_t *ed_addr = NULL;
	ip_addr_t edIPAddr = {0};
	struct ip_mac_id *pId = NULL;
	struct ip_mac_id *pMac = NULL;
	
	lid = nwk_getDuplicateLinkLid();//��ȡ�������������ӵ�lid
	ed_addr= nwk_getAddressByLinkId(lid); //ͨ��lid��ȡmac��ַ
	
  nwk_joinList_searchIP(ed_addr,&edIPAddr); //ͨ��MAC��ַ�ڼ����б��в�ѯ����ڵ��IP
 
  psearch_id(lid, &pId); //��ͨ�������е�LID��ѯ�����MAC��ַ��IP��ַ
	psearch_mac((uint8_t*)ed_addr, &pMac);
	
	//�����в����ڸýڵ���Ϣ
	if((pId == NULL) && (pMac == NULL))
	{
		add_net_list(lid);
		return;
	}
	//ͨ��lid���ҳ����Ľڵ���Ϣ��ͨ��MAC��ַ���ҳ�������Ϣһ��
	if(pId == pMac)
	{
		if(compare_listNode_insertNode(pId,ed_addr,edIPAddr ,lid))
		{
			return;
		}
		else
		{
			pId->id = lid;
			for(i=0;i<4;i++)
			{
				if(i<2)
				{
					pId->ip[i] = edIPAddr.edAddr[i];
				}
				else
				{
					pId->ip[i] = edIPAddr.apAddr[i-2];
				}
				pId->mac[i] = ed_addr->addr[i];
			}
			pId->id = lid; 
			pId->status = 0;
			
			head_list_to_flash(head_list);		
      return ;			
		}
	} 
	else ////ͨ��lid���ҳ����Ľڵ���Ϣ��ͨ��MAC��ַ���ҳ�������Ϣ��һ��
	{
		pdel_listNode(pId);//ɾ���б��еĽڵ�
		pdel_listNode(pMac);
		add_net_list(lid);
		return;
	}
}
/***************************************************************************
 * @fn          compare_listNode_insertNode
 *     
 * @brief       �ж���������Ľڵ���Ϣ�Ƿ��������ڵ����Ϣһ��
 *     
 * @data        2016��02��24��
 *     
 * @param       pNode  - ��������Ľڵ���Ϣ
 *              edAddr - �ڵ�MAC��ַ
 *              edIP   - �ڵ�IP��ַ
 *     
 * @return      1��һ�£�0�������
 ***************************************************************************
 */ 
uint8_t compare_listNode_insertNode(struct ip_mac_id *pNode,addr_t *edAddr,ip_addr_t edIP,linkID_t linkId)
{
	if((pNode->id == linkId) && \
		 !memcmp(edAddr->addr,pNode->mac,sizeof(addr_t)) && \
		 !memcmp(edIP.edAddr,&pNode->ip[0],2) &&\
	   !memcmp(edIP.apAddr,&pNode->ip[2],2))
	{
		return 1;
	}
	return 0;
}

