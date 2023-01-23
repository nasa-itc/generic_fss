/*******************************************************************************
** File: generic_fss_device.c
**
** Purpose:
**   This file contains the source code for the GENERIC_FSS device.
**
*******************************************************************************/

/*
** Include Files
*/
#include "generic_fss_device.h"


/* 
** Generic read data from device
*/
int32_t GENERIC_FSS_ReadData(int32_t handle, uint8_t* read_data, uint8_t data_length)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    int32_t bytes_available = 0;
    uint8_t ms_timeout_counter = 0;

    /* Wait until all data received or timeout occurs */
    bytes_available = uart_bytes_available(handle);
    while((bytes_available < data_length) && (ms_timeout_counter < GENERIC_FSS_CFG_MS_TIMEOUT))
    {
        ms_timeout_counter++;
        OS_TaskDelay(1);
        bytes_available = uart_bytes_available(handle);
    }

    if (ms_timeout_counter < GENERIC_FSS_CFG_MS_TIMEOUT)
    {
        /* Limit bytes available */
        if (bytes_available > data_length)
        {
            bytes_available = data_length;
        }
        
        /* Read data */
        bytes = uart_read_port(handle, read_data, bytes_available);
        if (bytes != bytes_available)
        {
            #ifdef GENERIC_FSS_CFG_DEBUG
                OS_printf("  GENERIC_FSS_ReadData: Bytes read != to requested! \n");
            #endif
            status = OS_ERROR;
        } /* uart_read */
    }
    else
    {
        status = OS_ERROR;
    } /* ms_timeout_counter */

    return status;
}


uint8_t compute_checksum(uint8_t in[], int starting_byte, int number_of_bytes)
{
    uint32_t sum = 0;
    uint8_t checksum;
    for (int i = starting_byte; i < starting_byte + number_of_bytes; i++) {
        sum += in[i];
    }
    checksum = (uint8_t)(sum & 0x000000FF);
    return checksum;
}

/* 
** Generic command to device
** Note that confirming the echoed response is specific to this implementation
*/
int32_t GENERIC_FSS_CommandDevice(int32_t handle, uint8_t cmd_code, uint32_t payload)
{
    int32_t status = OS_SUCCESS;
    int32_t bytes = 0;
    uint8_t write_data[GENERIC_FSS_DEVICE_CMD_SIZE] = {0};
    uint8_t read_data[GENERIC_FSS_DEVICE_CMD_SIZE] = {0};

    payload = CFE_MAKE_BIG32(payload);

    /* Prepare command */
    write_data[0] = GENERIC_FSS_DEVICE_HDR_0;
    write_data[1] = GENERIC_FSS_DEVICE_HDR_1;
    write_data[2] = GENERIC_FSS_DEVICE_HDR_2;
    write_data[3] = GENERIC_FSS_DEVICE_HDR_3;
    write_data[4] = cmd_code;
    write_data[5] = 0x01;
    write_data[6] = compute_checksum(write_data, 4, 2);

    /* Flush any prior data */
    status = uart_flush(handle);
    if (status == UART_SUCCESS)
    {
        /* Write data */
        bytes = uart_write_port(handle, write_data, GENERIC_FSS_DEVICE_CMD_SIZE);
        #ifdef GENERIC_FSS_CFG_DEBUG
            OS_printf("  GENERIC_FSS_CommandDevice[%d] = ", bytes);
            for (uint32_t i = 0; i < GENERIC_FSS_DEVICE_CMD_SIZE; i++)
            {
                OS_printf("%02x", write_data[i]);
            }
            OS_printf("\n");
        #endif
        if (bytes == GENERIC_FSS_DEVICE_CMD_SIZE)
        {
            status = GENERIC_FSS_ReadData(handle, read_data, GENERIC_FSS_DEVICE_CMD_SIZE);
            if (status == OS_SUCCESS)
            {
                /* Confirm echoed response */
                bytes = 0;
                while ((bytes < (int32_t) GENERIC_FSS_DEVICE_CMD_SIZE) && (status == OS_SUCCESS))
                {
                    if (read_data[bytes] != write_data[bytes])
                    {
                        status = OS_ERROR;
                    }
                    bytes++;
                }
            } /* GENERIC_FSS_ReadData */
            else
            {
                #ifdef GENERIC_FSS_CFG_DEBUG
                    OS_printf("GENERIC_FSS_CommandDevice - GENERIC_FSS_ReadData returned %d \n", status);
                #endif
            }
        } 
        else
        {
            #ifdef GENERIC_FSS_CFG_DEBUG
                OS_printf("GENERIC_FSS_CommandDevice - uart_write_port returned %d, expected %d \n", bytes, GENERIC_FSS_DEVICE_CMD_SIZE);
            #endif
        } /* uart_write */
    } /* uart_flush*/
    return status;
}

