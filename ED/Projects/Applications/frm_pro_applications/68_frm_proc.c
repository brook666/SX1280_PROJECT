#include <string.h>
#include "bsp.h"
#include "bui_pkg_fun.h"
#include "net_para_fun.h   "
#include "net_status_fun.h "
#include "68_frm_proc.h"     
#include "gtway_frm_proc.h"
#include "net_frm_proc.h"
#include "swsn_proc.h"

static void proc_68_data_frm(const ed_pkg_t *pIn_ed_pkg, uint8_t *pOut_msg, uint8_t *pOut_len);
static void proc_68_command_frm(const ed_pkg_t *pIn_ed_pkg, uint8_t *pOut_msg, uint8_t *pOut_len);

static app_net_para_t end_firmware_update(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len);
static app_net_para_t begin_firmware_update(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len);

app_net_para_t device_type_proc(app_pkg_t *,uint8_t *, uint8_t *);
app_status_t usart_command_proc(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len);

extern BootLoaderState bootloader_state ;

uint8_t reset_system_flag = 0;
uint8_t RefreshFirewareFlag = 0 ;
/**************************************************************************************************
 * @fn          proc_68_frm
 *
 * @brief       �ԴӴ������������ͨ��֡(0x69֡)�е�SWSN����֡(0x68֡)����֡���ͽ��з���.
 *
 * @param       pIn_gtway_pkg - ���������ͨ��֡(0x69֡)�ṹ��
 *              pOut_msg      - �����Ӧ��֡�ֽ�����
 *              pOut_len      - ������ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_frm(const ed_pkg_t *pIn_ed_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
  uint8_t frame_type = GET_FRAME_TYPE(pIn_ed_pkg->msg);
  
  if(frame_type == 0x00)//����֡
  {
    proc_68_data_frm(pIn_ed_pkg, pOut_msg, pOut_len);
  }
  else if(frame_type == 0x01)//����֡
  {
    proc_68_command_frm(pIn_ed_pkg, pOut_msg, pOut_len);
  }
  return;
}

/**************************************************************************************************
 * @fn          proc_68_data_frm
 *
 * @brief       ���������������ͨ��֡(0x69֡)�е�SWSN����֡(0x68֡)�еġ�����֡��.
 *
 * @param       pIn_gtway_pkg - ���������ͨ��֡(0x69֡)�ṹ��
 *              pOut_msg      - �����Ӧ��֡�ֽ�����
 *              pOut_len      - ������ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_data_frm(const ed_pkg_t *pIn_ed_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
//  uint8_t   comm_type = GET_COMM_TYPE(pIn_ed_pkg->msg);
  app_pkg_t in_pkg;
  bui_app_pkg((uint8_t *)pIn_ed_pkg->msg, &in_pkg);
	
	if(RefreshFirewareFlag)
  {
    uint8_t alarm_info[4];
    static uint8_t  data_buffer[2048]={0}; //����һ��block������Ϣ
    static uint16_t data_cnt = 0;   //buf��������ݳ���
    static uint32_t total_data_len = 0;
    static uint8_t  total_block_size = 0;//�ܹ���Ҫ���ٸ�block���洢����
    static uint8_t  data_frm_cnt = 0;   //���յ���ʼ֡�����¼���
    uint8_t i=0,trans_type = 0;
    
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
        *pOut_len = bui_pkg_alarm(&in_pkg,alarm_info,4,pOut_msg);      
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
        *pOut_len = bui_pkg_alarm(&in_pkg,alarm_info,4,pOut_msg);      
      }  
    }
    else
    {
    }
  } // end of RefreshFirewareFlag
    
	
	
}


/**************************************************************************************************
 * @fn          proc_68_command_frm
 *
 * @brief       ���������������ͨ��֡(0x69֡)�е�SWSN����֡(0x68֡)�еġ�����֡��.
 *
 * @param       pIn_gtway_pkg - ���������ͨ��֡(0x69֡)�ṹ��
 *              pOut_msg      - �����Ӧ��֡�ֽ�����
 *              pOut_len      - ������ֽ����鳤��
 *
 * @return      none
 **************************************************************************************************
 */
void proc_68_command_frm(const ed_pkg_t *pIn_ed_pkg, uint8_t *pOut_msg, uint8_t *pOut_len)
{
	app_pkg_t in_pkg;
  uint8_t comm_type;
  uint8_t   command_id_fun  = GET_COMMANA_ID_DOMAIN(pIn_ed_pkg->msg);
	comm_type       = GET_COMM_TYPE(pIn_ed_pkg->msg)
//  uint16_t  ed_ip           = GET_ED_IP(pIn_ed_pkg->msg);


  if (comm_type == 0x08) //���ʼ�����
  {
    bui_app_pkg((uint8_t *)pIn_ed_pkg->msg, &in_pkg);

    if (command_id_fun == 0x01) //�����������֡
    {
			  usart_command_proc(&in_pkg, pOut_msg, pOut_len);
//      net_para_proc(&in_pkg, pOut_msg, pOut_len);
    }
    else
    {
      /* code */
    }
  }
  else
  {
  }
}


