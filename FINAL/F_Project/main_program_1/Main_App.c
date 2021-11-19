/******************************************************************************
 *
 * Module: Main Application for MC1
 *
 * File Name: MAin_App.c
 *
 * Description: Source file for the main application of MC1
 *
 * Author: Belal Badr
 *
 *******************************************************************************/
#include "lcd.h"
#include "keypad.h"
#include "timer.h"
#include "uart.h"
#include <string.h>

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

/* Extern the global variable in timer and UART files */
extern Timer_ConfigType TIMER0_config;
extern UART_ConfigType UART_config;

/* global variable contain the ticks count of the timer */
uint32 g_tick=0;

/* declaring an array for the password */
uint8 password_arr[10] = "p1";

/* declaring an array to confirm the entered password */
uint8 password_confirm_arr[10] = "p2";

/* declaring a variable to recognize whether it's the first time to confirm password or not */
uint8 confirm_check = 0;

/* declaring a variable to check the confirmed password matches with the original one */
uint8 error_check = 0;

#define ERROR       1
#define CLEAR       0

/* define timer constants */
#define NUMBER_OF_OVERFLOWS_PER_SECOND          500
#define NUMBER_OF_OVERFLOWS_PER_MILLI_SECOND    5


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
 * Function to read the password and confirm it:
 * 1. takes the password from the user and confirms it
 * 2. if the password is correct, it changes error_check variable to zero
 * 3. if the password has been entered wrong for 3 times in row, the buzzer is fired & changes
 *    error_check variable to one
 */

void Read_Password(void)
{
	/* Variable to be used in the for loop */
	uint8 i;

	/* declaring a variable for password state */
	uint8 password_State;

	/* declaring a variable for error numbers */
	uint8 error_count = 0;

	/* Initializing the two password arrays before taking inputs from user */
	strcpy(password_arr,"p1");
	strcpy(password_confirm_arr,"p2");

	LCD_clearScreen();



	/* While loop to confirm that the input password is correctly taken from the user */
	while(error_count < 3)
	{

		while(strcmp(password_arr,password_confirm_arr) != 0)
		{

			/* checks if this is the first time the program enters this loop or not */
			if(confirm_check != 0)
			{
				LCD_displayStringRowColumn(0,0,"Password not");
				LCD_displayStringRowColumn(1,0,"Confirmed");
				LCD_displayStringRowColumn(2,0,"Please re-enter");
				LCD_displayStringRowColumn(3,0,"your password !");

				/* delay for 5 seconds until the user reads the message */
				_delay_second(5);

				LCD_clearScreen();
			}

			/*Takes the password from the user and send it to MC2 */
			LCD_displayStringRowColumn(1,0,"Enter Your");
			LCD_displayStringRowColumn(2,0,"Password !");
			LCD_moveCursor(3,6);

			/* A Loop to take the input password from the user */
			for(i=0; i<5; i++)
			{
				password_arr[i] =  KEYPAD_getPressedKey();
				LCD_displayCharacter('*');

				/* delay for 500 milli second until the button is released */
				_delay_milli_second(50);
			}
			/* Inserting '#' at the end of the password so that the MC2 Stops on reading it */
			password_arr[5] = '#';
			/* Inserting the null '\0' at the end of the password so that the MC1
		       stops on sending it */
			password_arr[6] = '\0';

			LCD_clearScreen();

			/*confirming the password entered from the user*/
			LCD_displayStringRowColumn(1,0,"Please Confirm");
			LCD_displayStringRowColumn(2,0,"your password !");
			LCD_moveCursor(3,6);

			/* A Loop to take the input password from the user */
			for(i=0; i<5; i++)
			{
				password_confirm_arr[i] =  KEYPAD_getPressedKey();
				LCD_displayCharacter('*');


				/* delay for 500 milli second until the button is released */
				_delay_milli_second(50);
			}
			/* Inserting '#' at the end of the password so that the MC2 Stops on reading it */
			password_confirm_arr[5] = '#';
			/* Inserting the null '\0' at the end of the password so that the MC1
		       stops on sending it */
			password_confirm_arr[6] = '\0';

			LCD_clearScreen();

			/* changing the value of the confirm_check variable to recognize that the loop has
			   been entered  */
			confirm_check = 1;
		}

		/* re-set the value of confirm_check variable */
		confirm_check = 0;

		/* Wait until MC2 is ready*/
		while(UART_recieveByte() != MC2_READY){}

		/*Request for password check */
		UART_sendByte(CHECK_PASSWORD);

		/* Wait until MC2 is ready*/
		while(UART_recieveByte() != MC2_READY){}

		/*sending the confirmed password for MC2 */
		UART_sendString(password_arr);

		LCD_displayStringRowColumn(0,3,"PROCESSING");

		/* wait 2 seconds until MC2 checks on the password */
		_delay_second(2);
		LCD_clearScreen();

		/* Checks about the condition of the password */
		password_State = UART_recieveByte();
		if(password_State == CORRECT_PASSWORD)
		{
			LCD_displayStringRowColumn(0,0,"Correct Password");
			_delay_second(2);

			error_check = CLEAR;
			return;
		}
		else if(password_State == WRONG_PASSWORD)
		{
			LCD_displayStringRowColumn(0,0,"Wrong Password");
			_delay_second(2);

			error_check = ERROR;
			error_count ++;
		}

		LCD_clearScreen();

		/* initializing the two arrays again so we can enter the read password loop again */
		strcpy(password_arr,"p1");
		strcpy(password_confirm_arr,"p2");
	}

	/* Wait until MC2 is ready*/
	while(UART_recieveByte() != MC2_READY){}

	/* IF it ever gets outside while loop, it means that password is entered wrong 3
	 * times in row, so we fire buzzer */
	UART_sendByte(FIRE_BUZZER);

	/*Buzzer is fired for seconds */
	LCD_displayStringRowColumn(0,0,"WRONG PASSWORD");
	LCD_displayStringRowColumn(1,0,"BUZZER ON");

	_delay_second(10);

	LCD_clearScreen();
	return ;
}


