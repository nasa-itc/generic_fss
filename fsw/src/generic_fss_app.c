/*******************************************************************************
** File: generic_fss_app.c
**
** Purpose:
**   This file contains the source code for the GENERIC_FSS application.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_fss_app.h"


/*
** Global Data
*/
GENERIC_FSS_AppData_t GENERIC_FSS_AppData;

static CFE_EVS_BinFilter_t  GENERIC_FSS_EventFilters[] =
{   /* Event ID    mask */
    {GENERIC_FSS_RESERVED_EID,           0x0000},
    {GENERIC_FSS_STARTUP_INF_EID,        0x0000},
    {GENERIC_FSS_LEN_ERR_EID,            0x0000},
    {GENERIC_FSS_PIPE_ERR_EID,           0x0000},
    {GENERIC_FSS_SUB_CMD_ERR_EID,        0x0000},
    {GENERIC_FSS_SUB_REQ_HK_ERR_EID,     0x0000},
    {GENERIC_FSS_PROCESS_CMD_ERR_EID,    0x0000},
    {GENERIC_FSS_CMD_ERR_EID,            0x0000},
    {GENERIC_FSS_CMD_NOOP_INF_EID,       0x0000},
    {GENERIC_FSS_CMD_RESET_INF_EID,      0x0000},
    {GENERIC_FSS_CMD_ENABLE_INF_EID,     0x0000},
    {GENERIC_FSS_ENABLE_INF_EID,         0x0000},
    {GENERIC_FSS_ENABLE_ERR_EID,         0x0000},
    {GENERIC_FSS_CMD_DISABLE_INF_EID,    0x0000},
    {GENERIC_FSS_DISABLE_INF_EID,        0x0000},
    {GENERIC_FSS_DISABLE_ERR_EID,        0x0000},
    {GENERIC_FSS_CMD_CONFIG_INF_EID,     0x0000},
    {GENERIC_FSS_CONFIG_INF_EID,         0x0000},
    {GENERIC_FSS_CONFIG_ERR_EID,         0x0000},
    {GENERIC_FSS_DEVICE_TLM_ERR_EID,     0x0000},
    {GENERIC_FSS_REQ_HK_ERR_EID,         0x0000},
    {GENERIC_FSS_REQ_DATA_ERR_EID,       0x0000},
    {GENERIC_FSS_UART_INIT_ERR_EID,      0x0000},
    {GENERIC_FSS_UART_CLOSE_ERR_EID,     0x0000},
    {GENERIC_FSS_UART_READ_ERR_EID,      0x0000},
    {GENERIC_FSS_UART_WRITE_ERR_EID,     0x0000},
    {GENERIC_FSS_UART_TIMEOUT_ERR_EID,   0x0000},
    /* TODO: Add additional event IDs (EID) to the table as created */
};


/*
** Application entry point and main process loop
*/
void GENERIC_FSS_AppMain(void)
{
    int32 status = OS_SUCCESS;

    /*
    ** Register the application with executive services
    */
    CFE_ES_RegisterApp();

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(GENERIC_FSS_PERF_ID);

    /* 
    ** Perform application initialization
    */
    status = GENERIC_FSS_AppInit();
    if (status != CFE_SUCCESS)
    {
        GENERIC_FSS_AppData.RunStatus = CFE_ES_APP_ERROR;
    }

    /*
    ** Main loop
    */
    while (CFE_ES_RunLoop(&GENERIC_FSS_AppData.RunStatus) == TRUE)
    {
        /*
        ** Performance log exit stamp
        */
        CFE_ES_PerfLogExit(GENERIC_FSS_PERF_ID);

        /* 
        ** Pend on the arrival of the next Software Bus message
        ** Note that this is the standard, but timeouts are available
        */
        status = CFE_SB_RcvMsg(&GENERIC_FSS_AppData.MsgPtr, GENERIC_FSS_AppData.CmdPipe, CFE_SB_PEND_FOREVER);
        
        /* 
        ** Begin performance metrics on anything after this line. This will help to determine
        ** where we are spending most of the time during this app execution.
        */
        CFE_ES_PerfLogEntry(GENERIC_FSS_PERF_ID);

        /*
        ** If the CFE_SB_RcvMsg was successful, then continue to process the command packet
        ** If not, then exit the application in error.
        ** Note that a SB read error should not always result in an app quitting.
        */
        if (status == CFE_SUCCESS)
        {
            GENERIC_FSS_ProcessCommandPacket();
        }
        else
        {
            CFE_EVS_SendEvent(GENERIC_FSS_PIPE_ERR_EID, CFE_EVS_ERROR, "GENERIC_FSS: SB Pipe Read Error = %d", (int) status);
            GENERIC_FSS_AppData.RunStatus = CFE_ES_APP_ERROR;
        }
    }

    /*
    ** Disable component, which cleans up the interface, upon exit
    */
    GENERIC_FSS_Disable();

    /*
    ** Performance log exit stamp
    */
    CFE_ES_PerfLogExit(GENERIC_FSS_PERF_ID);

    /*
    ** Exit the application
    */
    CFE_ES_ExitApp(GENERIC_FSS_AppData.RunStatus);
} 


