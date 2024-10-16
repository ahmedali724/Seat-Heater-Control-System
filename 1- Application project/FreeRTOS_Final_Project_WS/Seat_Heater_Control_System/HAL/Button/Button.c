/*
 ============================================================================
 Name        : Button.c
 Module Name : Button
 Author      : Ahmed Ali
 Date        : 22 Sept. 2024
 Description : Source file for Button Module
 ============================================================================
 */

#include "Button.h"
#include "NVIC.h"
#include "tm4c123gh6pm_registers.h"

/* GPIO configuration and interrupt initialization for Port F (PF0, PF4) and Port B (PB1) */
void GPIO_SetupButtonsInterrupt(void)
{
    /* Enable falling edge trigger for PF0 and PF4 */
    GPIO_PORTF_IS_REG &= ~(PF0 | PF4); /* Edge-sensitive */
    GPIO_PORTF_IBE_REG &= ~(PF0 | PF4); /* Not both edges */
    GPIO_PORTF_IEV_REG &= ~(PF0 | PF4); /* Falling edge */
    GPIO_PORTF_ICR_REG |= (PF0 | PF4); /* Clear any prior interrupts */
    GPIO_PORTF_IM_REG |= (PF0 | PF4); /* Enable PF0 and PF4 interrupts */

    /* Enable falling edge trigger for PB1 */
    GPIO_PORTB_IS_REG &= ~PB1; /* Edge-sensitive */
    GPIO_PORTB_IBE_REG &= ~PB1; /* Not both edges */
    GPIO_PORTB_IEV_REG &= ~PB1; /* Falling edge */
    GPIO_PORTB_ICR_REG |= PB1; /* Clear any prior interrupt */
    GPIO_PORTB_IM_REG |= PB1; /* Enable PB0 interrupt */

    /* Enable NVIC GPIO PORTF IRQ and set its priority */
    NVIC_EnableIRQ(GPIO_PORTF_IRQ_NUM);
    NVIC_SetPriorityIRQ(GPIO_PORTF_IRQ_NUM, GPIO_PORTF_INTERRUPT_PRIORITY);

    /* Enable NVIC GPIO PORTB IRQ and set its priority */
    NVIC_EnableIRQ(GPIO_PORTB_IRQ_NUM);
    NVIC_SetPriorityIRQ(GPIO_PORTB_IRQ_NUM, GPIO_PORTB_INTERRUPT_PRIORITY);
}
