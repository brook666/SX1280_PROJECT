#ifndef FLASH_START_ADDR_H
#define FLASH_START_ADDR_H

//#define NVIC_VECTTAB_FLASH_OFFSET  0x2000
#define NVIC_VECTTAB_FLASH_OFFSET  0x0


#define REMOTER_PRESS_NUM_PTR 0x08018000

//集中器串口复位标识，如果该标志已置位，则需要重置WIFI
#define AP_WIFI_RESET_ADDR_PTR     0x08019000

//记录系统重启次数的flash地址
#define SYSTEM_RESET_TIMES         0x08019800


//2k空间存储 地址信息
//#define GATEWAY_ADDR_PTR           0x0801a000
//#define AP_ADDR_PTR                0x0801a006
//#define AP_ADDR_TYPE_PTR           0x0801a00a

#define CHANNR_PTR                 0x0801a800   
/*0x0801b000 -- 0x0801cffff,共8K，存储节点路由表信息*/
#define LIST_IND_PTR               0x0801b000
#define ED_NUM_PTR                 0x0801b002
#define LEN_PTR                    0x0801b004
#define HEAD_LIST_PTR              0x0801b008 



//0x0801d000-0x0801ffff 共12k空间存储节点信息
#define SYSINFO_PTR                0x0801d000 

 /* Join设备列表在FLASH中存放的起始地址 */
#define JOIN_LIST_START_ADDR       0x08020000UL 

/*固件更新相关*/
#define FIRMWARE_INFO_PTR          0x08021000  //程序版本号
#define SWSN_APP_FRESH_BASE        0X08029800   //应用程序更新固件起始地址
#define SWSN_APP_BASE              0x08002000   //应用程序起始地址
#define SWSN_APP_FRESH_FLAG		     0x08001C00		//保存固件更新指示信息  
#define ONE_BLOCK_SIZE             0x800        //一个block为2K


#endif

