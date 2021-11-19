 /******************************************************************************
 *
 * Module: Dc_Motor
 *
 * File Name: Dc_Motor.h
 *
 * Description: header file for the ATmega16 Dc Motor driver
 *
 * Author: Belal Badr
 *
 *******************************************************************************/


#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_

#include "std_types.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define MOTOR_PORT_ID                 PORTC_ID
#define MOTOR_INPUT1_PIN_ID           PIN6_ID
#define MOTOR_INPUT2_PIN_ID           PIN7_ID

typedef enum
{
	STOP,CLOCKWISE,ANTI_CLOCKWISE
}DcMotor_State;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 *  The Function responsible for setup the direction for the two
    motor pins through the GPIO driver.
 *  Stop at the DC-Motor at the beginning through the GPIO driver.
 */
void DcMotor_Init(void);

/*
 * Description :
 * The function responsible for rotate the DC Motor CW/ or A-CW or
   stop the motor based on the state input state value..
 */

void DcMotor_Rotate(DcMotor_State state);

#endif /* DC_MOTOR_H_ */