/* 
** Initialize application
*/
int32 GENERIC_FSS_AppInit(void)
{
    int32 status = OS_SUCCESS;
    
    GENERIC_FSS_AppData.RunStatus = CFE_ES_APP_RUN;

    /*
    ** Register the events
    */ 
    status = CFE_EVS_Register(GENERIC_FSS_EventFilters,
                              sizeof(GENERIC_FSS_EventFilters)/sizeof(CFE_EVS_BinFilter_t),
                              CFE_EVS_BINARY_FILTER);    /* as default, no filters are used */
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GENERIC_FSS: Error registering for event services: 0x%08X\n", (unsigned int) status);
       return status;
    }

    /*
    ** Create the Software Bus command pipe 
    */
    status = CFE_SB_CreatePipe(&GENERIC_FSS_AppData.CmdPipe, GENERIC_FSS_PIPE_DEPTH, "FSS_CMD_PIPE");
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(GENERIC_FSS_PIPE_ERR_EID, CFE_EVS_ERROR,
            "Error Creating SB Pipe,RC=0x%08X",(unsigned int) status);
       return status;
    }
    
    /*
    ** Subscribe to ground commands
    */
    status = CFE_SB_Subscribe(GENERIC_FSS_CMD_MID, GENERIC_FSS_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(GENERIC_FSS_SUB_CMD_ERR_EID, CFE_EVS_ERROR,
            "Error Subscribing to HK Gnd Cmds, MID=0x%04X, RC=0x%08X",
            GENERIC_FSS_CMD_MID, (unsigned int) status);
        return status;
    }

    /*
    ** Subscribe to housekeeping (hk) message requests
    */
    status = CFE_SB_Subscribe(GENERIC_FSS_REQ_HK_MID, GENERIC_FSS_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(GENERIC_FSS_SUB_REQ_HK_ERR_EID, CFE_EVS_ERROR,
            "Error Subscribing to HK Request, MID=0x%04X, RC=0x%08X",
            GENERIC_FSS_REQ_HK_MID, (unsigned int) status);
        return status;
    }

    /*
    ** TODO: Subscribe to any other messages here
    */


    /* 
    ** Initialize the published HK message - this HK message will contain the 
    ** telemetry that has been defined in the GENERIC_FSS_HkTelemetryPkt for this app.
    */
    CFE_SB_InitMsg(&GENERIC_FSS_AppData.HkTelemetryPkt,
                   GENERIC_FSS_HK_TLM_MID,
                   GENERIC_FSS_HK_TLM_LNGTH, TRUE);

    /*
    ** Initialize the device packet message
    ** This packet is specific to your application
    */
    CFE_SB_InitMsg(&GENERIC_FSS_AppData.DevicePkt,
                   GENERIC_FSS_DEVICE_TLM_MID,
                   GENERIC_FSS_DEVICE_TLM_LNGTH, TRUE);

    /*
    ** TODO: Initialize any other messages that this app will publish
    */


    /* 
    ** Always reset all counters during application initialization 
    */
    GENERIC_FSS_ResetCounters();

    /*
    ** Initialize application data
    ** Note that counters are excluded as they were reset in the previous code block
    */
    GENERIC_FSS_AppData.HkTelemetryPkt.DeviceEnabled = GENERIC_FSS_DEVICE_DISABLED;

    /* 
     ** Send an information event that the app has initialized. 
     ** This is useful for debugging the loading of individual applications.
     */
    status = CFE_EVS_SendEvent(GENERIC_FSS_STARTUP_INF_EID, CFE_EVS_INFORMATION,
               "GENERIC_FSS App Initialized. Version %d.%d.%d.%d",
                GENERIC_FSS_MAJOR_VERSION,
                GENERIC_FSS_MINOR_VERSION, 
                GENERIC_FSS_REVISION, 
                GENERIC_FSS_MISSION_REV);	
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("GENERIC_FSS: Error sending initialization event: 0x%08X\n", (unsigned int) status);
    }
    return status;
} 


