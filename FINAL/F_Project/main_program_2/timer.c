  /******************************************************************************
 *
 * Module: Timer0
 *
 * File Name: timer.c
 *
 * Description: Source file for the Timer0 AVR driver
 *
 * Author: Belal Badr
 *
 *******************************************************************************/
#include "timer.h"
#include <avr/io.h>
#include "avr/interrupt.h"


Timer_ConfigType TIMER0_config; /* Declaring a global variable for Timer0 configuration */

static volatile void (*g_callBackPtr)(void) = NULL_PTR;

/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

ISR(TIMER0_OVF_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}

ISR(TIMER0_COMP_vect)
{

	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}

void Timer0_Init(const Timer_ConfigType * Config_Ptr)
{
	TCNT0 = ( Config_Ptr->initial_value);  // Set Initial Value to 0
	TCCR0 = (1<<FOC0) | ((Config_Ptr->clock & 0x07) | (TCCR0 & 0XF8));
	 SREG |=(1<<7); //Enable interrupts
	if ((Config_Ptr->mode) == CTC )
	{
		OCR0 = (Config_Ptr->compare_value); // Set Compare Value

		TCCR0 |= (1<<WGM01);
		TIMSK |= (1<<OCIE0); // Enable Timer0 Compare Interrupt

		//DDRB = DDRB | (1<<PB3); // Configure PB3/OC0 Pin as output pin

	}
	else  if ((Config_Ptr->mode) == NORMAL)
	{
		TCCR0 &= ~(1<<WGM01);
		TIMSK |= (1<<TOIE0); // Enable Timer0 Overflow Interrupt
	}

}

void Timer0_setCallBack(void(*a_ptr)(void))
{
	/* Save the address of the Call back function in a global variable */
	g_callBackPtr = a_ptr;
}


void Timer_DeInit(void)
{
	/* Clear All Timer0 Registers */
	TCNT0 = 0;
	OCR0=0;
	/* Disable Timers interrupt */
	TIMSK &= ~(1<<TOIE0) & ~(1<<OCIE0);
	SREG &=~(1<<7);

	g_callBackPtr = NULL_PTR; /* clear the call-back function */
}