float fourbytes_little_endian_to_float(uint8_t bytes[])
{
    union {float f; uint32_t u;} fu;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    fu.u = ((uint32_t)bytes[3] << 24) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[1] << 8) | (uint32_t)bytes[0];
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    fu.u = ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
#else
    #error "__BYTE_ORDER__ is not defined"
#endif
    return fu.f;
}

/*
** Request data command
*/
int32_t GENERIC_FSS_RequestData(int32_t handle, GENERIC_FSS_Device_Data_tlm_t* data)
{
    int32_t status = OS_SUCCESS;
    uint8_t read_data[GENERIC_FSS_DEVICE_DATA_SIZE] = {0};

    /* Command device to send HK */
    status = GENERIC_FSS_CommandDevice(handle, GENERIC_FSS_DEVICE_REQ_DATA_CMD, 0);
    if (status == OS_SUCCESS)
    {
        /* Read HK data */
        status = GENERIC_FSS_ReadData(handle, read_data, sizeof(read_data));
        if (status == OS_SUCCESS)
        {
            #ifdef GENERIC_FSS_CFG_DEBUG
                OS_printf("  GENERIC_FSS_RequestData = ");
                for (uint32_t i = 0; i < sizeof(read_data); i++)
                {
                    OS_printf("%02x", read_data[i]);
                }
                OS_printf("\n");
            #endif

            uint8_t checksum = compute_checksum(read_data, 4, 11);
            /* Verify data header, command code, length, and checksum */
            if ((read_data[0]  == GENERIC_FSS_DEVICE_HDR_0) && 
                (read_data[1]  == GENERIC_FSS_DEVICE_HDR_1) && 
                (read_data[2]  == GENERIC_FSS_DEVICE_HDR_2) && 
                (read_data[3]  == GENERIC_FSS_DEVICE_HDR_3) &&
                (read_data[4]  == 0x01)                     &&
                (read_data[5]  == 0x0A)                     &&
                (read_data[15] == checksum))
            {
                // alpha
                data->Alpha = fourbytes_little_endian_to_float(&read_data[6]);
                // beta
                data->Beta = fourbytes_little_endian_to_float(&read_data[10]);
                // error code
                data->ErrorCode = read_data[14];
                
                #ifdef GENERIC_FSS_CFG_DEBUG
                    OS_printf("  Header           = 0x%02x%02x%02x%02x  \n", read_data[0], read_data[1], read_data[2], read_data[3]);
                    OS_printf("  Command          = 0x%08x              \n", read[4]);
                    OS_printf("  Length           = 0x%08x              \n", read[5]);
                    OS_printf("  Checksum         = 0x%08x              \n", read[15]);
                    OS_printf("  Data alpha       = 0x%04x, %d          \n", data->Alpha, data->Alpha);
                    OS_printf("  Data beta        = 0x%04x, %d          \n", data->Beta, data->Beta);
                    OS_printf("  Data error code  = 0x%04x, %d          \n", data->ErrorCode, data->ErrorCode);
                #endif
            }
        } 
        else
        {
            #ifdef GENERIC_FSS_CFG_DEBUG
                OS_printf("  GENERIC_FSS_RequestData: Invalid data read! \n");
            #endif 
            status = OS_ERROR;
        } /* GENERIC_FSS_ReadData */
    }
    else
    {
        #ifdef GENERIC_FSS_CFG_DEBUG
            OS_printf("  GENERIC_FSS_RequestData: GENERIC_FSS_CommandDevice reported error %d \n", status);
        #endif 
    }
    return status;
}
