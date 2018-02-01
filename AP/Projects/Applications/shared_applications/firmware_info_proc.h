#ifndef   _GTWAY_AP_ADD_PROC_H
#define   _GTWAY_AP_ADD_PROC_H


#include "bsp.h"
#include "nwk_layer.h"
#include "gtway_config.h"


typedef struct
{
  uint8_t  type[2];
  uint8_t  addr[4];
} gtway_addr_t;

void initFirmwareInfo(void);
gtway_addr_t * get_gtawy_addr(void);
uint32_t get_gtawy_code_version(void);
void init_gtway_addr(gtway_addr_t *);
void init_ap_addr(void);
uint8_t MyAddressInFlashIsValid(void);
uint8_t LoadMyAddressFromFlash(void);
void  WriteMyAddressToFlash(uint8_t * );
void  initCodeVersionInfo(void);
void  WriteCodeVersionInfoToFlash(uint32_t * firmwareInfo);
void restoreFactorySettings(void);
uint32_t readSystemResetTimes(void);
void setSystemResetTimes(void);
void Form_Verion_frame(void);
void  WritePerEDToFlash(uint8_t * permission_EDinfo,uint8_t * ApAddr,uint8_t num);

void  initpermissoned(void);

#endif




