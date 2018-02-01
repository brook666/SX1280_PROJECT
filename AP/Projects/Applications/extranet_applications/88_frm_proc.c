#include "88_frm_proc.h"
#include "bui_pkg_fun.h"
#include "firmware_info_proc.h" 
#include "stm32_rtc.h"
#include "wifi_module.h"
extern BootLoaderState bootloader_state ;
extern uint8_t reset_system_flag ;
extern uint8_t RefreshFirewareFlag ;
/***************************************************************************
 * @fn          proc_88_frm
 *     
 * @brief       88֡���ݴ���
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���
 *     
 * @return      
 ***************************************************************************
 */ 
void proc_88_frm(uint8_t *pInMsg,uint8_t *out_pkg,uint8_t *len)//88֡Ϊ�û��˺ͷ�����֮���ͨ��ָ����������ط���ͨ��
{
  uint8_t frame_type = (*(pInMsg+1)) & 0x03;
  
  if(frame_type == 0x00)
  {
    proc_88_data_frm(pInMsg,out_pkg,len);//���ڹ̼�����
  }
  else if(frame_type == 0x01)
  {
    proc_88_command_frm(pInMsg,out_pkg,len);//����֡
  }
  else if(frame_type == 0x02)
  {
    proc_88_ack_frm(pInMsg,out_pkg,len);//��ʱû��
  }
  else if(frame_type == 0x03)
  {
    proc_88_alarm_frm(pInMsg,out_pkg,len);//��ʱû��
  }
  else
  {
  }
}

/***************************************************************************
 * @fn          proc_88_data_frm
 *     
 * @brief       88֡����֡����
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���
 *     
 * @return      
 ***************************************************************************
 */ 
extern uint8_t RefreshFirewareFlag  ;
void proc_88_data_frm(uint8_t *pInMsg,uint8_t *out_pkg,uint8_t *len)
{
  if(RefreshFirewareFlag)
  {
    app_pkg_t in_pkg; 
    uint8_t alarm_info[4];
    static uint8_t  data_buffer[2048]={0}; //����һ��block������Ϣ
    static uint16_t data_cnt = 0;   //buf��������ݳ���
    static uint32_t total_data_len = 0;
    static uint8_t  total_block_size = 0;//�ܹ���Ҫ���ٸ�block���洢����
    static uint8_t  data_frm_cnt = 0;   //���յ���ʼ֡�����¼���
    uint8_t i=0,trans_type = 0;
    
    bui_app_pkg(pInMsg,&in_pkg);
    trans_type = in_pkg.ctr_code & 0x30;
    
    if(trans_type == 0x10)    //��ʼ֡
    {
      data_cnt  = 0;
      total_data_len = 0;
      data_frm_cnt = in_pkg.frm_cnt;
      for(i=0; i<in_pkg.data_len;i++)
      {
        data_buffer[data_cnt] = in_pkg.msg[i];
        data_cnt++;
        total_data_len++;
      }
      data_frm_cnt++;
    }
    else if(trans_type == 0x30)   //�м�֡
    {
      if(data_frm_cnt == in_pkg.frm_cnt)
      {
        for(i=0; i<in_pkg.data_len;i++)
        {
          data_buffer[data_cnt] = in_pkg.msg[i];
          data_cnt++;
          total_data_len++;        
          if(data_cnt == 2048)
          {
            data_cnt = 0;
            FLASH_WriteByte(SWSN_APP_FRESH_BASE+total_block_size*ONE_BLOCK_SIZE,data_buffer,2048);
            total_block_size++;
          }
          else
          {
          }               
        }
        data_frm_cnt ++;   
      } 
      else
      {
        alarm_info[0]=0x00;
        alarm_info[1]=0x02;
        alarm_info[2]=0x10;
        alarm_info[3]=data_frm_cnt;
        *len = bui_pkg_alarm(&in_pkg,alarm_info,4,out_pkg);      
      }  
    }
    else if(trans_type == 0x20)   //����֡  
    {
      if(data_frm_cnt == in_pkg.frm_cnt)
      {
        for(i=0; i<in_pkg.data_len;i++)
        {
          data_buffer[data_cnt] = in_pkg.msg[i];
          data_cnt++;
          total_data_len++;        
          if(data_cnt == 2048)
          {
            data_cnt = 0;
            FLASH_WriteByte(SWSN_APP_FRESH_BASE+total_block_size*ONE_BLOCK_SIZE,data_buffer,data_cnt);
            total_block_size++;
          }
          else
          {
          }                         
        }
        data_frm_cnt ++; 
        FLASH_WriteByte(SWSN_APP_FRESH_BASE+total_block_size*ONE_BLOCK_SIZE,data_buffer,data_cnt);
        total_block_size++;
        if(total_data_len>40960) //�ļ���С����40k����С���ļ����ܲ��Ǳ�ϵͳ����
        {
          bootloader_state.STATE = 0x0001;
          bootloader_state.LENGTH = total_data_len;
          Set_BootLoader_State();
        }
        else
        {
        }
      } 
      else
      {
        alarm_info[0]=0x00;
        alarm_info[1]=0x02;
        alarm_info[2]=0x10;
        alarm_info[3]=data_frm_cnt;
        *len = bui_pkg_alarm(&in_pkg,alarm_info,4,out_pkg);      
      }  
    }
    else
    {
    }
  } // end of RefreshFirewareFlag
    
}

