/************************************************************************
** File:
**   $Id: generic_fss_msgids.h  $
**
** Purpose:
**  Define GENERIC_FSS Message IDs
**
*************************************************************************/
#ifndef _GENERIC_FSS_MSGIDS_H_
#define _GENERIC_FSS_MSGIDS_H_

/* 
** CCSDS V1 Command Message IDs (MID) must be 0x18xx
*/
#define GENERIC_FSS_CMD_MID              0x190A /* TODO: Change this for your app */ 

/* 
** This MID is for commands telling the app to publish its telemetry message
*/
#define GENERIC_FSS_REQ_HK_MID           0x190B /* TODO: Change this for your app */

/* 
** CCSDS V1 Telemetry Message IDs must be 0x08xx
*/
#define GENERIC_FSS_HK_TLM_MID           0x090A /* TODO: Change this for your app */
#define GENERIC_FSS_DEVICE_TLM_MID       0x090B /* TODO: Change this for your app */

#endif /* _GENERIC_FSS_MSGIDS_H_ */
