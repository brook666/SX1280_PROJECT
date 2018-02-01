#include <string.h>
#include "firmware_info_proc.h"
#include "smpl_config.h"
#include "bsp_leds.h"
#include "wifi_module.h"
#include "flash_start_addr.h"
#include "usart2_dma_proc.h"
#include "bui_pkg_fun.h"
#include "gtway_frm_proc.h"

static gtway_addr_t sMyGtwayAddr;
extern BootLoaderState bootloader_state ;
extern volatile  uint8_t usart_cur_frm_recv ;

uint8_t Verison_69Frame[64] = {0};
uint16_t Verison_69Frame_len=0;

/***************************************************************************
 * @fn          initFirmwareInfo
 *     
 * @brief       ��ʼ������,��������ַ�͹̼��汾��Ϣ
 *     
 * @data        2015��08��05��
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
}

/***************************************************************************
 * @fn          initCodeVersionInfo
 *     
 * @brief       ��ʼ������汾��Ϣ
 *     
 * @data        2015��08��05��
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
 * @fn          get_gtawy_addr
 *     
 * @brief       ��ȡ���ص�ַ������
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      ָ�����ص�ַ�ṹ���ָ��
 ***************************************************************************
 */ 
gtway_addr_t * get_gtawy_addr(void)
{
   return (&sMyGtwayAddr);
}

/***************************************************************************
 * @fn          get_gtawy_code_version
 *     
 * @brief       ��ȡ���صİ汾��
 *     
 * @data        2015��08��05��
 *     
 * @param       uint32_t
 *     
 * @return      �������ذ汾��
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
 * @brief       ��ʼ�����ص�ַ�����͡����ָ����flash�ռ������ݣ�����flash
 *              ���ݳ�ʼ�����ص�ַ�����ͣ���֮�����ó�����ָ�������ص�ַ��
 *              ��������ʼ�����������ص�ַ������д����Ӧָ����flash�ռ䡣
 *     
 * @data        2015��08��05��
 *     
 * @param       pMyGtwayAddr - ���ص�ַ�����͵Ľṹ��ָ��
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
//������������ַ����������ַ�����ص�ַ����һ��block����
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
 * @brief       ��ʼ����������ַ�����ָ����flash�д�����Ч��ַ������flash
 *              �е����ݳ�ʼ����������ַ����֮���� �ó���ָ���ĵ�ַ��ʼ��
 *              ��������ַ��
 *     
 * @data        2015��08��05��
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
 * @brief       �ж�FLASH�еı�����ַ�Ƿ���Ч
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      0    , ��Ч
 *              ���� , ��Ч
 ***************************************************************************
 */ 
uint8_t MyAddressInFlashIsValid(void)
{
  addr_t addr;
  /* ������ַ�ĵ�һ�ֽڲ���Ϊ0x00��0xFF */
  if(*(uint16_t *)AP_ADDR_PTR == 0xFFFF)
  {
     return 0;
  }
  memset(&addr, 0x0, sizeof(addr));

  return memcmp((addr_t *)AP_ADDR_PTR, &addr, MAC_ADDR_SIZE);//FLASH�еı�����ַ��Ϊ0����Ч
}

/***************************************************************************
 * @fn          LoadMyAddressFromFlash
 *     
 * @brief       ����FLASH�еı�����ַ
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      SMPL_SUCCESS    , ���óɹ�
 *              SMPL_BAD_PARAM  , ����ʧ��     
 ***************************************************************************
 */ 
uint8_t LoadMyAddressFromFlash(void)
{
  /* �˲���������SMPL_Init()ǰִ�вŻ�ɹ� */
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
 * @brief       ���������͡���ַ�ͼ�������ַд��flash,DI1=0xff,DI0=1������
 *              Ϊ12�ֽڣ����������У���������(AB)+���ص�ַ(CDEF)+������MAC
 *              ��ַ(GHIJKL)����������Ǹ�λ��ǰ��λ�ں���
 *              00 fb + 00 00 00 01 + fe 08 09 19 00 01��
 *              ע�⣺������������豸����ʱʹ��һ�Σ�֮��Ӧ��ʹ�ã��Ҳ�Ӧ
 *              �ṩ���ͻ�ʹ�á�
 *     
 * @data        2015��08��05��
 *     
 * @param       gtway_ap_addr - �������ͺ͵�ַ
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
 * @brief       ������汾��Ϣд��flash,DI1=0x88,DI0=1
 *     
 * @data        2015��08��05��
 *     
 * @param       curVersion - ����汾��Ϣ
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
 * @fn          restoreFactorySettings
 *     
 * @brief       ������汾��Ϣд��flash,DI1=0x88,DI0=1
 *     
 * @data        2015��08��05��
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
  FLASH_ErasePage(JOIN_LIST_START_ADDR);      //�������ڵ����Ϣ��
  FLASH_Lock();
 
  BSP_EXIT_CRITICAL_SECTION(intState);
}

/***************************************************************************
 * @fn          readSystemResetTimes
 *     
 * @brief       ��ȡ�豸�����Ĵ���
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      �����豸�����Ĵ���
 ***************************************************************************
 */
uint32_t readSystemResetTimes(void)
{
	return *(uint32_t *)(SYSTEM_RESET_TIMES);
}

/***************************************************************************
 * @fn          setSystemResetTimes
 *     
 * @brief       �����豸�����Ĵ���
 *     
 * @data        2015��08��05��
 *     
 * @param       void
 *     
 * @return      �����豸�����Ĵ���
 ***************************************************************************
 */
void setSystemResetTimes(void)
{
	uint32_t  system_reset_times = readSystemResetTimes(); 
  uint8_t   intState;  
  
	system_reset_times +=1;
  intState = OS_CPU_SR_Save();
  /* ����FLASH */
  FLASH_Unlock();
  
  /* ������й����־λ */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
  
  /* ����FLASHҳ */
  FLASH_ErasePage(SYSTEM_RESET_TIMES);
#ifdef IWDG_START 
     IWDG_ReloadCounter();  
#endif  
    FLASH_ProgramHalfWord(SYSTEM_RESET_TIMES, system_reset_times);
  /* ����FLASH */
  FLASH_Lock();
  OS_CPU_SR_Restore(intState);
}

/***************************************************************************
 * @fn          Form_Verion_frame
 *     
 * @brief       �����汾֡
 *     
 * @data        2015��04��28��
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