/***************************************************************************
 * @fn          usart_command_proc
 *     
 * @brief       �������Դ��ڵ�����֡
 *     
 * @data        2015��08��07��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���     
 *     
 * @return      APP_SUCCESS - �ɹ�
 *              APP_ERR     - ʧ��
 ***************************************************************************
 */ 
app_status_t usart_command_proc(app_pkg_t *in_pkg,uint8_t *out_pkg,uint8_t *len)
{
  
  app_net_para_t para_rc= APP_NET_PARA_SUCCESS;
  uint8_t msg_id_1 = in_pkg->msg[2];
  uint8_t alarm_info[4];
  switch(msg_id_1)
  {

		case 0x30: para_rc = device_type_proc (in_pkg, out_pkg, len);
               break;
    case 0x80: para_rc = begin_firmware_update (in_pkg, out_pkg, len);
               break;
    case 0x88: para_rc = end_firmware_update (in_pkg, out_pkg, len);
               break;
    default:   para_rc = NO_PAPA_FUN_FAIL;
  }
  
  
  if(para_rc ==APP_NET_PARA_SUCCESS)
  {  
    return APP_SUCCESS;
  }
  else
  {
    alarm_info[0] = 0x00;
    alarm_info[1] = 0x02;     
    alarm_info[2] = 0x02;
    alarm_info[3] = 0x08;
    *len =bui_pkg_alarm(in_pkg, alarm_info, 4, out_pkg);    
    return APP_ERR;
  }
}

/***************************************************************************
 * @fn          begin_firmware_update
 *     
 * @brief       ��ʼ�̼���Ϣ,DI1=0x80,DI0=1������Ϊ4���ֽڣ���ʾ�汾�ţ���WXYZ��
 *              W:�汾�ţ�01��WiFi��02����������X����.Y����.Z���ա�
 *     
 * @data        2015��09��28��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS             - �ɹ�
 *              APP_BEGIN_FIRMWARE_UPDATE_FAIL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t begin_firmware_update(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_BEGIN_FIRMWARE_UPDATE_FAIL;
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
      RefreshFirewareFlag = 1;  //׼����ʼ�̼�����
    }
    for(i=0; i<4; i++)
    {
      cur_version_tmp[i] = (cur_version>>((3-i)*8)); 
    }  

    *len = bui_rep_pkg_command(in_pkg,cur_version_tmp,4,out_pkg);//��������֡
        
    rc = APP_NET_PARA_SUCCESS;
  } 
  return rc;
  
}

/***************************************************************************
 * @fn          end_firmware_update
 *     
 * @brief       �����̼���Ϣ,DI1=0x88,DI0=1������Ϊ4���ֽڣ���ʾ�汾�ţ���WXYZ��
 *              W:�汾�ţ�01��WiFi��02����������X����.Y����.Z���ա��յ�����֡��
 *              ��������
 *     
 * @data        2015��09��28��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS             - �ɹ�
 *              APP_END_FIRMWARE_UPDATE_FAIL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t end_firmware_update(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_END_FIRMWARE_UPDATE_FAIL;
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

    *len = bui_rep_pkg_command(in_pkg,cur_version_tmp,4,out_pkg);//��������֡
    
    WriteCodeVersionInfoToFlash(&cur_version);//����FLASH�еı�����ַ
    
    reset_system_flag = 1;//����׼������
        
    rc = APP_NET_PARA_SUCCESS;
  } 
  return rc;
  
}



/***************************************************************************
 * @fn          device_type_proc
 *     
 * @brief       �豸���ͻ�ȡ��00Ϊ���أ�01Ϊ�ڵ�
 *     
 * @data        2017��05��08��
 *     
 * @param       in_pkg  - �Ѵ���õĽṹ������
 *              out_pkg - ��������
 *              len     - ������ݳ���       
 *     
 * @return      APP_NET_PARA_SUCCESS    - �ɹ�
 *              APP_CHANSPC_READ_FALL   - ʧ��
 ***************************************************************************
 */ 
app_net_para_t device_type_proc(app_pkg_t *in_pkg,uint8_t *out_pkg, uint8_t *len)
{

  app_net_para_t  rc= APP_TYPE_PROC_FALL;
  uint8_t   msg_id_0 = in_pkg->msg[3];
  uint8_t   device_type[1]={0xFF};
  
  if(msg_id_0==0)
  {
#ifdef ACCESS_POINT
		device_type[0]=0x00;
#endif	
#ifdef END_DEVICE
		device_type[0]=0x01;
#endif		
		*len = bui_rep_pkg_command(in_pkg,device_type,1,out_pkg);
    rc= APP_NET_PARA_SUCCESS;
  }
  else if(msg_id_0 == 0x01)
  {		
    return rc; 		
  } 
  return rc;
}

