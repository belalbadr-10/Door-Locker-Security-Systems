/******************************************************************************
 *
 * Module: Main Application for MC2
 *
 * File Name: MAin_App.c
 *
 * Description: Source file for the main application of MC2
 *
 * Author: Belal Badr
 *
 *******************************************************************************/
#include "buzzer.h"
#include "external_eeprom.h"
#include "Dc_Motor.h"
#include "uart.h"
#include "timer.h"
#include "twi.h"
#include <string.h>

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

/* Extern the global variable in timer, UART and I2C files */
extern Timer_ConfigType TIMER0_config;
extern UART_ConfigType UART_config;
extern Twi_ConfigType TWI_config;

/* global variable contain the ticks count of the timer */
uint32 g_tick=0;

/* declaring an array for the received password */
uint8 password_received[10];

/* declaring an array for the stored password */
uint8 password_real[10];

/* declaring a variable to check the confirmed password matches with the original one */
uint8 error_check = 0;

#define UN_MATCHED       1
#define MATCHED          0

/* define timer constants */
#define NUMBER_OF_OVERFLOWS_PER_SECOND          500
#define NUMBER_OF_OVERFLOWS_PER_MILLI_SECOND    5

/* EEPROM position to be stored in */
#define EEPROM_STORAGE_PLACE                    0x0311

/* shared Commands between MC1 & MC2 */
#define CORRECT_PASSWORD          (0x01)
#define WRONG_PASSWORD            (0x02)
#define OPEN_DOOR                 (0x03)
#define FIRE_BUZZER               (0x04)
#define CHANGE_PASSWORD           (0x05)
#define CHECK_PASSWORD            (0x06)

/* Shared condition to make sure that MC2 is ready to receive new data */
#define MC2_READY                 (0x10)
/*******************************************************************************
 *                      Functions Definitions                                  *
 *******************************************************************************/

/*
 * Description :
 * Call-back function that is called to be done when the timer interrupt occurs
 */
void _TIMER0_CALLBACK(void)
{
	/* incrementing the number of ticks whenever the timer interrupt happens */
	g_tick++;
}


/*
 * Description :
 * Delay Function that works to make specific delays based on the user choice
 */
void _delay_second(uint8 seconds)
{
	/*Setting up the configuration object for the timer */
	TIMER0_config.clock = F_CPU_8;
	TIMER0_config.compare_value = 0;
	TIMER0_config.initial_value = 0;
	TIMER0_config.mode = NORMAL;

	/* Setting up & initializing the timer before starting up */
	 Timer0_setCallBack(_TIMER0_CALLBACK);
	 Timer0_Init(&TIMER0_config);

	 /* while loop to hold on the processor until the required delay time occures */
	 while (g_tick <(seconds * NUMBER_OF_OVERFLOWS_PER_SECOND));
	 g_tick=0;

	 /* Closing the timer after using it */
	 Timer_DeInit();
}

/*
 * Description :
 * Delay Function that works to make specific delays based on the user choice
 */
void _delay_milli_second(uint32 m_seconds)
{
	/*Setting up the configuration object for the timer */
	TIMER0_config.clock = F_CPU_8;
	TIMER0_config.compare_value = 0;
	TIMER0_config.initial_value = 0;
	TIMER0_config.mode = NORMAL;

	/* Setting up & initializing the timer before starting up */
	 Timer0_setCallBack(_TIMER0_CALLBACK);
	 Timer0_Init(&TIMER0_config);

	 /* while loop to hold on the processor until the required delay time occurs */
	 while (g_tick <(m_seconds * NUMBER_OF_OVERFLOWS_PER_MILLI_SECOND));
	 g_tick=0;

	 /* Closing the timer after using it */
	 Timer_DeInit();
}


/*
 * Description :
 * Function that handles the operation of buzzer
 */
void TURN_ON_BUZZER(void)
{
	/* Turn on the buzzer */
	BUZZER_on();

	/* Wait for 10 seconds */
	_delay_second(10);

	/* Turn off the buzzer */
	BUZZER_off();
}


/*
 * Description :
 * Function that handles the opening & closing of the door
 * 1. Open the door by turning the motor on in the Anti-clockwise direction for 15 seconds
 * 2. hold the door opened for 3 seconds
 * 3. Close the door by turning the motor on in the clockwise direction for 15 seconds
 * 4. Stop the motor
 */
void DOOR_OPERATION(void)
{
	/* Rotate the motor Anti-clokwise for 15 seconds until the door is opened */
	DcMotor_Rotate(ANTI_CLOCKWISE);
	_delay_second(15);

	/* wait for 3 seconds keeping the door open */
	DcMotor_Rotate(STOP);
	_delay_second(3);

	/* Rotate the motor Clockwise for 15 seconds until the door is closed */
	DcMotor_Rotate(CLOCKWISE);
	_delay_second(15);

	/* Stop the motor after using it */
	DcMotor_Rotate(STOP);
}