/***************************************************************************
 * @fn          proc_88_command_frm
 *     
 * @brief       88֡����֡����
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���
 *     
 * @return      
 ***************************************************************************
 */ 
void proc_88_command_frm(uint8_t *pInMsg,uint8_t *out_pkg,uint8_t *len)
{
  uint8_t msg_id3;
  app_pkg_t in_pkg; 
  bui_app_pkg(pInMsg,&in_pkg);
  msg_id3=in_pkg.msg[0];
  if(msg_id3==0xff)   //�������û��Ҫ��
  {
    com_gtway_88_proc(&in_pkg,out_pkg,len);//

  }
  else if(msg_id3==0xfd) //���ܿ�������
  {
    sw_gtway_88_proc(&in_pkg,out_pkg,len);//return APP_SUCCESS;
  }
}

/***************************************************************************
 * @fn          proc_88_ack_frm
 *     
 * @brief       88֡Ӧ��֡����
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���
 *     
 * @return      
 ***************************************************************************
 */ 
void proc_88_ack_frm(uint8_t *pInMsg,uint8_t *out_pkg,uint8_t *len)
{
  
}

/***************************************************************************
 * @fn          proc_88_alarm_frm
 *     
 * @brief       88֡�澯֡����
 *     
 * @data        2015��08��05��
 *     
 * @param       pInMsg  - ��������
 *              out_pkg - �������
 *              len     - ���ݰ����ȡ�������ڴ�Ϊ�������ݵĳ��ȣ�
 *                        ���ڴ�Ϊ������ݵĳ���
 *     
 * @return      
 ***************************************************************************
 */ 
void proc_88_alarm_frm(uint8_t *pInMsg,uint8_t *out_pkg,uint8_t *len)
{
//  app_pkg_t in_pkg; 
//  uint8_t alarm_info[10];
//  bui_app_pkg(msg,&in_pkg);
//  
//  alarm_info[0]=0x00;
//  alarm_info[1]=0x04;
//  alarm_info[2]=0x03;
//  alarm_info[3]=0x09;
//  *len =bui_pkg_alarm(&in_pkg, alarm_info, 4, out_pkg);     
}

/***************************************************************************
 * @fn          com_gtway_88_proc
 *     
 * @brief       ͨ������֡����msg_id3=0xff��
 *     
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���
 *     
 * @return      APP_SUCCESS - �ɹ�
 *              APP_ERR     - ʧ��
 ***************************************************************************
 */ 
app_status_t com_gtway_88_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  app_device_disg_t para_rc=APP_DEVICE_DISG_SUCCESS;
  uint8_t msg_id2 = in_pkg->msg[1];//�������CI2λ

  
  if(msg_id2 == 0x20)
  {
    para_rc = server_frm_proc(in_pkg,out_pkg,len);
  }
  else
  {
  }
  
  if(para_rc !=APP_DEVICE_DISG_SUCCESS)
  {
    return APP_ERR;
  }
  else
  {
    return APP_SUCCESS;
  }
}

/***************************************************************************
 * @fn          sw_gtway_88_proc
 *     
 * @brief       ���ܲ�������֡����msg_id3=0xfd��
 *     
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_SUCCESS - �ɹ�
 ***************************************************************************
 */ 
app_status_t sw_gtway_88_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{
  return APP_SUCCESS;
}

/***************************************************************************
 * @fn          server_frm_proc
 *     
 * @brief       ������֡����msg_id3=0xff��msg_id2 = 0x20��
 *     
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���   
 *     
 * @return      APP_DEVICE_DISG_SUCCESS - �ɹ�
 *              APP_DEVICE_DISG_FAIL    - ʧ��
 ***************************************************************************
 */ 