/*
 * Description :
 * Function to handle all operations done in the open door choice:
 * 1. if the password is correct it, it opens the opens the door
 * 2. if the password has been entered wrong
 */

void DOOR_CHOICE(void)
{

	LCD_clearScreen();

	/* call the Read_Password function to test the password */
	Read_Password();

	/* Now checking on the error_check variable */
	if(error_check == CLEAR)
	{
		LCD_clearScreen();


		/* Wait until MC2 is ready*/
		while(UART_recieveByte() != MC2_READY){}

		UART_sendByte(OPEN_DOOR);

		LCD_displayStringRowColumn(0,0,"Opening the");
		LCD_displayStringRowColumn(1,0,"Door");

		/* wait for 15 seconds until the door is opened */
		_delay_second(15);

		/* Keep the door open for 3 seconds */
		_delay_second(3);

		/* wait for 15 seconds until the door closed */
		_delay_second(15);
	}

	LCD_clearScreen();
	return;
}


/*
 * Description :
 * Function to handle all operations done in the change password choice:
 * 1. if the password is correct it, password is taken from the user to be changed
 * 2. if the password has been entered wrong, it returns to the main menu
 */
void CHANGE_PASSWORD_CHOICE(void)
{
	/* Variable to be used in the for loop */
	uint8 i;

	LCD_clearScreen();

	/* Initializing the two password arrays before taking inputs from user */
	strcpy(password_arr,"p1");
	strcpy(password_confirm_arr,"p2");

	/* call the Read_Password function to test the password */
	Read_Password();


	/* Initializing the two password arrays before taking inputs from user */
	strcpy(password_arr,"p1");
	strcpy(password_confirm_arr,"p2");

	/* Now checking on the error_check variable */
	if(error_check == CLEAR)
	{
		/* Now taking the new password to be changed */
		LCD_clearScreen();

		/* While loop to confirm that the input password is correctly taken from the user */
		while(strcmp(password_arr,password_confirm_arr) != 0)
		{

			/* checks if this is the first time the program enters this loop or not */
			if(confirm_check != 0)
			{
				LCD_displayStringRowColumn(0,0,"Password not");
				LCD_displayStringRowColumn(1,0,"Confirmed");
				LCD_displayStringRowColumn(2,0,"Please re-enter");
				LCD_displayStringRowColumn(3,0,"your password !");

				/* delay for 5 seconds until the user reads the message */
				_delay_second(5);

				LCD_clearScreen();
			}

			/*Takes the password from the user and send it to MC2 */
			LCD_displayStringRowColumn(1,0,"Enter Your");
			LCD_displayStringRowColumn(2,0,"New Password !");
			LCD_moveCursor(3,6);

			/* A Loop to take the input password from the user */
			for(i=0; i<5; i++)
			{
				password_arr[i] =  KEYPAD_getPressedKey();
				LCD_displayCharacter('*');


				/* delay for 500 milli second until the button is released */
				_delay_milli_second(50);
			}
			/* Inserting '#' at the end of the password so that the MC2 Stops on reading it */
			password_arr[5] = '#';
			/* Inserting the null '\0' at the end of the password so that the MC1
		       stops on sending it */
			password_arr[6] = '\0';

			LCD_clearScreen();

			/*confirming the password entered from the user*/
			LCD_displayStringRowColumn(1,0,"Please Confirm");
			LCD_displayStringRowColumn(2,0,"your password !");
			LCD_moveCursor(3,6);

			/* A Loop to take the input password from the user */
			for(i=0; i<5; i++)
			{
				password_confirm_arr[i] =  KEYPAD_getPressedKey();
				LCD_displayCharacter('*');


				/* delay for 500 milli second until the button is released */
				_delay_milli_second(50);
			}
			/* Inserting '#' at the end of the password so that the MC2 Stops on reading it */
			password_confirm_arr[5] = '#';
			/* Inserting the null '\0' at the end of the password so that the MC1
		       stops on sending it */
			password_confirm_arr[6] = '\0';

			LCD_clearScreen();

			/* changing the value of the confirm_check variable to recognize that the loop has
			   been entered  */
			confirm_check = 1;
		}

		/* re-set the value of confirm_check variable */
		confirm_check = 0;

		/* Wait until MC2 is ready*/
		while(UART_recieveByte() != MC2_READY){}

		/* Send the the change password command for MC2 */
		UART_sendByte(CHANGE_PASSWORD);

		/* Wait until MC2 is ready*/
		while(UART_recieveByte() != MC2_READY){}

		/*sending the new confirmed password for MC2 */
		UART_sendString(password_arr);
	}

	return;
}

