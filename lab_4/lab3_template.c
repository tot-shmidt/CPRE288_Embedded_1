/**
 * lab3_template.c
 * 
 * Template file for CprE 288 lab 3
 *
 * @author Zhao Zhang, Chad Nelson, Zachary Glanz
 * @date 08/14/2016
 */

#include "button.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_uart.h"  // Functions for communiticate between CyBot and Putty (via UART)
                         // PuTTy: Buad=115200, 8 data bits, No Flow Control, No Party,  COM1

#define REPLACEME 0



int main(void) {
	button_init();
	lcd_init();

/*****************************************************
 * THIS IS PART 2 of Lab 3
 *****************************************************
//    // Don't forget to initialze the cyBot UART before trying to use it.
 *
 */
	// YOUR CODE HERE
//	uint8_t buttonPressed = 0;
//
//	while(1) {
//	    buttonPressed = button_getButton();
//      lcd_printf("\n  Button pressed: %d ", buttonPressed);
//	}
//
//	return 0;
/*
 * *************************************************
 */

/*
 * THIS IS PART 4 of Lab 3
 */
	cyBot_uart_init();

	uint8_t buttonPressed = 0;
	char byteSend;

	while (1) {
	    buttonPressed = button_getButton();
	    lcd_printf("\n  Button pressed: %d ", buttonPressed);

	    if (buttonPressed != 0) {
	        if (buttonPressed == 1) {
	            byteSend = '1';
	        } else if (buttonPressed == 2) {
	            byteSend = '2';
	        } else if (buttonPressed == 3) {
	            byteSend = '3';
	        } else if (buttonPressed == 4) {
	            byteSend = '4';
	        }

	        cyBot_sendByte(byteSend);
	    }
	}
}
