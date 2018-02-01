#include <string.h>
#include "firmware_info_proc.h"
#include "smpl_config.h"
#include "bsp_leds.h"
#include "wifi_module.h"
#include "flash_start_addr.h"
#include "usart2_dma_proc.h"
#include "bui_pkg_fun.h"
#include "gtway_frm_proc.h"
#include "smpl_config.h"

static gtway_addr_t sMyGtwayAddr;
extern BootLoaderState bootloader_state ;
extern volatile  uint8_t usart_cur_frm_recv ;

uint8_t Verison_69Frame[64] = {0};
uint16_t Verison_69Frame_len=0;

/***************************************************************************
 * @fn          initFirmwareInfo
 *     
 * @brief       初始化网关,集中器地址和固件版本信息
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void initFirmwareInfo(void)
{
  init_gtway_addr(&sMyGtwayAddr);
  init_ap_addr();
  initCodeVersionInfo();
	
 // initpermissoned();

}

/***************************************************************************
 * @fn          initCodeVersionInfo
 *     
 * @brief       初始化软件版本信息
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void  initCodeVersionInfo(void)
{
  uint32_t cur_version = *(uint32_t*)FIRMWARE_INFO_PTR;
  if((cur_version == 0) || (cur_version == 0xffffffff))
  {
    cur_version = THIS_FIRMWARE_INFO;
    WriteCodeVersionInfoToFlash(&cur_version);
  }
}

/***************************************************************************
 * @fn          initpermissoned
 *     
 * @brief       初始化允许加入的设备列表
 *     
 * @data        2017年04月26日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void  initpermissoned(void)
{
	uint8_t size=0,i;
  uint8_t permission_EDinfo[100]=PERMISSION_ADDRESS;
	uint8_t Apinfo[4]=THIS_DEVICE_ADDRESS;
	for(i=0;i<sizeof(permission_EDinfo);i++)
  {
	  if((permission_EDinfo[i]!=0)&&((i%4)==0))
    {
		  size++;
		}
	}
  if(sizeof(permission_EDinfo)!=0)
  {
    WritePerEDToFlash(permission_EDinfo,Apinfo,size);
  }
}


/***************************************************************************
 * @fn          get_gtawy_addr
 *     
 * @brief       获取网关地址和类型
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      指向网关地址结构体的指针
 ***************************************************************************
 */ 
gtway_addr_t * get_gtawy_addr(void)
{
   return (&sMyGtwayAddr);
}

/***************************************************************************
 * @fn          get_gtawy_code_version
 *     
 * @brief       获取网关的版本号
 *     
 * @data        2015年08月05日
 *     
 * @param       uint32_t
 *     
 * @return      返回网关版本号
 ***************************************************************************
 */ 
uint32_t get_gtawy_code_version(void)
{
	uint32_t cur_version = *(uint32_t*)FIRMWARE_INFO_PTR;
	return (cur_version);
}
/***************************************************************************
 * @fn          init_gtway_addr
 *     
 * @brief       初始化网关地址和类型。如果指定的flash空间有数据，则用flash
 *              数据初始化网关地址和类型；反之，则用程序中指定的网关地址和
 *              类型来初始化，并将网关地址和类型写入相应指定的flash空间。
 *     
 * @data        2015年08月05日
 *     
 * @param       pMyGtwayAddr - 网关地址和类型的结构体指针
 *     
 * @return      void
 ***************************************************************************
 */ 
void init_gtway_addr(gtway_addr_t *pMyGtwayAddr)
{
  uint16_t addr_data[6];
  bspIState_t intState;
  gtway_addr_t * gtway_tmp = (gtway_addr_t *)GATEWAY_ADDR_PTR;
  
  if(*(u16 *)GATEWAY_ADDR_PTR != 0xffff)
  {
    pMyGtwayAddr->type[0] = gtway_tmp->type[0];
    pMyGtwayAddr->type[1] = gtway_tmp->type[1];
    
    pMyGtwayAddr->addr[0] = gtway_tmp->addr[0];
    pMyGtwayAddr->addr[1] = gtway_tmp->addr[1];
    pMyGtwayAddr->addr[2] = gtway_tmp->addr[2];
    pMyGtwayAddr->addr[3] = gtway_tmp->addr[3];
  }
  else
  {
    uint8_t i=0,count=0;
    uint8_t gtway_type[2] = this_gtway_type;
    uint8_t gtway_addr[4] = this_gtway_address;
    
    memcpy(pMyGtwayAddr->addr,gtway_addr,4);
    memcpy(pMyGtwayAddr->type,gtway_type,2);
    
    addr_data[0] = gtway_type[0] |(gtway_type[1]<<8);
    addr_data[1] = gtway_addr[0] |(gtway_addr[1]<<8);
    addr_data[2] = gtway_addr[2] |(gtway_addr[3]<<8);
//读出集中器地址，集中器地址和网关地址放在一个block里面
    for(i=0;i<2;i++)
       addr_data[3+i]=*(u16 *)(AP_ADDR_PTR + i*2);
    
    BSP_ENTER_CRITICAL_SECTION(intState);
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(GATEWAY_ADDR_PTR);

    while(count < 5)
    {
      FLASH_ProgramHalfWord((GATEWAY_ADDR_PTR +count*2),addr_data[count]);
      count++;
    } 
    FLASH_Lock(); 
    count = 0;
    BSP_EXIT_CRITICAL_SECTION(intState);    
  }
}

