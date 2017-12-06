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

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"


///////////////
char str1[16];
char str2[16];
char str3[16];
unsigned char receiveBL[20];

#define K_P 10.00
#define K_I 0.00
#define K_D 0.00

unsigned char data = 0x00;
unsigned char ReceivePara = 0x00;


unsigned char Mstate = 0;

int16_t ScaledValue = 0;
int16_t momentValue = 0;
int16_t tempFilter = 0;
int16_t Settings = 0;
int16_t BLValue = 0;

int16_t referenceValue, measurementValue, inputValue, outValue;

/*! \brief Flags for status information
 */
struct GLOBAL_FLAGS {
	//! True when PID control loop should run one time
	uint8_t pidTimer : 1;
	uint8_t dummy : 7;
} gFlags = {0, 0};
	
struct PID_DATA pidData;

#define TIME_INTERVAL 30



/*! \brief Init of PID controller demo
 */
void Initpid(void)
{
	pid_Init(K_P * SCALING_FACTOR, K_I * SCALING_FACTOR, K_D * SCALING_FACTOR, &pidData);

}

/*! \brief Read reference value.
 *
 * This function must return the reference value.
 * May be constant or varying
 */
int16_t Get_Reference(void)
{
	return Settings;
}

/*! \brief Read system process value
 *
 * This function must return the measured data
 */
int16_t Get_Measurement(void)
{
	return ScaledValue;
}

/*! \brief Set control input to system
 *
 * Set the output from the controller as input
 * to system.
 */
void Set_Input(int16_t inputValue)
{
	if(inputValue >= 0)
	outValue = 0;
	else
	outValue = inputValue * -1;
	//
	if(outValue < 255)
	data = (outValue & 0xFF);
	else
	data = 255;
}


enum States
{
	Wait,
	Run
} state;



void Init()
{
	state = Wait;
}


void InitADC()
{
	ADCSRA|=(1<<ADEN)|(1<< ADSC)|(1<< ADATE);
}

void Set_A2D( unsigned char pinNum ) {
	ADMUX = ( pinNum <= 0x07 ) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i = 0 ; i < 15 ; i ++ ) { asm ( "nop" ); }
}

uint16_t adc_scale()
{
	uint16_t result_C;
	uint16_t result_F;
	uint16_t temp;
	//Convert to degree Centrigrade
	temp = ADC * (5000/1024);
	result_C = (temp - 465)/10;
	result_F = result_C * 9/5 + 32;
	return result_F;
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
		//Set_A2D(0x00);
		Mstate = 0x00;	
		momentValue = adc_scale();
		//ScaledValue = adc_scale();

		hc_05_bluetooth_transmit_byte('W');
		
		
		hc_05_bluetooth_receive_string(receiveBL,'a'); 
		BLValue = atoi(receiveBL);
		
		if(BLValue > 40 && BLValue < 100)
		{
			Settings = BLValue;
		}
		
		if (USART_IsSendReady(1)) {
			USART_Send(data, 1);
		}
			
		static uint16_t i = 0;

		if (i < TIME_INTERVAL) 
			{
				i++;
				tempFilter = tempFilter + momentValue;
			} 
		else 
			{
				gFlags.pidTimer = 1;
				i               = 0;
				ScaledValue = tempFilter / TIME_INTERVAL;
				tempFilter		= 0;
			}
			
		if (gFlags.pidTimer == 1) {
			referenceValue   = Get_Reference();
			measurementValue = Get_Measurement();
			inputValue = pid_Controller(referenceValue, measurementValue, &pidData);		
			Set_Input(inputValue);

			gFlags.pidTimer = FALSE;
		}
			
		itoa(referenceValue, str1, 10);
		itoa(measurementValue, str2, 10);
		//itoa(BLValue, str3, 10);
		//itoa(data, str3, 10);
		unsigned char displayString[16] = "ST:";
		strcat(displayString, str1);
		strcat(displayString, " T:");
		strcat(displayString, str2);
		//strcat(displayString, " O:");
		strcat(displayString, "  ");
		if (data > 0x00)
			{
				strcat(displayString, "ON");
			}
		else
			{
				strcat(displayString, "OFF");
			}
		//strcat(displayString, str3);
		strcat(displayString, " ");
		LCD_DisplayString(1, displayString);

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
		vTaskDelay(100);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(Task, (signed portCHAR *)"LedSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL);
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRB = 0xFF; PORTB = 0x00;

	Initpid();
	InitADC();
	initUSART(0);
	initUSART(1);
	LCD_init();
	Settings = 	75;
	
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();

	while(1)
	{
	}
	
	return 0;
}





