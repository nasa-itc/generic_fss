/*******************************************************************************
** File: generic_fss_device.h
**
** Purpose:
**   This is the header file for the GENERIC_FSS device.
**
*******************************************************************************/
#ifndef _GENERIC_FSS_DEVICE_H_
#define _GENERIC_FSS_DEVICE_H_

/*
** Required header files.
*/
#include "device_cfg.h"
#include "hwlib.h"
#include "generic_fss_platform_cfg.h"


/*
** Type definitions
** TODO: Make specific to your application
*/
#define GENERIC_FSS_DEVICE_HDR_0            0xDE
#define GENERIC_FSS_DEVICE_HDR_1            0xAD
#define GENERIC_FSS_DEVICE_HDR_2            0xBE
#define GENERIC_FSS_DEVICE_HDR_3            0xEF

#define GENERIC_FSS_DEVICE_NOOP_CMD         0x00
#define GENERIC_FSS_DEVICE_REQ_DATA_CMD     0x01

#define GENERIC_FSS_DEVICE_HDR_TRL_LEN      4
#define GENERIC_FSS_DEVICE_CMD_SIZE         7
#define GENERIC_FSS_DEVICE_DATA_SIZE        16

/*
** GENERIC_FSS device data telemetry definition
*/
typedef struct
{
    float   Alpha;
    float   Beta;
    uint8_t ErrorCode; // 0 = no error (valid alpha, beta), 1 = error (invalid alpha, beta)
} OS_PACK GENERIC_FSS_Device_Data_tlm_t;

/*
** Prototypes
*/
int32_t GENERIC_FSS_ReadData(spi_info_t *device, uint8_t* write_data, uint8_t* read_data, uint8_t data_length);
int32_t GENERIC_FSS_CommandDevice(spi_info_t *device, uint8_t cmd);
int32_t GENERIC_FSS_RequestData(spi_info_t *device, GENERIC_FSS_Device_Data_tlm_t* data);


#endif /* _GENERIC_FSS_DEVICE_H_ */
