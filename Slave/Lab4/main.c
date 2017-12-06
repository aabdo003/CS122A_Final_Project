#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>
#include <math.h>
#include "lcd.h"
#include "pid.h"
#include "hc05.h"
#include "usart_ATmega1284.h"
#include "PWM.h"

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"


///////////////
char str1[16];
char str2[16];
char str3[16];

unsigned char data = 0x00;
unsigned char ReceivePara = 0x00;

unsigned char Mstate = 0;
unsigned char CheckPWM = 0;



enum States
{
	Wait,
	Run
} state;



void Init()
{
	state = Wait;
}


void Tick()
{
	//Transitions
	switch (state)
	{
		case Wait:
		state = Run;
		break;
		case Run:
		//state = Wait;
		break;
		default:
		state = Wait;
		break;
	}
	//Actions
	switch (state)
	{
		case Wait:
		Mstate = 0x00;
		break;
		case Run:

		if (USART_HasReceived(0)) {
			data = USART_Receive(0);
			PORTC = 0x03;
			USART_Flush(0);
		}
		else
		{
			PORTC = 0x01;
		}
		

		if(data == 0x00)
		{
			PWM_off();
			CheckPWM = 0x01;
		}
		else
		{
			if(CheckPWM == 0x01) 
			{
				CheckPWM = 0x00;
				PWM_on();
				delay_ms(10);
			}
			set_PWM(data);
		}

		break;
		default:
		Mstate = 0x00;
		break;
	}
}

void Task()
{
	Init();
	for (;;)
	{
		Tick();
		vTaskDelay(10);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(Task, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL);
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00;

	initUSART(0);
	
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();

	while(1)
	{
	}
	
	return 0;
}