/***************************************************************************
 * @fn          init_ap_addr
 *     
 * @brief       初始化集中器地址。如果指定的flash中存在有效地址，则用flash
 *              中的数据初始化集中器地址；反之，则 用程序指定的地址初始化
 *              集中器地址。
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */ 
void init_ap_addr(void)
{
  if(MyAddressInFlashIsValid())
  {
    LoadMyAddressFromFlash();
  }
  else
  {
  }
}

/***************************************************************************
 * @fn          MyAddressInFlashIsValid
 *     
 * @brief       判断FLASH中的本机地址是否有效
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      0    , 无效
 *              其它 , 有效
 ***************************************************************************
 */ 
uint8_t MyAddressInFlashIsValid(void)
{
  addr_t addr;
  /* 本机地址的第一字节不能为0x00或0xFF */
  if(*(uint16_t *)AP_ADDR_PTR == 0xFFFF)
  {
     return 0;
  }
  memset(&addr, 0x0, sizeof(addr));

  return memcmp((addr_t *)AP_ADDR_PTR, &addr, MAC_ADDR_SIZE);//FLASH中的本机地址若为0则无效
}

/***************************************************************************
 * @fn          LoadMyAddressFromFlash
 *     
 * @brief       加载FLASH中的本机地址
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      SMPL_SUCCESS    , 设置成功
 *              SMPL_BAD_PARAM  , 设置失败     
 ***************************************************************************
 */ 
uint8_t LoadMyAddressFromFlash(void)
{
  /* 此操作必须在SMPL_Init()前执行才会成功 */
  uint8_t i=0;
  addr_t ap_addr;
  type_t ap_type;
  for(i=0;i<4;i++)
     ap_addr.addr[i] = *(u8 *)(AP_ADDR_PTR + i);
  
  nwk_setMyAddress(&ap_addr);
  
  for(i=0;i<2;i++)
     ap_type.type[i] = *(u8 *)(AP_ADDR_TYPE_PTR + i);
  
  nwk_setMyType(&ap_type);
  
  return 1;
}

/***************************************************************************
 * @fn          WriteMyAddressToFlash
 *     
 * @brief       将网关类型、地址和集中器地址写入flash,DI1=0xff,DI0=1。数据
 *              为12字节，数据域排列：网关类型(AB)+网关地址(CDEF)+集中器MAC
 *              地址(GHIJKL)，数据域均是高位在前低位在后，如
 *              00 fb + 00 00 00 01 + fe 08 09 19 00 01。
 *              注意：此命令仅在新设备出厂时使用一次，之后不应再使用，且不应
 *              提供给客户使用。
 *     
 * @data        2015年08月05日
 *     
 * @param       gtway_ap_addr - 网关类型和地址
 *     
 * @return      void
 ***************************************************************************
 */ 
void  WriteMyAddressToFlash(uint8_t * gtway_ap_addr)
{
  uint8_t count = 0;
  uint16_t ap_addr[4];
  uint16_t gtway_addr[6];
  bspIState_t intState;
  
  gtway_addr[0] = gtway_ap_addr[1] |(gtway_ap_addr[0]<<8);
  gtway_addr[1] = gtway_ap_addr[5] |(gtway_ap_addr[4]<<8);
  gtway_addr[2] = gtway_ap_addr[3] |(gtway_ap_addr[2]<<8);

  ap_addr[0] = gtway_ap_addr[11] |(gtway_ap_addr[10]<<8);
  ap_addr[1] = gtway_ap_addr[9]  |(gtway_ap_addr[8]<<8);
  ap_addr[2] = gtway_ap_addr[7]  |(gtway_ap_addr[6]<<8);
  
  BSP_ENTER_CRITICAL_SECTION(intState);
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
  FLASH_ErasePage(GATEWAY_ADDR_PTR);
  
  while(count < 3)
  {
    FLASH_ProgramHalfWord((GATEWAY_ADDR_PTR +count*2),gtway_addr[count]);
    count++;
  } 
  count = 0;
  
  while(count < 3)
  {
    FLASH_ProgramHalfWord((AP_ADDR_PTR +count*2),ap_addr[count]);
    count++;
  } 
  FLASH_Lock();

  BSP_EXIT_CRITICAL_SECTION(intState);    
  
}
/***************************************************************************
 * @fn          WriteCodeVersionInfoToFlash
 *     
 * @brief       将软件版本信息写入flash,DI1=0x88,DI0=1
 *     
 * @data        2015年08月05日
 *     
 * @param       curVersion - 软件版本信息
 *     
 * @return      void
 ***************************************************************************
 */ 