/* 
** Process packets received on the GENERIC_FSS command pipe
*/
void GENERIC_FSS_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(GENERIC_FSS_AppData.MsgPtr);
    switch (MsgId)
    {
        /*
        ** Ground Commands with command codes fall under the GENERIC_FSS_CMD_MID (Message ID)
        */
        case GENERIC_FSS_CMD_MID:
            GENERIC_FSS_ProcessGroundCommand();
            break;

        /*
        ** All other messages, other than ground commands, add to this case statement.
        */
        case GENERIC_FSS_REQ_HK_MID:
            GENERIC_FSS_ProcessTelemetryRequest();
            break;

        /*
        ** All other invalid messages that this app doesn't recognize, 
        ** increment the command error counter and log as an error event.  
        */
        default:
            GENERIC_FSS_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(GENERIC_FSS_PROCESS_CMD_ERR_EID,CFE_EVS_ERROR, "GENERIC_FSS: Invalid command packet, MID = 0x%x", MsgId);
            break;
    }
    return;
} 


/*
** Process ground commands
** TODO: Add additional commands required by the specific component
*/
void GENERIC_FSS_ProcessGroundCommand(void)
{
    int32 status = OS_SUCCESS;

    /*
    ** MsgId is only needed if the command code is not recognized. See default case
    */
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(GENERIC_FSS_AppData.MsgPtr);   

    /*
    ** Ground Commands, by definition, have a command code (_CC) associated with them
    ** Pull this command code from the message and then process
    */
    uint16 CommandCode = CFE_SB_GetCmdCode(GENERIC_FSS_AppData.MsgPtr);
    switch (CommandCode)
    {
        /*
        ** NOOP Command
        */
        case GENERIC_FSS_NOOP_CC:
            /*
            ** First, verify the command length immediately after CC identification 
            ** Note that VerifyCmdLength handles the command and command error counters
            */
            if (GENERIC_FSS_VerifyCmdLength(GENERIC_FSS_AppData.MsgPtr, sizeof(GENERIC_FSS_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                /* Second, send EVS event on successful receipt ground commands*/
                CFE_EVS_SendEvent(GENERIC_FSS_CMD_NOOP_INF_EID, CFE_EVS_INFORMATION, "GENERIC_FSS: NOOP command received");
                /* Third, do the desired command action if applicable, in the case of NOOP it is no operation */
            }
            break;

        /*
        ** Reset Counters Command
        */
        case GENERIC_FSS_RESET_COUNTERS_CC:
            if (GENERIC_FSS_VerifyCmdLength(GENERIC_FSS_AppData.MsgPtr, sizeof(GENERIC_FSS_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(GENERIC_FSS_CMD_RESET_INF_EID, CFE_EVS_INFORMATION, "GENERIC_FSS: RESET counters command received");
                GENERIC_FSS_ResetCounters();
            }
            break;

        /*
        ** Enable Command
        */
        case GENERIC_FSS_ENABLE_CC:
            if (GENERIC_FSS_VerifyCmdLength(GENERIC_FSS_AppData.MsgPtr, sizeof(GENERIC_FSS_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(GENERIC_FSS_CMD_ENABLE_INF_EID, CFE_EVS_INFORMATION, "GENERIC_FSS: Enable command received");
                GENERIC_FSS_Enable();
            }
            break;

        /*
        ** Disable Command
        */
        case GENERIC_FSS_DISABLE_CC:
            if (GENERIC_FSS_VerifyCmdLength(GENERIC_FSS_AppData.MsgPtr, sizeof(GENERIC_FSS_NoArgs_cmd_t)) == OS_SUCCESS)
            {
                CFE_EVS_SendEvent(GENERIC_FSS_CMD_DISABLE_INF_EID, CFE_EVS_INFORMATION, "GENERIC_FSS: Disable command received");
                GENERIC_FSS_Disable();
            }
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            GENERIC_FSS_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(GENERIC_FSS_CMD_ERR_EID, CFE_EVS_ERROR, 
                "GENERIC_FSS: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", MsgId, CommandCode);
            break;
    }
    return;
} 


/*
** Process Telemetry Request - Triggered in response to a telemetery request
** TODO: Add additional telemetry required by the specific component
*/
void GENERIC_FSS_ProcessTelemetryRequest(void)
{
    int32 status = OS_SUCCESS;

    /* MsgId is only needed if the command code is not recognized. See default case */
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(GENERIC_FSS_AppData.MsgPtr);   

    /* Pull this command code from the message and then process */
    uint16 CommandCode = CFE_SB_GetCmdCode(GENERIC_FSS_AppData.MsgPtr);
    switch (CommandCode)
    {
        case GENERIC_FSS_REQ_HK_TLM:
            GENERIC_FSS_AppData.HkTelemetryPkt.CommandCount++;
            GENERIC_FSS_ReportHousekeeping();
            break;

        case GENERIC_FSS_REQ_DATA_TLM:
            GENERIC_FSS_AppData.HkTelemetryPkt.CommandCount++;
            GENERIC_FSS_ReportDeviceTelemetry();
            break;

        /*
        ** Invalid Command Codes
        */
        default:
            /* Increment the error counter upon receipt of an invalid command */
            GENERIC_FSS_AppData.HkTelemetryPkt.CommandErrorCount++;
            CFE_EVS_SendEvent(GENERIC_FSS_DEVICE_TLM_ERR_EID, CFE_EVS_ERROR, 
                "GENERIC_FSS: Invalid command code for packet, MID = 0x%x, cmdCode = 0x%x", MsgId, CommandCode);
            break;
    }
    return;
}


/* 
** Report Application Housekeeping
*/
void GENERIC_FSS_ReportHousekeeping(void)
{
    int32 status = OS_SUCCESS;

    /* Time stamp and publish housekeeping telemetry */
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &GENERIC_FSS_AppData.HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *) &GENERIC_FSS_AppData.HkTelemetryPkt);
    return;
}


/*
** Collect and Report Device Telemetry
*/
void GENERIC_FSS_ReportDeviceTelemetry(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is enabled */
    if (GENERIC_FSS_AppData.HkTelemetryPkt.DeviceEnabled == GENERIC_FSS_DEVICE_ENABLED)
    {
        status = GENERIC_FSS_RequestData(GENERIC_FSS_AppData.Generic_fssUart.handle, (GENERIC_FSS_Device_Data_tlm_t*) &GENERIC_FSS_AppData.DevicePkt.Generic_fss);
        if (status == OS_SUCCESS)
        {
            GENERIC_FSS_AppData.HkTelemetryPkt.DeviceCount++;
            /* Time stamp and publish data telemetry */
            CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &GENERIC_FSS_AppData.DevicePkt);
            CFE_SB_SendMsg((CFE_SB_Msg_t *) &GENERIC_FSS_AppData.DevicePkt);
        }
        else
        {
            GENERIC_FSS_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(GENERIC_FSS_REQ_DATA_ERR_EID, CFE_EVS_ERROR, 
                    "GENERIC_FSS: Request device data reported error %d", status);
        }
    }
    /* Intentionally do not report errors if disabled */
    return;
}


/*
** Reset all global counter variables
*/
void GENERIC_FSS_ResetCounters(void)
{
    GENERIC_FSS_AppData.HkTelemetryPkt.CommandErrorCount = 0;
    GENERIC_FSS_AppData.HkTelemetryPkt.CommandCount = 0;
    GENERIC_FSS_AppData.HkTelemetryPkt.DeviceErrorCount = 0;
    GENERIC_FSS_AppData.HkTelemetryPkt.DeviceCount = 0;
    return;
} 


/*
** Enable Component
** TODO: Edit for your specific component implementation
*/
void GENERIC_FSS_Enable(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is disabled */
    if (GENERIC_FSS_AppData.HkTelemetryPkt.DeviceEnabled == GENERIC_FSS_DEVICE_DISABLED)
    {
        /*
        ** Initialize hardware interface data
        ** TODO: Make specific to your application depending on protocol in use
        ** Note that other components provide examples for the different protocols available
        */ 
        GENERIC_FSS_AppData.Generic_fssUart.deviceString = GENERIC_FSS_CFG_STRING;
        GENERIC_FSS_AppData.Generic_fssUart.handle = GENERIC_FSS_CFG_HANDLE;
        GENERIC_FSS_AppData.Generic_fssUart.isOpen = PORT_CLOSED;
        GENERIC_FSS_AppData.Generic_fssUart.baud = GENERIC_FSS_CFG_BAUDRATE_HZ;
        GENERIC_FSS_AppData.Generic_fssUart.access_option = uart_access_flag_RDWR;

        /* Open device specific protocols */
        status = uart_init_port(&GENERIC_FSS_AppData.Generic_fssUart);
        if (status == OS_SUCCESS)
        {
            GENERIC_FSS_AppData.HkTelemetryPkt.DeviceCount++;
            GENERIC_FSS_AppData.HkTelemetryPkt.DeviceEnabled = GENERIC_FSS_DEVICE_ENABLED;
            CFE_EVS_SendEvent(GENERIC_FSS_ENABLE_INF_EID, CFE_EVS_INFORMATION, "GENERIC_FSS: Device enabled");
        }
        else
        {
            GENERIC_FSS_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(GENERIC_FSS_UART_INIT_ERR_EID, CFE_EVS_ERROR, "GENERIC_FSS: UART port initialization error %d", status);
        }
    }
    else
    {
        GENERIC_FSS_AppData.HkTelemetryPkt.DeviceErrorCount++;
        CFE_EVS_SendEvent(GENERIC_FSS_ENABLE_ERR_EID, CFE_EVS_ERROR, "GENERIC_FSS: Device enable failed, already enabled");
    }
    return;
}


/*
** Disable Component
** TODO: Edit for your specific component implementation
*/
void GENERIC_FSS_Disable(void)
{
    int32 status = OS_SUCCESS;

    /* Check that device is enabled */
    if (GENERIC_FSS_AppData.HkTelemetryPkt.DeviceEnabled == GENERIC_FSS_DEVICE_ENABLED)
    {
        /* Open device specific protocols */
        status = uart_close_port(GENERIC_FSS_AppData.Generic_fssUart.handle);
        if (status == OS_SUCCESS)
        {
            GENERIC_FSS_AppData.HkTelemetryPkt.DeviceCount++;
            GENERIC_FSS_AppData.HkTelemetryPkt.DeviceEnabled = GENERIC_FSS_DEVICE_DISABLED;
            CFE_EVS_SendEvent(GENERIC_FSS_DISABLE_INF_EID, CFE_EVS_INFORMATION, "GENERIC_FSS: Device disabled");
        }
        else
        {
            GENERIC_FSS_AppData.HkTelemetryPkt.DeviceErrorCount++;
            CFE_EVS_SendEvent(GENERIC_FSS_UART_CLOSE_ERR_EID, CFE_EVS_ERROR, "GENERIC_FSS: UART port close error %d", status);
        }
    }
    else
    {
        GENERIC_FSS_AppData.HkTelemetryPkt.DeviceErrorCount++;
        CFE_EVS_SendEvent(GENERIC_FSS_DISABLE_ERR_EID, CFE_EVS_ERROR, "GENERIC_FSS: Device disable failed, already disabled");
    }
    return;
}


/*
** Verify command packet length matches expected
*/
int32 GENERIC_FSS_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 expected_length)
{     
    int32 status = OS_SUCCESS;
    CFE_SB_MsgId_t msg_id = 0xFFFF;
    uint16 cmd_code = 0xFFFF;
    uint16 actual_length = CFE_SB_GetTotalMsgLength(msg);

    if (expected_length == actual_length)
    {
        /* Increment the command counter upon receipt of a valid command */
        GENERIC_FSS_AppData.HkTelemetryPkt.CommandCount++;
    }
    else
    {
        msg_id = CFE_SB_GetMsgId(msg);
        cmd_code = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(GENERIC_FSS_LEN_ERR_EID, CFE_EVS_ERROR,
           "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
              msg_id, cmd_code, actual_length, expected_length);

        status = OS_ERROR;

        /* Increment the command error counter upon receipt of an invalid command */
        GENERIC_FSS_AppData.HkTelemetryPkt.CommandErrorCount++;
    }
    return status;
} 