app_device_disg_t server_frm_proc(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  app_server_para_t server_rc=APP_SERVER_PARA_SUCCESS;
  uint8_t msg_id1 = in_pkg->msg[2];//88֡��CI1
  if(msg_id1==0x00)//����֡
  {
    server_rc=check_heart_beat(in_pkg,out_pkg,len);
  }
  else if(msg_id1==0x01)//���������豸��ַ���豸����
  { 
    server_rc=get_gtway_type_mac(in_pkg,out_pkg,len);
  }
  else if(msg_id1==0x02)//��ѯ����·������SSID
  {
//    server_rc=get_router_ssid(in_pkg,out_pkg,len);
  }
  else if(msg_id1 == 0x03)
  {
    server_rc= sync_gtway_time(in_pkg,out_pkg,len);
  }
	else if(msg_id1 == 0x04)
  {
    server_rc= server_get_gtway_code_version(in_pkg,out_pkg,len);
  }
  else if(msg_id1 == 0x07)
  {
    server_rc= server_begin_firmware_update(in_pkg,out_pkg,len); 
  }
  else if(msg_id1 == 0x08)
  {
    server_rc= server_end_firmware_update(in_pkg,out_pkg,len);
  }
	else if(msg_id1 == 0x10)
	{
		server_rc= check_wifi_heart_frm(in_pkg,out_pkg,len);
	}
  else
  {
  }   
  if(server_rc==APP_SERVER_PARA_SUCCESS)
  {
    return APP_DEVICE_DISG_SUCCESS;
  }
  else
  {
    return APP_DEVICE_DISG_FAIL;
  }
}

/***************************************************************************
 * @fn          check_heart_beat
 *     
 * @brief       ����֡��ά�ַ����������ص�����
 *     
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SERVER_PARA_SUCCESS - �ɹ�
 ***************************************************************************
 */ 
app_server_para_t check_heart_beat(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  app_server_para_t rc = APP_SERVER_PARA_SUCCESS;
  *len = bui_rep_88_command(in_pkg,NULL,0,out_pkg);
  return rc;
}

/***************************************************************************
 * @fn          get_gtway_type_mac
 *     
 * @brief       88֡��ȡ���ص�ַ����������
 *     
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���         
 *     
 * @return      APP_SERVER_PARA_SUCCESS - �ɹ�
 ***************************************************************************
 */ 
app_server_para_t get_gtway_type_mac(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  uint8_t i;
  uint8_t gtway_type_addr[6];
  app_server_para_t rc = APP_SERVER_PARA_SUCCESS;
  gtway_addr_t * pMyGtwayAddr = get_gtawy_addr();
  
  for(i=0;i<2;i++)
    gtway_type_addr[i] = pMyGtwayAddr->type[1-i];
  for(i=0;i<4;i++)
    gtway_type_addr[i+2] = pMyGtwayAddr->addr[3-i];
	
  *len = bui_rep_88_command(in_pkg,gtway_type_addr,6,out_pkg);
   return rc;
}

//app_server_para_t get_router_ssid(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
//{
//  uint8_t i;
//  uint8_t gtway_ssid[20];
//  app_server_para_t rc = APP_SERVER_PARA_SUCCESS;
//  for(i=0;i<ssid_data_len;i++)
//    gtway_ssid[i] = ssid_data[i+1];
//  
//  *len = bui_rep_88_command(in_pkg,gtway_ssid,ssid_data_len,out_pkg);
//   return rc;
//  
//}

/***************************************************************************
 * @fn          sync_gtway_time
 *     
 * @brief       ���غͷ���������ʱ��ͬ��
 *     
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SERVER_PARA_SUCCESS -�ɹ�
 ***************************************************************************
 */ 
