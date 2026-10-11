uart.h

#ifndef UART_H_
#define UART_H_

void uart_init(void);
void uart_sendChar(char data);
char uart_receive(void);

#endif




uart.c

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>
#include "uart.h"

void uart_init(void)
{

	UART1_LCRH_R = 0x60;

	//Need to configure PB0 and PB1 for UART
	GPIO_PORTB_AFSEL_R |= 0x03;

	//clear old function settings and assign PB0 and PB1 0001 for U1Tx and U1Rx
	GPIO_PORTB_PTCL_R &= ~0x000000FF;
	GPIO_PORTB_PTCL_R |= 0x00000011;

	//Enable PB0 and PB1
	GPIO_PORTB_DEN_R |+ 0x03;

	//Disable analog function since it is digital
	GPIO_PORTB_AMSEL_R &= ~0x03;

	// Enable UART1 (0011 0000 0001)=(0x301)
	UART1_CTL_R |=0x301;

}


//we need to receive a character
char uart_receive(void)
{
	
	//wait while the Receive FIFO is empty (0001 0000)=(0x10)
	while(UART1_FR_R & 0x10)
	{
	}
	//return the received character
	return (char)(UART1_DR_R & 0xFF);
}


//next we need to send a character
void uart_sendChar(char data)
{
	//wait while transmit FIFO is full (0010 000)=(0x20)
	while (UART_FR_R & 0x20)
	{
	}
	//send the character
	UART1_DR_R = data;
}




main.c

#include "uart.h"
#include "lcd.h"
#include "timer.h"

int main(void)
{
	timer_init();
	lcd_init();
	uart_init();
	
	//create an array for the 20 characters (21st for \0)
	char buffer[21];
	int count = 0;
	char data;

	//
	while(1)
	{
		data = uart_receive();
		
		//When enter is pressed, send '\r'
		if(data == '\r')
		{
			//end the completed string, display the buffer, reset the count
			buffer[count] = '\0';
			lcd_clear();
			lcd_printf("%s", buffer);
			
			count = 0;
		}
		//when enter isn't pressed
		else
		{
			buffer[count] = data;
			count++;

			//print to display for debugging
			lcd_printf("Count: %d\nChar: %c", count, data);
			
			// check if we've received 20 characters, if no, keep receiving, if 			//yes, print the array
			if(count == 20)
			{
				buffer[count] = '\0';
				lcd_clear();
				lcd_printf("%s", buffer);

				count = 0;
			}
		}
	}
}













