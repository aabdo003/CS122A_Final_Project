#ifndef _HC05_H_
#define _HC05_H_  1

#include<avr/io.h> 
#include<util/delay.h>
#include "usart_ATmega1284.h"
#define delay_ms               	              _delay_ms
char hc_05_buffer1[25], hc_05_buffer2[50];
char temp;

void hc_05_bluetooth_transmit_byte(unsigned char data_byte);
unsigned char hc_05_bluetooth_receive_byte(void);
void hc_05_bluetooth_transmit_string(unsigned char *transmit_string);
void hc_05_bluetooth_receive_string(unsigned char *receive_string, unsigned char terminating_character);

void hc_05_bluetooth_transmit_byte(unsigned char data_byte)
{
	if (USART_IsSendReady(0)) {
		USART_Send(data_byte, 0);
	}
}
unsigned char  hc_05_bluetooth_receive_byte(void)
{
	unsigned char data = 0x00;
	if (USART_HasReceived(0)) {
		data = USART_Receive(0);
		USART_Flush(0);
	}	
 return data;
}
void hc_05_bluetooth_transmit_string(unsigned char *transmit_string)
{
	while(*transmit_string)
	{
		USART_Send(*transmit_string++,0);
	}
}
void hc_05_bluetooth_receive_string(unsigned char  *receive_string, unsigned char terminating_character)
{
	unsigned char temp= 0x00;
		for(unsigned char i=0;i<20;i++)
		{
			if (USART_HasReceived(0)) 
			{
				*(receive_string+i)=USART_Receive(0);
				if (*(receive_string+i)==terminating_character)
				{
					USART_Flush(0);
					break;
				}
				else
				{
					temp++;
				}
			}
		}
		*(receive_string+temp)='\0';
}
#endif