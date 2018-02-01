#ifndef FLASH_START_ADDR_H
#define FLASH_START_ADDR_H

//#define NVIC_VECTTAB_FLASH_OFFSET  0x2000
#define NVIC_VECTTAB_FLASH_OFFSET  0x0


#define REMOTER_PRESS_NUM_PTR 0x08018000

//���������ڸ�λ��ʶ������ñ�־����λ������Ҫ����WIFI
#define AP_WIFI_RESET_ADDR_PTR     0x08019000

//��¼ϵͳ����������flash��ַ
#define SYSTEM_RESET_TIMES         0x08019800


//2k�ռ�洢 ��ַ��Ϣ
//#define GATEWAY_ADDR_PTR           0x0801a000
//#define AP_ADDR_PTR                0x0801a006
//#define AP_ADDR_TYPE_PTR           0x0801a00a

#define CHANNR_PTR                 0x0801a800   
/*0x0801b000 -- 0x0801cffff,��8K���洢�ڵ�·�ɱ���Ϣ*/
#define LIST_IND_PTR               0x0801b000
#define ED_NUM_PTR                 0x0801b002
#define LEN_PTR                    0x0801b004
#define HEAD_LIST_PTR              0x0801b008 



//0x0801d000-0x0801ffff ��12k�ռ�洢�ڵ���Ϣ
#define SYSINFO_PTR                0x0801d000 

 /* Join�豸�б���FLASH�д�ŵ���ʼ��ַ */
#define JOIN_LIST_START_ADDR       0x08020000UL 

/*�̼��������*/
#define FIRMWARE_INFO_PTR          0x08021000  //����汾��
#define SWSN_APP_FRESH_BASE        0X08029800   //Ӧ�ó�����¹̼���ʼ��ַ
#define SWSN_APP_BASE              0x08002000   //Ӧ�ó�����ʼ��ַ
#define SWSN_APP_FRESH_FLAG		     0x08001C00		//����̼�����ָʾ��Ϣ  
#define ONE_BLOCK_SIZE             0x800        //һ��blockΪ2K


#endif

