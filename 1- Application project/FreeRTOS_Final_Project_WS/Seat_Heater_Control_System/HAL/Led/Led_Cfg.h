/*
 ============================================================================
 Name        : Led_Cfg.h
 Module Name : Led
 Author      : Ahmed Ali
 Date        : 22 Sept. 2024
 Description : Header file for Pre-Compiler configuration used by Led Module
 ============================================================================
 */

#ifndef LED_CFG_H_
#define LED_CFG_H_

/* Set the led ON/OFF according to its configuration Positive logic or negative logic */
#define LED_ON      STD_HIGH
#define LED_OFF     STD_LOW

/*--------------------------------------------------------------------------------------*/

/* Set the LED RED1 Port */
#define LED_RED1_PORT               DioConf_LED_RED1_PORT_NUM

/* Set the LED RED1 Pin Number */
#define LED_RED1_PIN_NUM            DioConf_LED_RED1_CHANNEL_NUM

/*--------------------------------------------------------------------------------------*/

/* Set the LED RED2 Port */
#define LED_RED2_PORT               DioConf_LED_RED2_PORT_NUM

/* Set the LED RED2 Pin Number */
#define LED_RED2_PIN_NUM            DioConf_LED_RED2_CHANNEL_NUM

/*--------------------------------------------------------------------------------------*/

/* Set the LED GREEN1 Port */
#define LED_GREEN1_PORT             DioConf_LED_GREEN1_PORT_NUM

/* Set the LED GREEN1 Pin Number */
#define LED_GREEN1_PIN_NUM          DioConf_LED_GREEN1_CHANNEL_NUM

/*--------------------------------------------------------------------------------------*/

/* Set the LED GREEN2 Port */
#define LED_GREEN2_PORT             DioConf_LED_GREEN2_PORT_NUM

/* Set the LED GREEN2 Pin Number */
#define LED_GREEN2_PIN_NUM          DioConf_LED_GREEN2_CHANNEL_NUM

/*--------------------------------------------------------------------------------------*/

/* Set the LED BLUE1 Port */
#define LED_BLUE1_PORT              DioConf_LED_BLUE1_PORT_NUM

/* Set the LED BLUE1 Pin Number */
#define LED_BLUE1_PIN_NUM           DioConf_LED_BLUE1_CHANNEL_NUM

/*--------------------------------------------------------------------------------------*/

/* Set the LED BLUE2 Port */
#define LED_BLUE2_PORT              DioConf_LED_BLUE2_PORT_NUM

/* Set the LED BLUE2 Pin Number */
#define LED_BLUE2_PIN_NUM           DioConf_LED_BLUE2_CHANNEL_NUM

/*--------------------------------------------------------------------------------------*/

#endif /* LED_CFG_H_ */
