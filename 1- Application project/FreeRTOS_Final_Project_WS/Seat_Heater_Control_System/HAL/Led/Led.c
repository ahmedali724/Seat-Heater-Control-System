/*
 ============================================================================
 Name        : Led.c
 Module Name : Led
 Author      : Ahmed Ali
 Date        : 22 Sept. 2024
 Description : Source file for Led Module
 ============================================================================
 */

#include "Dio.h"
#include "Led.h"

/* Description: Set the LED RED1 state to ON */
void Led_RED1_SetOn(void)
{
    Dio_WriteChannel(DioConf_LED_RED1_CHANNEL_ID_INDEX, LED_ON); /* LED RED1 ON */
}

/* Description: Set the LED RED1 state to OFF */
void Led_RED1_SetOff(void)
{
    Dio_WriteChannel(DioConf_LED_RED1_CHANNEL_ID_INDEX, LED_OFF); /* LED RED1 OFF */
}

/*Description: Toggle the RED1 LED state */
void Led_RED1_Toggle(void)
{
    Dio_LevelType state = Dio_FlipChannel(DioConf_LED_RED1_CHANNEL_ID_INDEX);
}

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED RED2 state to ON */
void Led_RED2_SetOn(void)
{
    Dio_WriteChannel(DioConf_LED_RED2_CHANNEL_ID_INDEX, LED_ON); /* LED RED2 ON */
}

/* Description: Set the LED RED2 state to OFF */
void Led_RED2_SetOff(void)
{
    Dio_WriteChannel(DioConf_LED_RED2_CHANNEL_ID_INDEX, LED_OFF); /* LED RED2 OFF */
}

/*Description: Toggle the RED2 LED state */
void Led_RED2_Toggle(void)
{
    Dio_LevelType state = Dio_FlipChannel(DioConf_LED_RED2_CHANNEL_ID_INDEX);
}

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED GREEN1 state to ON */
void Led_GREEN1_SetOn(void)
{
    Dio_WriteChannel(DioConf_LED_GREEN1_CHANNEL_ID_INDEX, LED_ON); /* LED GREEN1 ON */
}

/* Description: Set the LED GREEN1 state to OFF */
void Led_GREEN1_SetOff(void)
{
    Dio_WriteChannel(DioConf_LED_GREEN1_CHANNEL_ID_INDEX, LED_OFF); /* LED GREEN1 OFF */
}

/*Description: Toggle the GREEN1 LED state */
void Led_GREEN1_Toggle(void)
{
    Dio_LevelType state = Dio_FlipChannel(DioConf_LED_GREEN1_CHANNEL_ID_INDEX);
}

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED GREEN2 state to ON */
void Led_GREEN2_SetOn(void)
{
    Dio_WriteChannel(DioConf_LED_GREEN2_CHANNEL_ID_INDEX, LED_ON); /* LED GREEN2 ON */
}

/* Description: Set the LED GREEN2 state to OFF */
void Led_GREEN2_SetOff(void)
{
    Dio_WriteChannel(DioConf_LED_GREEN2_CHANNEL_ID_INDEX, LED_OFF); /* LED GREEN2 OFF */
}

/*Description: Toggle the GREEN2 LED state */
void Led_GREEN2_Toggle(void)
{
    Dio_LevelType state = Dio_FlipChannel(DioConf_LED_GREEN2_CHANNEL_ID_INDEX);
}

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED BLUE1 state to ON */
void Led_BLUE1_SetOn(void)
{
    Dio_WriteChannel(DioConf_LED_BLUE1_CHANNEL_ID_INDEX, LED_ON); /* LED BLUE1 ON */
}

/* Description: Set the LED BLUE1 state to OFF */
void Led_BLUE1_SetOff(void)
{
    Dio_WriteChannel(DioConf_LED_BLUE1_CHANNEL_ID_INDEX, LED_OFF); /* LED BLUE1 OFF */
}

/*Description: Toggle the BLUE1 LED state */
void Led_BLUE1_Toggle(void)
{
    Dio_LevelType state = Dio_FlipChannel(DioConf_LED_BLUE1_CHANNEL_ID_INDEX);
}

/*--------------------------------------------------------------------------------------*/

/* Description: Set the LED BLUE2 state to ON */
void Led_BLUE2_SetOn(void)
{
    Dio_WriteChannel(DioConf_LED_BLUE2_CHANNEL_ID_INDEX, LED_ON); /* LED BLUE2 ON */
}

/* Description: Set the LED BLUE2 state to OFF */
void Led_BLUE2_SetOff(void)
{
    Dio_WriteChannel(DioConf_LED_BLUE2_CHANNEL_ID_INDEX, LED_OFF); /* LED BLUE2 OFF */
}

/*Description: Toggle the BLUE2 LED state */
void Led_BLUE2_Toggle(void)
{
    Dio_LevelType state = Dio_FlipChannel(DioConf_LED_BLUE2_CHANNEL_ID_INDEX);
}

/*--------------------------------------------------------------------------------------*/
