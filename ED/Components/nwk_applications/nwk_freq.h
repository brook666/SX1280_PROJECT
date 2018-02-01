/**************************************************************************************************
  Filename:       nwk_freq.h
  Revised:        $Date: 2008-05-06 16:48:33 -0700 (Tue, 06 May 2008) $
  Revision:       $Revision: 17025 $
  Author:         $Author: lfriedman $

  Description:    This header file supports the SimpliciTI Freq network application.

**************************************************************************************************/

#ifndef NWK_FREQ_H
#define NWK_FREQ_H

/* application payload offsets */
/*    both */
#define FB_APP_INFO_OS     0
#define FB_TID_OS          1

/* Logical channel number for MOVE request. Frame brodcast so no TID
 * is used. Channel number can occupy the TID location. Same offset
 * used for channel change request. No reply to that frame. 
 */
#define F_CHAN_OS          1

/* MGMT frame application requests */
#define  FREQ_REQ_MOVE        0x01
#define  FREQ_REQ_PING        0x02
#define  FREQ_REQ_REQ_MOVE    0x03

/* change the following as protocol developed */
#define MAX_FREQ_APP_FRAME    2

/* set the out frame sizes */
#define  FREQ_REQ_MOVE_FRAME_SIZE   2
#define  FREQ_REQ_PING_FRAME_SIZE   2

/* prototypes */
void         nwk_freqInit(void);
fhStatus_t   nwk_processFreq(mrfiPacket_t *);
#if defined( FREQUENCY_AGILITY )
smplStatus_t nwk_setChannel(freqEntry_t *);
void         nwk_getChannel(freqEntry_t *);
uint8_t      nwk_scanForChannels(freqEntry_t *);
smplStatus_t nwk_freqControl(ioctlAction_t, void *);
#endif

#endif
