/*****************************************************************************
// Copyright:   2014-2015, Hangzhou Thingcom Information Technology. Co., Ltd.
// File name:   net_list_proc.c
// Description: User APP.
//              节点信息链表处理.
// Author:      Leidi
// Version:     1.0
// Date:        2014-10-25
// History:     2014-10-25  Leidi 添加函数Modify_IP_by_MAC().
//              2014-10-28  Leidi 修改函数add_net_list(),在其中检查新添加的节点IP是否冲突,并设置status字段.
//                                修改函数init_list(),head_list_to_flash(),flash_to_head_list(),status字段从FLASH中存取.
//                                删除无用函数pdel_ip(),modify_ip().
//                                使用函数Modify_IP_by_MAC()修改IP时,清除status字段中的IP冲突标志位.
//                                使用函数pdel_mac()删除节点时,调用函数Free_tx_ed_buffer()释放相应的节点发送缓冲区.
//              2014-10-29  Leidi 添加函数detect_ip_conflict().其调用者有add_net_list(),pdel_mac(),Modify_IP_by_MAC().
//              2014-11-03  Leidi 在函数init_list()中调用detect_ip_conflict().以解决每次开机后IP冲突标志位不正确的问题.
//              2014-11-24  Leidi 在初始化链表函数init_list()中,将写入信道号的函数channr_num_to_flash()使用FREQUENCY_AGILITY
//                                条件编译.使之在不开启FREQUENCY_AGILITY时不会写入信道号.
//              2015-09-01  Zengjia 修改add_net_list(),其中IP地址的有许可join的列表分配。 
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

//初始化链表
/***************************************************************************
 * @fn          init_list
 *     
 * @brief       初始化链表。falsh中没有数据时，将程序中的初始化链表 信息写入
 *              flash。flash中有数据时，从flash中读出数据，并用该数据初始化
 *              链表,链表初始化完毕之后，对节点IP地址进行冲突检测。
 *     
 * @data        2015年08月07日
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
    
    /* 从FLASH中读取连接信息 */
    sysinfo_from_flash();
    
    /* 从FLASH中读取节点信息链表 */
    flash_to_head_list(head_list);
    
    /* 检测链表中的IP冲突情况 */
    detect_ip_conflict();
    
    /* 找到链表的尾指针 */
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
 * @brief       比较IP地址
 *     
 * @data        2015年08月07日
 *     
 * @param       node - 链表中节点信息
 *              ip   - 帧中节点IP
 *     
 * @return      1 , 匹配成功
 *              0 , 匹配失败
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
 * @fn          mac_comp(现在不用)
 *     
 * @brief       比较MAC地址
 *     
 * @data        2015年08月07日
 *     
 * @param       node - 链表中节点信息
 *              mac  - 帧中节点MAC
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
 * @brief       在链表尾部添加新增节点信息，并写入flash。
 *     
 * @data        2015年08月07日
 *     
 * @param       linkid - 节点ID号
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
  
	//查找链表中是否存在该节点的信息，如果存在节点信息，在修改节点信息
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
	}//在链表中没有找到该节点的信息，在添加节点
	else
	{
		
		flash_ed_num++;
		
		nwk_joinList_searchIP(ed_info,&edIPAddr);
		
		pnew=head_list;
		
		/* 找到链表的尾部 */
		while(pnew->next != NULL)
		{
			pnew=pnew->next;
		}
		
		/* 构造新的链表节点 */
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
		
		/* 将新的链表节点添加到链表尾部 */
		pnew->next=pnext;
		pnew->next->next=NULL;
		tail_list = pnext;
	}
  
  /* 检测链表中的IP冲突情况 */
  detect_ip_conflict();
  
  head_list_to_flash(head_list);
  sysinfo_to_flash();
  
  return 1;
}

/***************************************************************************
 * @fn          pdel_mac
 *     
 * @brief       通过MAC地址删除节点
 *     
 * @data        2015年08月07日
 *     
 * @param       mac - 节点MAC地址
 *     
 * @return      1 , 成功
 *              0 , 失败
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

      /* 找到要删除的节点的前一个节点 */
      while(p_temp->next != p)
      {
        p_temp = p_temp->next;
      }
      
      /* 断开要删除的节点 */
      p_temp->next = p->next;
      
      /* 找到要删除的节点的后一个节点 */
      p_temp = p_temp->next;
      
