/*
 ============================================================================
 Name        : Button.h
 Module Name : Button
 Author      : Ahmed Ali
 Date        : 22 Sept. 2024
 Description : Header file for Button Module
 ============================================================================
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include "Std_Types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

/* Define masks and constants */
#define PF0     0x01
#define PF4     0x10
#define PB1     0x02

#define GPIO_PORTF_IRQ_NUM                  30
#define GPIO_PORTF_INTERRUPT_PRIORITY       5

#define GPIO_PORTB_IRQ_NUM                  1
#define GPIO_PORTB_INTERRUPT_PRIORITY       5

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/* GPIO configuration and interrupt initialization for Port F (PF0, PF4) and Port B (PB1) */
void GPIO_SetupButtonsInterrupt(void);

#endif /* BUTTON_H_ */
