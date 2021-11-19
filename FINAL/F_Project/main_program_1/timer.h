 /******************************************************************************
 *
 * Module: Timer0
 *
 * File Name: timer.h
 *
 * Description: Header file for the Timer0 AVR driver
 *
 * Author: Belal Badr
 *
 *******************************************************************************/

#include "std_types.h"


typedef enum
{
	NO_CLOCK,F_CPU_CLOCK,F_CPU_8,F_CPU_64,F_CPU_256,F_CPU_1024
}Timer_Prescalar;

typedef enum
{
	NORMAL,CTC=2
}Timer_mode;

typedef struct
{
	Timer_Prescalar clock;
	Timer_mode  mode;
	uint16  initial_value;
	uint16  compare_value;
}Timer_ConfigType;


/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Functional responsible for Initialize Timer0 by:
 * 1. Setup the timer mode (compare/normal).
 * 2. Enable the I-bit.
 * 3. Set up the timer prescalar number.
 * 4. setup the initial value for normal mode / compare value for CTC mode
 */
void Timer0_Init(const Timer_ConfigType * Config_Ptr);


/*
 * Description :
 * Functional responsible to set up the callback function for the timer.
 */
void Timer0_setCallBack(void(*a_ptr)(void));

/*
 * Description :
 * Functional that stops the timer after using it.
 */
void Timer_DeInit(void);