//      /* 之后的节点的LID均减1 */
//      while(p_temp!= NULL)
//      {
//        p_temp->id--;
//        p_temp = p_temp->next;
//      }
      /* 调整LID,链表中大于该lid的均减一，链表中的前两个除外*/
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
      /* FLASH中数据有效标志 */
      list_ind_data =0x5a5a;
      
      /* 节点数减少1 */
      flash_ed_num--;
    }
    
    
    /* 释放该节点发送缓冲区 */
    ip_addr = (p->ip[1] << 8) | p->ip[0];
    Free_tx_ed_buffer(ip_addr);
    
    /* 释放要删除节点占用的空间 */
    free(p);
    p = NULL;
    
    /* 检测链表中的IP冲突情况 */
    detect_ip_conflict();
    
    /* 将节点信息链表写入FLASH */  
    head_list_to_flash(head_list); 
    
    return 1;
  }
}

/***************************************************************************
 * @fn          pdel_listNode
 *     
 * @brief       删除链表中的节点。主要用来删除一些无效链表信息,不要轻易使用。
 *              因为大多数情况下链表信息均为正常信息，只有在一些特殊情况才使用，
 *              如删除信息时MCU重启，残留了部分信息。
 *     
 * @data        2015年08月07日
 *     
 * @param       pNode - 链表中的无效信息。
 *     
 * @return      1 , 成功
 *              0 , 失败
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

		/* 找到要删除的节点的前一个节点 */
		while(p_temp->next != pNode)
		{
			p_temp = p_temp->next;
		}
		
		/* 断开要删除的节点 */
		p_temp->next = pNode->next;
			
	}
	
	/* 释放要删除节点占用的空间 */
	free(pNode);
	pNode = NULL;
	return 1;
}
/***************************************************************************
 * @fn          print_all_ed_info
 *     
 * @brief       打印节点信息
 *     
 * @data        2015年08月07日
 *     
 * @param       head - 节点链表头指针
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
 * @brief       通过帧中给出的IP地址信息查找链表中节点信息。
 *     
 * @data        2015年08月07日
 *     
 * @param       ip     - 帧中IP地址
 *              p_list - 链表信息
 *     
 * @return      1 , 查找到节点信息
 *              0 , 查找不到节点信息
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
 * @brief       通过帧中给出的MAC地址信息查找链表中节点信息。
 *     
 * @data        2015年08月07日
 *     
 * @param       mac    - 帧中mac地址
 *              p_list - 链表信息
 *     
 * @return      1 , 查找到节点信息
 *              0 , 查找不到节点信息
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

//查找节点通过id号
/***************************************************************************
 * @fn          psearch_id
 *     
 * @brief       通过ID信息查找链表中节点信息。
 *     
 * @data        2015年08月07日
 *     
 * @param       id     - ID号
 *              p_list - 链表信息
 *     
 * @return      1 , 查找到节点信息
 *              0 , 查找不到节点信息
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
 * @brief       根据MAC地址修改对应的IP地址.
 *
 * @param       new_ip  - 新的IP地址
 *              mac     - 节点的MAC地址
 *
 * @return      0 - 提供的MAC地址不存在，修改失败
 *              1 - 修改成功
 **************************************************************************************************
 */