void  WriteCodeVersionInfoToFlash(uint32_t *curVersion)
{
  bspIState_t intState;
  
  BSP_ENTER_CRITICAL_SECTION(intState);
  
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
  FLASH_ErasePage(FIRMWARE_INFO_PTR);
  
  FLASH_ProgramWord(FIRMWARE_INFO_PTR, *curVersion);

  FLASH_Lock();

  BSP_EXIT_CRITICAL_SECTION(intState);    
  
}


/***************************************************************************
 * @fn          WritePerEDToFlash
 *     
 * @brief       ????????flash
 *     
 * @data        2017?04?24?
 *     
 * @param       curVersion - ??????
 *     
 * @return      void
 ***************************************************************************
 */ 
void  WritePerEDToFlash(uint8_t * permission_EDinfo,uint8_t * ApAddr,uint8_t num)
{
  bspIState_t intState;
	uint32_t i=0;
	uint32_t w_addr=(uint32_t)JOIN_LIST_START_ADDR;
  BSP_ENTER_CRITICAL_SECTION(intState);
  
  FLASH_Unlock();

	FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
  FLASH_ErasePage(JOIN_LIST_START_ADDR);
  for(i=0;i<num;i++)
	{
    FLASH_ProgramWord(w_addr, *(uint32_t *)(permission_EDinfo));
	  FLASH_ProgramHalfWord(w_addr+4, *(uint16_t *)(permission_EDinfo+2));
	  FLASH_ProgramHalfWord(w_addr+6, *(uint16_t *)(permission_EDinfo));
	  FLASH_ProgramHalfWord(w_addr+8, *(uint16_t *)(ApAddr));
		w_addr=w_addr+10;
		permission_EDinfo=permission_EDinfo+4;
	}
  FLASH_Lock();

  BSP_EXIT_CRITICAL_SECTION(intState);   

}


/***************************************************************************
 * @fn          restoreFactorySettings
 *     
 * @brief       将软件版本信息写入flash,DI1=0x88,DI0=1
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      void
 ***************************************************************************
 */
void restoreFactorySettings(void)
{
  bspIState_t intState;
  BSP_ENTER_CRITICAL_SECTION(intState);
  
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR); 
#ifdef IWDG_START 
  IWDG_ReloadCounter();  
#endif													
  FLASH_ErasePage(LIST_IND_PTR);											
  FLASH_ErasePage(SYSINFO_PTR);											
  FLASH_ErasePage(SYSINFO_PTR+ONE_BLOCK_SIZE);  										
  FLASH_ErasePage(JOIN_LIST_START_ADDR);      //清除加入节点的信息；
  FLASH_Lock();
 
  BSP_EXIT_CRITICAL_SECTION(intState);
}

/***************************************************************************
 * @fn          readSystemResetTimes
 *     
 * @brief       读取设备重启的次数
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      返回设备重启的次数
 ***************************************************************************
 */
uint32_t readSystemResetTimes(void)
{
	return *(uint32_t *)(SYSTEM_RESET_TIMES);
}

/***************************************************************************
 * @fn          setSystemResetTimes
 *     
 * @brief       设置设备重启的次数
 *     
 * @data        2015年08月05日
 *     
 * @param       void
 *     
 * @return      返回设备重启的次数
 ***************************************************************************
 */
void setSystemResetTimes(void)
{
	uint32_t  system_reset_times = readSystemResetTimes(); 
  uint8_t   intState;  
  
	system_reset_times +=1;
  intState = OS_CPU_SR_Save();
  /* 解锁FLASH */
  FLASH_Unlock();
  
  /* 清除所有挂起标志位 */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  
  /* 擦除FLASH页 */
  FLASH_ErasePage(SYSTEM_RESET_TIMES);
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif  
    FLASH_ProgramHalfWord(SYSTEM_RESET_TIMES, system_reset_times);
  /* 锁定FLASH */
  FLASH_Lock();
  OS_CPU_SR_Restore(intState);
}

/***************************************************************************
 * @fn          Form_Verion_frame
 *     
 * @brief       构建版本帧
 *     
 * @data        2015年04月28日
 *     
 * @param       void
 *     
 * @return      
 ***************************************************************************
 */
void Form_Verion_frame(void)
{
	uint8_t Verion_88frame[18] = {0x88,0x01,0x00,0x00,0x00,0x00,0x00,
	0x08,0xff,0x20,0x06,0x01,0x01,0x10,0x00,0x00,0xFF,0x16};

	uint32_t Version_date = get_gtawy_code_version();

	Verion_88frame[15] = (uint8_t)Version_date;
	Verion_88frame[14] = (uint8_t)(Version_date>>8);
	Verion_88frame[13] = (uint8_t)(Version_date>>16);
//	Verion_88frame[14] = (uint8_t)(Version_date>>24);

	Verion_88frame[16] = gen_crc(Verion_88frame,16);
	
	
	#ifdef SEGGER_DEBUG 
		SEGGER_RTT_WriteString(0, RTT_CTRL_TEXT_BRIGHT_MAGENTA"Form the Frame 0f Verion Success"RTT_CTRL_RESET"\n");        
	#endif
	Verison_69Frame_len = Bui_send_to_cloudserver_msg(Verion_88frame,18,Verison_69Frame);
}