/*******************************************************************************
 *******************************************************************************
 *                             Main Function                                   *
 *******************************************************************************
 *******************************************************************************/

int main(void)
{
	/* Variable to be used in the for loop */
	uint8 i;

	/* declaring choice variable to use it in the program */
	uint8 choice = 0;

	LCD_init();

	/*Setting up the Configuration object for UART */
	UART_config.Baud_Rate = 9600;
	UART_config.Bits_Number = _8_BITS;
	UART_config.Parity = EVEN_PARITY;
	UART_config.Stop_Bits_Number = _1_STOP_BIT;

	/* Initializing UART */
	UART_init(&UART_config);

	/* The Program starts as follows
	 * 1. Show a welcome message for the user
	 * 2. Takes the password from the user and send it to MC2
	 * 3. Menu shows on the LCD to choose between opening the door or changing the password
	 * 4. if it's required to change the password, the existing password is required twice then
	 *    you can change the password
	 * 5. if it's required to open the door, the existing password is required twice then the
	 *    the door is opened
	 * 6. if the wring password entered for 3 times in row, buzzer is fired for 10 seconds
	 */


	/* Showing the welcome message */
	LCD_displayStringRowColumn(0,3,"Welcome !");

	/* While loop to confirm that the input password is correctly taken from the user */
	while((strcmp(password_arr,password_confirm_arr)) != 0)
	{

		/* checks if this is the first time the program enters this loop or not */
		if(confirm_check != 0)
		{
			LCD_displayStringRowColumn(0,0,"Password not");
			LCD_displayStringRowColumn(1,0,"Confirmed");
			LCD_displayStringRowColumn(2,0,"Please re-enter");
			LCD_displayStringRowColumn(3,0,"your password !");

			/* delay for 5 seconds until the user reads the message */
			_delay_second(5);

			LCD_clearScreen();
		}


		/*Takes the password from the user and send it to MC2 */
		LCD_displayStringRowColumn(1,0,"Enter Password");
		LCD_displayStringRowColumn(2,0,"of 5 numbers !");
		LCD_moveCursor(3,6);

		/* A Loop to take the input password from the user */
		for(i=0; i<5; i++)
		{
			password_arr[i] =  KEYPAD_getPressedKey();
			LCD_displayCharacter('*');

			/* delay for 500 milli second until the button is released */
			_delay_milli_second(50);

		}
		/* Inserting '#' at the end of the password so that the MC2 Stops on reading it */
		password_arr[5] = '#';
		/* Inserting the null '\0' at the end of the password so that the MC1
	       stops on sending it */
		password_arr[6] = '\0';

		LCD_clearScreen();

		/*confirming the password entered from the user*/
		LCD_displayStringRowColumn(1,0,"Please Confirm");
		LCD_displayStringRowColumn(2,0,"your password !");
		LCD_moveCursor(3,6);

		/* A Loop to take the input password from the user */
		for(i=0; i<5; i++)
		{
			password_confirm_arr[i] =  KEYPAD_getPressedKey();
			LCD_displayCharacter('*');

			/* delay for 500 milli second until the button is released */
			_delay_milli_second(50);

		}
		/* Inserting '#' at the end of the password so that the MC2 Stops on reading it */
		password_confirm_arr[5] = '#';
		/* Inserting the null '\0' at the end of the password so that the MC1
	       stops on sending it */
		password_confirm_arr[6] = '\0';

		LCD_clearScreen();

		/* changing the value of the confirm_check variable to recognize that the loop has
		   been entered  */
		confirm_check = 1;
	}

	/* re-set the value of confirm_check variable */
	confirm_check = 0;

	/*sending the confirmed password for MC2 */
	UART_sendString(password_arr);


	/* Showing the menu to choose between opening the door or changing the password */
	while(1)
	{
		LCD_displayStringRowColumn(1,0,"+: Open the door");
		LCD_displayStringRowColumn(2,0,"-: Change the");
		LCD_displayStringRowColumn(3,3,"password");

		/* taking the choice from the user*/
		choice = KEYPAD_getPressedKey();

		switch(choice)
		{
		case '+': DOOR_CHOICE();
		          break;
		case '-': CHANGE_PASSWORD_CHOICE();
		          break;
		}
		LCD_clearScreen();
	}
}
