/*
 ============================================================================
 Name        : Led.h
 Module Name : Led
 Author      : Ahmed Ali
 Date        : 22 Sept. 2024
 Description : Header file for Led Module
 ============================================================================
 */

#ifndef LED_H_
#define LED_H_

#include "Std_Types.h"
#include "Led_Cfg.h"

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/* Description: Set the LED RED1 state to ON */
void Led_RED1_SetOn(void);

/* Description: Set the LED RED1 state to OFF */
void Led_RED1_SetOff(void);

/*Description: Toggle the RED1 LED state */
void Led_RED1_Toggle(void);

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED RED2 state to ON */
void Led_RED2_SetOn(void);

/* Description: Set the LED RED2 state to OFF */
void Led_RED2_SetOff(void);

/*Description: Toggle the RED2 LED state */
void Led_RED2_Toggle(void);

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED GREEN1 state to ON */
void Led_GREEN1_SetOn(void);

/* Description: Set the LED GREEN1 state to OFF */
void Led_GREEN1_SetOff(void);

/*Description: Toggle the GREEN1 LED state */
void Led_GREEN1_Toggle(void);

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED GREEN2 state to ON */
void Led_GREEN2_SetOn(void);

/* Description: Set the LED GREEN2 state to OFF */
void Led_GREEN2_SetOff(void);

/*Description: Toggle the GREEN2 LED state */
void Led_GREEN2_Toggle(void);

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED BLUE1 state to ON */
void Led_BLUE1_SetOn(void);

/* Description: Set the LED BLUE1 state to OFF */
void Led_BLUE1_SetOff(void);

/*Description: Toggle the BLUE1 LED state */
void Led_BLUE1_Toggle(void);

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED BLUE2 state to ON */
void Led_BLUE2_SetOn(void);

/* Description: Set the LED BLUE2 state to OFF */
void Led_BLUE2_SetOff(void);

/*Description: Toggle the BLUE2 LED state */
void Led_BLUE2_Toggle(void);

/*--------------------------------------------------------------------------------------*/

#endif /* LED_H_ */
