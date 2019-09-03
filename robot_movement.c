/*****************************************************************************
 * - Source code for robot
 * - This code is mainly for bowling competition
 * - File: main.c
 * - PSoC Designer 5.1.1875.0
 ****************************************************************************/

/*
C(10): vihreä johto
D(12): keltainen johto
*/

#include <stdlib.h>		// itoa(), utoa() etc.
#include <m8c.h>		// part specific constants and macros
#include "PSoCAPI.h"	// PSoC API definitions for all User Modules
#include "Timer8.h"

// Sonar //
#define MIN_WALL_DISTANCE 45
#define SONAR_OFFSET 8	// Sonar distance back from measuring point (cm)

// Interrupt Control Register bits
// IntCtrl_0 = 0, IntCtrl_1 = 1 -> Rising Edge
// IntCtrl_0 = 1, IntCtrl_1 = 0 -> Falling Edge
#define ECHO_INTMODE_RISING 	Echo_IntCtrl_1_ADDR & Echo_MASK
#define SET_TRIG                Trig_Data_ADDR |= Trig_MASK
#define CLEAR_TRIG              Trig_Data_ADDR &= ~Trig_MASK

/* Mahdollinen GYRO
#define SET_SPIM_MODE			spim_mode_Data_ADDR &= ~spim_mode_MASK // Sets pin to 0 to activate SPIM mode on gyro
*/

// Servo //
#define SERVO_CENTER 16
#define SERVO_LEFT 5
#define SERVO_RIGHT 25

// Motors //
#define FAST 159 // 100 %	| Nämä tehty sitä varten, että sähkömoottorit
#define SLOW 160 // ? %		| pyörivät taaksepäin hitaammin.
#define FINISH_AREA 140

#define TURNING_TIME_LEFT 174
#define TURNING_TIME_RIGHT 167

// Interrupt handlers //
#pragma interrupt_handler echo_ISR
//#pragma interrupt_handler echo_counter_ISR
#pragma interrupt_handler timer_ISR // Tarkan ajastuksen timeri

// Function prototypes //
void motors_forward(int left_speed, int right_speed);
void motors_stop(void);
void motors_reverse(void);

void motors_turn_left(void);
void motors_turn_right(void);

void servo_center(void);
void servo_turn_left(void);
void servo_turn_right(void);

void echo_ISR(void);
//void echo_counter_ISR(void);
void trigger_sonar(void);

void timer_ISR(void);

int get_distance(void);

void delay(int second100);

// Global variables //
volatile int time_turning = 0;
volatile unsigned int echo_length_us = 0;


void main(void)
{
	// Variables //
	int distance = 0;
	int distance_left = 0;
	int distance_right = 0;
	int flag = 0;
	
	char str[12] = " ";
	char print[10], print2[10];
	
	// Enable interrupts //
    //M8C_EnableGInt;
    //M8C_EnableIntMask(INT_MSK0 , INT_MSK0_GPIO);
	M8C_EnableGInt ;                            // Turn on interrupts 
	SleepTimer_Start();
	SleepTimer_SetInterval(SleepTimer_64_HZ);   // Set interrupt to a
	SleepTimer_EnableInt();
	
	/*
	// Timer //
	Timer8_EnableInt();
    Timer8_Start();
	
	// PWM //
	PWM_left_DisableInt();
	PWM_left_Start();
	
	PWM_right_DisableInt();
	PWM_right_Start();*/
	
	//PWM_Servo_DisableInt();
	PWM_Servo_Start();
	
	// LCD //
	//LCD_Start();

	
	while (1)
	{	
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);

		PWM_Servo_WritePulseWidth(SERVO_RIGHT);
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		SleepTimer_SyncWait(64, SleepTimer_WAIT_RELOAD);
		PWM_Servo_WritePulseWidth(SERVO_CENTER);

				
		
		/*while(1)
				{
				trigger_sonar();
				
				delay(7);
				
				distance = get_distance();
				
				LCD_Position(0,0);
				LCD_PrCString("        ");
				itoa(str, distance, 10);
				LCD_Position(0,0);
				LCD_PrString(str);
				}
				
			servo_center();

			trigger_sonar();
			
			delay(7);
			
			distance = get_distance();

			motors_forward(SLOW,FAST);
			
			while(distance > MIN_WALL_DISTANCE)
			{
				while(1)
				{
				trigger_sonar();
				
				delay(7);
				
				distance = get_distance();
				
				LCD_Position(0,0);
				LCD_PrCString("        ");
				itoa(str, distance, 10);
				LCD_Position(0,0);
				LCD_PrString(str);
				}
			}
				
			motors_stop();
		
			servo_turn_left();
			
			trigger_sonar();
			
			delay(7);

			distance_left = get_distance();
			
			
			
			servo_turn_right();
			
			trigger_sonar();
			
			delay(7);
			
			distance_right = get_distance();
			
			LCD_Position(1,0);
			LCD_PrCString("        ");
			itoa(str, distance_right, 10);
			LCD_Position(1,0);
			LCD_PrString(str);
				
			if (distance_right > distance_left)
			{
				motors_turn_right();
				motors_stop();
			}
			
			else if (distance_right < distance_left)
			{
				motors_turn_left();
				motors_stop();
			}
			
			else
			{
				motors_reverse();
			}*/
	}
}

