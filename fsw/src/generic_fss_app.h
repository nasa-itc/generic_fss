/*******************************************************************************
** File: generic_fss_app.h
**
** Purpose:
**   This is the main header file for the GENERIC_FSS application.
**
*******************************************************************************/
#ifndef _GENERIC_FSS_APP_H_
#define _GENERIC_FSS_APP_H_

/*
** Include Files
*/
#include "cfe.h"
#include "generic_fss_device.h"
#include "generic_fss_events.h"
#include "generic_fss_platform_cfg.h"
#include "generic_fss_perfids.h"
#include "generic_fss_msg.h"
#include "generic_fss_msgids.h"
#include "generic_fss_version.h"
#include "hwlib.h"


/*
** Specified pipe depth - how many messages will be queued in the pipe
*/
#define GENERIC_FSS_PIPE_DEPTH            32


/*
** Enabled and Disabled Definitions
*/
#define GENERIC_FSS_DEVICE_DISABLED       0
#define GENERIC_FSS_DEVICE_ENABLED        1


/*
** GENERIC_FSS global data structure
** The cFE convention is to put all global app data in a single struct. 
** This struct is defined in the `generic_fss_app.h` file with one global instance 
** in the `.c` file.
*/
typedef struct
{
    /*
    ** Housekeeping telemetry packet
    ** Each app defines its own packet which contains its OWN telemetry
    */
    GENERIC_FSS_Hk_tlm_t   HkTelemetryPkt;   /* GENERIC_FSS Housekeeping Telemetry Packet */
    
    /*
    ** Operational data  - not reported in housekeeping
    */
    CFE_SB_MsgPtr_t MsgPtr;             /* Pointer to msg received on software bus */
    CFE_SB_PipeId_t CmdPipe;            /* Pipe Id for HK command pipe */
    uint32 RunStatus;                   /* App run status for controlling the application state */

    /*
	** Device data 
	*/
	uint32 DeviceID;		            /* Device ID provided by CFS on initialization */
    GENERIC_FSS_Device_tlm_t DevicePkt;      /* Device specific data packet */

    /* 
    ** Device protocol
    ** TODO: Make specific to your application
    */ 
    uart_info_t Generic_fssUart;             /* Hardware protocol definition */

} GENERIC_FSS_AppData_t;


/*
** Exported Data
** Extern the global struct in the header for the Unit Test Framework (UTF).
*/
extern GENERIC_FSS_AppData_t GENERIC_FSS_AppData; /* GENERIC_FSS App Data */


/*
**
** Local function prototypes.
**
** Note: Except for the entry point (GENERIC_FSS_AppMain), these
**       functions are not called from any other source module.
*/
void  GENERIC_FSS_AppMain(void);
int32 GENERIC_FSS_AppInit(void);
void  GENERIC_FSS_ProcessCommandPacket(void);
void  GENERIC_FSS_ProcessGroundCommand(void);
void  GENERIC_FSS_ProcessTelemetryRequest(void);
void  GENERIC_FSS_ReportHousekeeping(void);
void  GENERIC_FSS_ReportDeviceTelemetry(void);
void  GENERIC_FSS_ResetCounters(void);
void  GENERIC_FSS_Enable(void);
void  GENERIC_FSS_Disable(void);
int32 GENERIC_FSS_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 expected_length);

#endif /* _GENERIC_FSS_APP_H_ */
