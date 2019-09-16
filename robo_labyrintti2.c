/*****************************************************************************
 * - Source code for robot
 * - This code is for labyrinth
 * - File: main.c
 * - PSoC Designer 5.1.1875.0
 ****************************************************************************/

#include <stdlib.h>		// itoa(), utoa() etc.
#include <m8c.h>		// part specific constants and macros
#include "PSoCAPI.h"	// PSoC API definitions for all User Modules
//#include <math.h>
#include "Timer8.h"

// Sonar //
#define SONAR_OFFSET 0	// Sonar distance back from measuring point (cm)

#define ECHO_INTMODE_RISING 	Echo_IntCtrl_1_ADDR & Echo_MASK
#define SET_TRIG                Trig_Data_ADDR |= Trig_MASK
#define CLEAR_TRIG              Trig_Data_ADDR &= ~Trig_MASK

// Interrupt Control Register bits
// IntCtrl_0 = 0, IntCtrl_1 = 1 -> Rising Edge
// IntCtrl_0 = 1, IntCtrl_1 = 0 -> Falling Edge

// Servo //
#define SERVO_CENTER 32
#define SERVO_LEFT 10
#define SERVO_RIGHT 51

// H-bridge PWM pulse widths //
#define FAST 160 // 100 %	| Nämä tehty sitä varten, että sähkömoottorit
#define SLOW 160 // ? %		| pyörivät taaksepäin hitaammin.

// Interrupt handlers //
#pragma interrupt_handler echo_ISR
#pragma interrupt_handler timer // KELLO


// Function prototypes //
void motors_forward(void);
void motors_stop(void);
void motors_reverse(void);

void motors_left(void);
void motors_right(void);

void echo_ISR(void);
void trigger_sonar(void);

//int distance(void);

// Global variables //
volatile int aika = 0;
volatile unsigned int echo_length_us = 0;
float distance_cm;
int distance_cm_rounded;


void main(void)
{
	

	// Enable interrupts //
    M8C_EnableGInt;
    M8C_EnableIntMask(INT_MSK0 , INT_MSK0_GPIO);
	
	// Timer
	Timer8_EnableInt();
    Timer8_Start();


	
	// LCD //
	LCD_Start();
	
	// SleepTimer //
	SleepTimer_Start();
	SleepTimer_SetInterval(SleepTimer_64_HZ);
	SleepTimer_EnableInt();
	
	// PWM //
	PWM_left_DisableInt();
	PWM_left_Start();
	
	
	PWM_right_DisableInt();
	PWM_right_Start();
	

	while(1){

		motors_forward();
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		
		
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_right();
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_forward();
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		
		
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_right();
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_forward();
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		
		
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_right();
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_forward();
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		
		
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		// ASDS
		
		motors_left();
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_forward();
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		
		
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_left();
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_forward();
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		
		
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_left();
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		motors_forward();
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		
		
		motors_stop();
		SleepTimer_SyncWait(32, SleepTimer_WAIT_RELOAD);
		
		/*trigger_sonar();
		
		if(distance() >= 10)
			motors_forward();
	/*
		if(distance_cm_rounded < 10)
		{	
			motors_stop();
			PWM_Servo_WritePulseWidth(SERVO_LEFT);
		}
			SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
			
			trigger_sonar();
	
			if(distance_cm_rounded < 10)
			{
				PWM_Servo_WritePulseWidth(SERVO_RIGHT);
			}
			
			else
			{
				
			}
		{	
	
		}
	
		*/

	}
}

void motors_forward(void)
{
	left_C_Data_ADDR |= left_C_MASK; 	// C=1
	left_D_Data_ADDR &= ~left_D_MASK;	// D=0
	PWM_left_WritePulseWidth(SLOW);

	right_C_Data_ADDR &= ~right_C_MASK; // C=0 
	right_D_Data_ADDR |= right_D_MASK; 	// D=1
	PWM_right_WritePulseWidth(FAST);

	PWM_left_Start();
	PWM_right_Start();
}

void motors_stop(void)
{
	left_C_Data_ADDR &= ~left_C_MASK; 	// C=0
	left_D_Data_ADDR &= ~left_D_MASK; 	// D=0

	right_C_Data_ADDR &= ~right_C_MASK; // C=0
	right_D_Data_ADDR &= ~right_D_MASK; // D=0

	PWM_left_Stop();
	PWM_right_Stop();
}

void motors_reverse(void)
{
	left_C_Data_ADDR &= ~left_C_MASK; 	// C=0
	left_D_Data_ADDR |= left_D_MASK; 	// D=1
	PWM_left_WritePulseWidth(FAST);

	right_C_Data_ADDR |= right_C_MASK; 	// C=1
	right_D_Data_ADDR &= ~right_D_MASK; // D=0
	PWM_right_WritePulseWidth(SLOW);

	PWM_left_Start();
	PWM_right_Start();
}

void motors_left(void)
{
	left_C_Data_ADDR &= ~left_C_MASK; 	// C=0
	left_D_Data_ADDR |= left_D_MASK; 	// D=1
	PWM_left_WritePulseWidth(FAST);

	right_C_Data_ADDR &= ~right_C_MASK; // C=0 
	right_D_Data_ADDR |= right_D_MASK; 	// D=1
	PWM_right_WritePulseWidth(FAST);
	
	aika=0;
	
	PWM_left_Start();
	PWM_right_Start();
	
	while(aika<=67);
	
	PWM_left_Stop();
	PWM_right_Stop();
	
	
}

void motors_right(void)
{
	left_C_Data_ADDR |= left_C_MASK; 	// C=1
	left_D_Data_ADDR &= ~left_D_MASK;	// D=0
	PWM_left_WritePulseWidth(SLOW);
	
	right_C_Data_ADDR |= right_C_MASK; 	// C=1
	right_D_Data_ADDR &= ~right_D_MASK; // D=0
	PWM_right_WritePulseWidth(SLOW);
	
	aika=0;
	
	PWM_left_Start();
	PWM_right_Start();
	
	while(aika<=67);
	
	PWM_left_Stop();
	PWM_right_Stop();
	
	
}



void echo_ISR(void)
{
	if (ECHO_INTMODE_RISING)
	{
     	EchoCounter_Start();
	}

	else
 	{
		EchoCounter_Stop();
		echo_length_us = 0xffff - EchoCounter_wReadCounter();
		EchoCounter_WritePeriod(0xffff);
	}

    // Toggle echo interrupt mode //
    Echo_IntCtrl_0_ADDR ^= Echo_MASK;
    Echo_IntCtrl_1_ADDR ^= Echo_MASK;
}

void trigger_sonar(void)
{
    int i;

    SET_TRIG;

    for (i = 3; i > 0; i--);

    CLEAR_TRIG;
}

void timer(void)
{
	aika++;
}

/*int distance(void)
{
	distance_cm = echo_length_us / 58 - SONAR_OFFSET;
	distance_cm_rounded = (distance_cm + 0.5); //distance_cm_rounded=(int)ceil(distance_cm);
	
	return distance_cm_rounded;
}

*/
