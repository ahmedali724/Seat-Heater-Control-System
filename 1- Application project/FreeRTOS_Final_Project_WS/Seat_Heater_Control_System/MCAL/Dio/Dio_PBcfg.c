/*
 ============================================================================
 Name        : Dio_PBcfg.c
 Module Name : Dio
 Author      : Ahmed Ali
 Date        : 7 Aug. 2024
 Description : Post Build Configuration Source file for TM4C123GH6PM Microcontroller - Dio Driver
 ============================================================================
 */

#include "Dio.h"

/* AUTOSAR Version checking between Dio_PBcfg.c and Dio.h files */
#if ((DIO_PBCFG_AR_RELEASE_MAJOR_VERSION != DIO_AR_RELEASE_MAJOR_VERSION)\
 ||  (DIO_PBCFG_AR_RELEASE_MINOR_VERSION != DIO_AR_RELEASE_MINOR_VERSION)\
 ||  (DIO_PBCFG_AR_RELEASE_PATCH_VERSION != DIO_AR_RELEASE_PATCH_VERSION))
  #error "The AR version of Dio_PBcfg.c does not match the expected version"
#endif

/* Software Version checking between Dio_PBcfg.c and Dio.h files */
#if ((DIO_PBCFG_SW_MAJOR_VERSION != DIO_SW_MAJOR_VERSION)\
 ||  (DIO_PBCFG_SW_MINOR_VERSION != DIO_SW_MINOR_VERSION)\
 ||  (DIO_PBCFG_SW_PATCH_VERSION != DIO_SW_PATCH_VERSION))
  #error "The SW version of Dio_PBcfg.c does not match the expected version"
#endif

/* PB structure used with Dio_Init API */
const Dio_ConfigType Dio_Configuration =
{
     DioConf_SW1_PORT_NUM, DioConf_SW1_CHANNEL_NUM,
     DioConf_SW2_PORT_NUM, DioConf_SW2_CHANNEL_NUM,
     DioConf_SW3_PORT_NUM, DioConf_SW3_CHANNEL_NUM,
     DioConf_LED_RED1_PORT_NUM, DioConf_LED_RED1_CHANNEL_NUM,
     DioConf_LED_GREEN1_PORT_NUM, DioConf_LED_GREEN1_CHANNEL_NUM,
     DioConf_LED_BLUE1_PORT_NUM, DioConf_LED_BLUE1_CHANNEL_NUM,
     DioConf_LED_RED2_PORT_NUM, DioConf_LED_RED2_CHANNEL_NUM,
     DioConf_LED_GREEN2_PORT_NUM, DioConf_LED_GREEN2_CHANNEL_NUM,
     DioConf_LED_BLUE2_PORT_NUM, DioConf_LED_BLUE2_CHANNEL_NUM
};