void motors_forward(int left_speed, int right_speed)
{
	left_C_Data_ADDR |= left_C_MASK; 	// C=1
	left_D_Data_ADDR &= ~left_D_MASK;	// D=0
	PWM_left_WritePulseWidth(left_speed);

	right_C_Data_ADDR &= ~right_C_MASK; // C=0 
	right_D_Data_ADDR |= right_D_MASK; 	// D=1
	PWM_right_WritePulseWidth(right_speed);

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
	
	// Viive ettei tuhota moottoreita
	delay(50);
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

void motors_turn_right(void)
{
	left_C_Data_ADDR &= ~left_C_MASK; 	// C=0
	left_D_Data_ADDR |= left_D_MASK; 	// D=1
	PWM_left_WritePulseWidth(FAST);

	right_C_Data_ADDR &= ~right_C_MASK; // C=0 
	right_D_Data_ADDR |= right_D_MASK; 	// D=1
	PWM_right_WritePulseWidth(FAST);
	
	time_turning = 0;
	
	Timer8_Start();
	
	PWM_left_Start();
	PWM_right_Start();
	
	while (time_turning < TURNING_TIME_RIGHT);
	
	Timer8_Stop();
	
	PWM_left_Stop();
	PWM_right_Stop();
}

void motors_turn_left(void)
{
	
	left_C_Data_ADDR |= left_C_MASK; 	// C=1
	left_D_Data_ADDR &= ~left_D_MASK;	// D=0
	PWM_left_WritePulseWidth(SLOW);
	
	right_C_Data_ADDR |= right_C_MASK; 	// C=1
	right_D_Data_ADDR &= ~right_D_MASK; // D=0
	PWM_right_WritePulseWidth(SLOW);
	
	time_turning = 0;
	
	Timer8_Start();
	
	PWM_left_Start();
	PWM_right_Start();
	
	while (time_turning < TURNING_TIME_LEFT);
	
	Timer8_Stop();
	
	PWM_left_Stop();
	PWM_right_Stop();
}

void servo_center(void)
{
	PWM_Servo_WritePulseWidth(SERVO_CENTER);
	
	// Viive 2 sec ettei heti räiskitä seinille kaikua
	delay(200);
}

void servo_turn_left(void)
{
	PWM_Servo_WritePulseWidth(SERVO_LEFT);
				
	// Viive 2 sec ettei heti räiskitä seinille kaikua
	delay(200);
}

void servo_turn_right(void)
{
	PWM_Servo_WritePulseWidth(SERVO_RIGHT);

	// Viive 2 sec ettei heti räiskitä seinille kaikua
	delay(200);
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

/*void echo_counter_ISR(void)
{
	EchoCounter_Stop();
	echo_length_us = 40000;
	EchoCounter_WritePeriod(40000);
	
	// Toggle echo interrupt mode //
    Echo_IntCtrl_0_ADDR ^= Echo_MASK;
    Echo_IntCtrl_1_ADDR ^= Echo_MASK;
}*/

void delay(int second100)
{
	time_turning = 0;
	
	Timer8_Start();
	
	while (time_turning < second100);
	
	Timer8_Stop();
}

void trigger_sonar(void)
{
    int i;

    SET_TRIG;

    for (i = 3; i > 0; i--);

    CLEAR_TRIG;
}

void timer_ISR(void)
{
	time_turning++;
}

int get_distance(void)
{
	int distance_cm = 0;

	distance_cm = echo_length_us / 58 - SONAR_OFFSET;

	return distance_cm;
}

