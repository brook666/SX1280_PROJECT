#ifndef SWSN_PROC_H
#define SWSN_PROC_H

#include "net_para_fun.h"
#include "net_frm_proc.h"
#include "net_status_fun.h"
#include "bsp_mac_addr.h"
#include "nwk_token.h"
#include "nwk_types.h"
#include "nwk_globals.h"
#include "nwk_api.h"
#include "nwk_frame.h"
#include "nwk.h"
#include "bsp_leds.h"
#include "bsp.h"
#include "bsp_config.h"
#include <string.h>
#include "mrfi_si4438_api.h"
#include "mrfi_board_defs.h"


uint8_t initAppFunc(void);
uint8_t registerAppFuncDomain(uint8_t ,pAppFunc );
void swsn_info_init(void);
void swsn_app_proc(void);


#endif