uint8_t Modify_IP_by_MAC(const addr_t *mac, uint16_t new_ip)
{
  struct ip_mac_id *p = head_list;
  uint16_t old_ip;
  
  while(p != NULL)
  {
    /* 找到匹配的MAC地址 */
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
  
  /* 获取修改前的IP地址 */
  old_ip = (p->ip[1] << 8) | p->ip[0];
  
  /* 修改链表中的IP地址(目前只使用2字节) */
  p->ip[0] = new_ip;
  p->ip[1] = new_ip >> 8;
  
  /* 检测链表中的IP冲突情况 */
  detect_ip_conflict();
  
  /* 修改节点发送缓冲区中用于标识不同节点的IP */  
  Modify_ip_in_tx_ed_buffer(old_ip, new_ip);
  

  /* 将修改后的链表写入FLASH */
  head_list_to_flash(head_list);

  return 1;
}

/**************************************************************************************************
 * @fn          detect_ip_conflict
 *
 * @brief       检测节点信息链表中存在的IP地址地址冲突,对于冲突的节点置IP冲突标志位.
 *
 * @param       none
 *
 * @return      存在IP冲突的节点个数.
 **************************************************************************************************
 */
uint8_t detect_ip_conflict(void)
{
  uint8_t conflict_num;
  struct ip_mac_id *p, *q;

  conflict_num = 0;

  /* 先将链表中的节点的IP冲突状态标志清除 */
  p = head_list;
  while(p != NULL)
  {
    p->status &= ~0x01;
    p = p->next;
  }
  
  /* p用于遍历链表,重新设置IP冲突状态标志 */
  p = head_list->next;
  while(p != NULL)
  {  
    q = p;
    
    /* q遍历p后面的结点，并与p的IP比较 */
    
    while(q->next != NULL)
    {
      /* 如果IP相同,置IP冲突标志位 */
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
 * @brief       将信道信息写入flash
 *     
 * @data        2015年08月07日
 *     
 * @param       chan_num - 信道号
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
 * @brief       从flash中读出信道信息
 *     
 * @data        2015年08月07日
 *     
 * @param       void
 *     
 * @return      信道号
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
 * @brief       将节点的链表信息写入flash
 *     
 * @data        2015年08月07日
 *     
 * @param       head_list - 链表指针
 *     
 * @return      转换为uint16_t数组后的长度
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
  
  head_data[0] = list_ind_data; //数据有效标志
  head_data[1] = flash_ed_num;  //节点数量
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

	//将LIST_IND_PTR标志位信息最后写入,确保信息信息写入完毕。  
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
 * @brief       从flash中读出节点信息，并转化为链表
 *     
 * @data        2015年08月07日
 *     
 * @param       head_list - 链表
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
 * @brief       删除在网节点的信息
 *     
 * @data        2016年02月24日
 *     
 * @param       mac - 节点的MAC地址
 *              lid - 节点的ID号
 *     
 * @return      1，删除成功；0，删除失败
 ***************************************************************************
 */ 
uint8_t SWSN_deleteInnetED(addr_t mac_addr, linkID_t lid)
{
//  if(pdel_mac(mac_addr.addr))//删除链表中的节点成功
//  {
//    net_ed_num--;//有效节点数.定义于net_frm_proc.c
//    SWSN_DeleteEDByLid(&lid);
//    sysinfo_to_flash( ); //保存连接信息
//    
//    /* 在许可Join的设备列表中删除对应的设备，并保存 */
//    if(-1 != nwk_joinList_del(&mac_addr))
//    {
//      nwk_joinList_save();
//    }
//    return 1;
//  }      
	if(-1 != nwk_joinList_del(&mac_addr))
	{
		/* 在许可Join的设备列表中删除对应的设备，并保存 */
		nwk_joinList_save();
		
		pdel_mac(mac_addr.addr);//删除链表中的节点成功
		net_ed_num--;//有效节点数.定义于net_frm_proc.c
		SWSN_DeleteEDByLid(&lid);
		sysinfo_to_flash( ); //保存连接信息
	
		return 1;
	}
	

	
  return 0;
}

/***************************************************************************
 * @fn          SWSN_checkDuplicateLinkEdIPList
 *     
 * @brief       检查重复加入网关的节点路由表是否出现异常，如果异常则进行修改
 *     
 * @data        2016年02月24日
 *     
 * @param       mac - 节点的MAC地址
 *              lid - 节点的ID号
 *     
 * @return      1，删除成功；0，删除失败
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
	
	lid = nwk_getDuplicateLinkLid();//获取重新与网关连接的lid
	ed_addr= nwk_getAddressByLinkId(lid); //通过lid获取mac地址
	
  nwk_joinList_searchIP(ed_addr,&edIPAddr); //通过MAC地址在加入列表中查询加入节点的IP
 
  psearch_id(lid, &pId); //通过链表中的LID查询链表的MAC地址和IP地址
	psearch_mac((uint8_t*)ed_addr, &pMac);
	
	//链表中不存在该节点信息
	if((pId == NULL) && (pMac == NULL))
	{
		add_net_list(lid);
		return;
	}
	//通过lid查找出来的节点信息与通过MAC地址查找出来的信息一致
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
	else ////通过lid查找出来的节点信息与通过MAC地址查找出来的信息不一致
	{
		pdel_listNode(pId);//删除列表中的节点
		pdel_listNode(pMac);
		add_net_list(lid);
		return;
	}
}
/***************************************************************************
 * @fn          compare_listNode_insertNode
 *     
 * @brief       判断链表里面的节点信息是否与待插入节点的信息一致
 *     
 * @data        2016年02月24日
 *     
 * @param       pNode  - 链表里面的节点信息
 *              edAddr - 节点MAC地址
 *              edIP   - 节点IP地址
 *     
 * @return      1，一致；0，不相等
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