/*
 * Description :
 * Function that stores the password in the EEPROM
 */
void STORE_PASSWORD(void)
{

	/* variable to store the password byte by byte */
	uint8 byte;

	/* declaring a variable to be used in the for loop */
	uint8 i;

	/*declaring a dummy variable to store the return value from the write function */
	uint8 dummy;

	/* for loop to store the Password in the EEPROM */
	for(i=0 ; i<5; i++)
	{
		byte = password_real[i];
		dummy = EEPROM_writeByte((EEPROM_STORAGE_PLACE + i), byte);

		/* delay for 100 milli second until the byte is stored */
		_delay_milli_second(10);
	}

	/* Clear the contents of password_real array */
	strcpy(password_real,"\0");

	return;
}



/*
 * Description :
 * Function that changes the password
 * 1. it receives the password from the user
 * 2 stores the password in a specific place in the EEPROM
 */
void PASSWORD_CHANGE(void)
{
	/*Sending a message for MC1 to let it know that MC2 is ready */
	UART_sendByte(MC2_READY);

	/*receiving the new password */
	UART_receiveString(password_real);

	/* storing the password in EEPROM */
	STORE_PASSWORD();
}


/*
 * Description :
 * Function that gets the stored password in EEPROM
 */
void GET_STORED_PASSWORD(void)
{
	/* variable to store the password byte by byte */
		uint8 byte;

		/* declaring a variable to be used in the for loop */
		uint8 i;

		/*declaring a dummy variable to store the return value from the write function */
		uint8 dummy;

		/* for loop to store the Password in the EEPROM */
		for(i=0 ; i<5; i++)
		{
			dummy = EEPROM_readByte((EEPROM_STORAGE_PLACE + i), &byte);
			password_real[i]=byte;

			/* delay for 100 milli second until the byte is stored */
			_delay_milli_second(10);
		}

		/* terminating the saved password by NULL */
		password_real[5] = '\0';

		return;
}


/*
 * Description :
 * Function that compares the received password with the stored one
 */
void COMPARE_PASSWORD(void)
{
	/*Sending a message for MC1 to let it know that MC2 is ready */
	UART_sendByte(MC2_READY);

	/*receiving the password */
	UART_receiveString(password_received);

	/* get the stored password in the password array */
	GET_STORED_PASSWORD();

	/* comparing between the stored variable and the received one */
	if((strcmp(password_real,password_received)) == 0)
		error_check = MATCHED;
	else
		error_check = UN_MATCHED;

	/* clear the content of the password arrays */
	strcpy(password_real,"\0");
	strcpy(password_received,"\0");
}


/*******************************************************************************
 *******************************************************************************
 *                             Main Function                                   *
 *******************************************************************************
 *******************************************************************************/
int main(void)
{
	/* declaring choice variable to use it in the program */
	uint8 choice = 0;

	/*Setting up the Configuration object for I2C */
	TWI_config.Bit_Rate = Fast_mode;
	TWI_config.address = 0b00000010;

	/* Initializing I2C */
	TWI_init(&TWI_config);

	/*Setting up the Configuration object for UART */
	UART_config.Baud_Rate = 9600;
	UART_config.Bits_Number = _8_BITS;
	UART_config.Parity = EVEN_PARITY;
	UART_config.Stop_Bits_Number = _1_STOP_BIT;

	/* Initializing UART */
	UART_init(&UART_config);

	/* initializing the motor */
	DcMotor_Init();

	/*Initializing the buzzer */
	BUZZER_init();

	/* MC2 takes the password for the first time and stores it in EEPROM  */

	/*receiving the password from MC1 in password_real array as it's the first time to recive
	  the password
	 */
	UART_receiveString(password_real);

	/*store the password in EEPROM */
	STORE_PASSWORD();

	/* Now going through the program of MC2
	 * 1. checks for the received password is it correct or not
	 * 2. fires the buzzer if requested
	 * 3. opens the door using the motor if requested
	 * 4.changes the password if requested
	 */


	while(1)
	{

		/*Sending a message for MC1 to let it know that MC2 is ready */
		UART_sendByte(MC2_READY);

		/*receiving the request from MC1 */
		choice = UART_recieveByte();

		if(choice == OPEN_DOOR)
		{
			/* call the function to open the door */
			DOOR_OPERATION();
		}
		else if(choice == FIRE_BUZZER)
		{
			/* call the function to fire the buzzer */
			TURN_ON_BUZZER();
		}
		else if(choice == CHECK_PASSWORD)
		{
			/* call the function that checks for entered password */
			COMPARE_PASSWORD();

			/* checking on the state of the received password */
			if(error_check == MATCHED)
			{
				UART_sendByte(CORRECT_PASSWORD);
			}
			else if(error_check == UN_MATCHED)
			{
				UART_sendByte(WRONG_PASSWORD);
			}
		}
		else if(choice == CHANGE_PASSWORD)
		{
			/* call a function that changes the password */
			PASSWORD_CHANGE();
		}
	}
}