app_server_para_t sync_gtway_time(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{

  app_server_para_t rc = APP_SERVER_PARA_SUCCESS;
  uint8_t hasTime = 0;   //��1������ʾ����ʱ�䣻��0������ʾ����ʱ��
  hasTime = in_pkg->msg[4];
  if(hasTime == 0x01)
  {
    Time_Set(&in_pkg->msg[5],7);
  }
  else
  {
  }
  
  *len = bui_rep_88_command(in_pkg,&in_pkg->msg[4],in_pkg->data_len-4,out_pkg);
  
   return rc;
  
}

/***************************************************************************
 * @fn          server_get_gtway_code_version
 *     
 * @brief       ��������ȡ���ذ汾��Ϣ��ע��69֡������Ϊ0x11�����صĿ�����Ϊ0x12
 *     
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SERVER_PARA_SUCCESS -�ɹ�
 ***************************************************************************
 */ 
app_server_para_t server_get_gtway_code_version(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{

  app_server_para_t rc = APP_SERVER_PARA_SUCCESS;
	uint8_t version_temp[3] = {0};
	
	uint32_t Version_date = get_gtawy_code_version();
	
	version_temp[0] = (uint8_t)(Version_date>>16);
	version_temp[1] = (uint8_t)(Version_date>>8);
	version_temp[2] = (uint8_t)Version_date;
	
	
  *len = bui_rep_88_command(in_pkg,version_temp,3,out_pkg);
  
   return rc;
  
}

/***************************************************************************
 * @fn          server_begin_firmware_update
 *     
 * @brief       ��ʼ�̼���Ϣ,DI1=0x80,DI0=1������Ϊ4���ֽڣ���ʾ�汾�ţ���WXYZ��
 *              W:�汾�ţ�01��WiFi��02����������X����.Y����.Z���ա�
 *    
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SERVER_PARA_SUCCESS -�ɹ�
 ***************************************************************************
 */ 
app_server_para_t server_begin_firmware_update(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{

  app_server_para_t rc = APP_SERVER_PARA_SUCCESS;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  
  if(msg_id_0 == 0x00)
  {
    uint8_t i =0 ;
    uint32_t cur_version = *(uint32_t*) FIRMWARE_INFO_PTR;
    uint32_t fresh_version =  (in_pkg->msg[4]<<24) | (in_pkg->msg[5]<<16) |\
                              (in_pkg->msg[6]<<8) | in_pkg->msg[7];
    uint8_t cur_version_tmp[4] = {0};
    
    if(fresh_version>cur_version)
    {
      RefreshFirewareFlag = 1;  //׼����ʼ�̼�����
    }
    for(i=0; i<4; i++)
    {
      cur_version_tmp[i] = (cur_version>>((3-i)*8)); 
    }  

    *len = bui_rep_88_command(in_pkg,cur_version_tmp,4,out_pkg);//��������֡
        
    rc = APP_SERVER_PARA_SUCCESS;
  } 
  
   return rc;
  
}


/***************************************************************************
 * @fn          server_end_firmware_update
 *     
 * @brief       �����̼���Ϣ,DI1=0x88,DI0=1������Ϊ4���ֽڣ���ʾ�汾�ţ���WXYZ��
 *              W:�汾�ţ�01��WiFi��02����������X����.Y����.Z���ա��յ�����֡��
 *              ��������
 *     
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SERVER_PARA_SUCCESS -�ɹ�
 ***************************************************************************
 */ 
app_server_para_t server_end_firmware_update(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{

  app_server_para_t rc = APP_SERVER_PARA_SUCCESS;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  
  if(msg_id_0 == 0x01)
  {
    uint8_t i =0 ;
    uint32_t cur_version = *(uint32_t*) FIRMWARE_INFO_PTR;
    uint32_t fresh_version =  (in_pkg->msg[4]<<24) | (in_pkg->msg[5]<<16) |\
                              (in_pkg->msg[6]<<8) | in_pkg->msg[7];
    uint8_t cur_version_tmp[4] = {0};
    
    if(fresh_version>cur_version)
    {
       cur_version = fresh_version;
       RefreshFirewareFlag = 0;//�̼����½���
    }
    for(i=0; i<4; i++)
    {
      cur_version_tmp[i] = (cur_version>>((3-i)*8)); 
    }  

    *len = bui_rep_88_command(in_pkg,cur_version_tmp,4,out_pkg);//��������֡
    
    WriteCodeVersionInfoToFlash(&cur_version);//����FLASH�еı�����ַ
    
    
    reset_system_flag = 1;//����׼������
        
    rc = APP_SERVER_PARA_SUCCESS;
  } 
  return rc;
  
}

/***************************************************************************
 * @fn          check_wifi_heart_frm
 *     
 * @brief       ���WIFI����֡���ж��������ͺ����ص�ַ�Ƿ���ȷ��
 *     
 * @data        2015��08��06��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SERVER_PARA_SUCCESS -�ɹ�
 ***************************************************************************
 */ 
app_server_para_t check_wifi_heart_frm(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{

  app_server_para_t rc = APP_SERVER_PARA_SUCCESS;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  
  if(msg_id_0 == 0x01)
  {
		gtway_addr_t gtway_tmp ;
		gtway_addr_t *myGtway = get_gtawy_addr();
		
		gtway_tmp.type[1] = in_pkg->msg[4];
		gtway_tmp.type[0] = in_pkg->msg[5];
		gtway_tmp.addr[3] = in_pkg->msg[6];
		gtway_tmp.addr[2] = in_pkg->msg[7];
		gtway_tmp.addr[1] = in_pkg->msg[8];
		gtway_tmp.addr[0] = in_pkg->msg[9];
		
		if(memcmp(&gtway_tmp,myGtway,sizeof(gtway_addr_t)))
		{
			BSP_RESET_BOARD();
		}
		rc = APP_SERVER_PARA_SUCCESS;
  } 
  return rc;
  
}




