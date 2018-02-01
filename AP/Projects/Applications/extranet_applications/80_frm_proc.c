#include "80_frm_proc.h"
#include "stdint.h"
#include "string.h"
#include "nwk.h"
#include "flash_start_addr.h"
#include "bui_pkg_fun.h"
#include "nwk_globals.h"

extern  persistentContext_t sPersistInfo;



/**************************************************************************************************
 * @fn          buid_80_frm
 *
 * @brief       建立一条发送给串口1的80帧
 *
 * @param       lid            节点的连接标识
 *              msg            输入的68帧内容
 *              inner_msg_len  输入的68帧长度
 *              out_msg        输出的80帧  
 *
 * @return      输出的80帧长度
 **************************************************************************************************
 */
uint8_t buid_80_frm(uint8_t lid,uint8_t * msg, uint8_t inner_msg_len,uint8_t * out_msg)
{
  int8_t i;
	uint8_t ed_mac_addr[4];
	uint8_t ap_addr[4];
	uint8_t data_len=msg[7]-4;
	const addr_t * ap_addr_pointer=nwk_getMyAddress();
	for(i=100;i>=0;i--)
  {
	  if(lid==sPersistInfo.connStruct[i].thisLinkID)                              //根据连接标识获取节点地址
    {
		  ed_mac_addr[0]=sPersistInfo.connStruct[i].peerAddr[3];
			ed_mac_addr[1]=sPersistInfo.connStruct[i].peerAddr[2];
			ed_mac_addr[2]=sPersistInfo.connStruct[i].peerAddr[1];
			ed_mac_addr[3]=sPersistInfo.connStruct[i].peerAddr[0];
			break;
		}	
	}
	if(i<0)
  {
	  return 0;
	}
  
	memcpy(ap_addr,(uint8_t *)ap_addr_pointer,4);
	
	out_msg[0]=YIXIANG_FRM_HEAD;
	out_msg[1]=ap_addr[3];
	out_msg[2]=ap_addr[2];
	out_msg[3]=ap_addr[1];
	out_msg[4]=ap_addr[0];
	out_msg[5]=ed_mac_addr[0];
	out_msg[6]=ed_mac_addr[1];
	out_msg[7]=ed_mac_addr[2];
	out_msg[8]=ed_mac_addr[3];
	out_msg[9]=data_len;
	memcpy(&out_msg[10],&msg[12],out_msg[9]);
	out_msg[10+data_len]=gen_crc(out_msg,10+data_len);
	out_msg[11+data_len]=YIXIANG_FRM_END;
	
	return (12+data_len);
	
}

