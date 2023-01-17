/************************************************************************
** File:
**   $Id: generic_fss_platform_cfg.h  $
**
** Purpose:
**  Define generic_fss Platform Configuration Parameters
**
** Notes:
**
*************************************************************************/
#ifndef _GENERIC_FSS_PLATFORM_CFG_H_
#define _GENERIC_FSS_PLATFORM_CFG_H_

/*
** Default GENERIC_FSS Configuration
*/
#ifndef GENERIC_FSS_CFG
    /* Notes: 
    **   NOS3 uart requires matching handle and bus number
    */
    #define GENERIC_FSS_CFG_STRING           "usart_28"
    #define GENERIC_FSS_CFG_HANDLE           28 
    #define GENERIC_FSS_CFG_BAUDRATE_HZ      115200
    #define GENERIC_FSS_CFG_MS_TIMEOUT       50            /* Max 255 */
    /* Note: Debug flag disabled (commented out) by default */
    //#define GENERIC_FSS_CFG_DEBUG
#endif

#endif /* _GENERIC_FSS_PLATFORM_CFG_H_ */
