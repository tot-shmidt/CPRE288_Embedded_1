/**
 * lab6_template.c
 * 
 * Template file for CprE 288 lab 6
 *
 * @author Zhao Zhang, Chad Nelson, Zachary Glanz
 * @date 08/14/2016
 *
 * @author Phillip Jones, updated 6/4/2019
 */

#include "button.h"
#include "timer.h"
#include "lcd.h"
#include <string.h>

#include "cyBot_uart.h"  // Functions for communiticate between CyBot and Putty (via UART)
                         // PuTTy: Buad=115200, 8 data bits, No Flow Control, No Party,  COM1

#include "cyBot_Scan.h"  // For scan sensors 

int main(void) {
	button_init();
	init_button_interrupts();
	lcd_init();
	
    cyBot_uart_init_clean();  // Clean UART initialization, before running your UART GPIO init code

	// configuring the (GPIO) part of UART initialization
    SYSCTL_RCGCGPIO_R |= 0b10;          // Activating port B, as UART is on PB0, PB1
    timer_waitMillis(1);                // Small delay before accessing device after turning on clock

    GPIO_PORTB_AFSEL_R |= 0b11;         // Configure pin 0 and 1 to use Alternate Functions(p.1351) instead of data register.
    GPIO_PORTB_PCTL_R &= 0xFFFFFF00;    // Force 0's in the disired locations( WHAT DO THEY MEAN? WHY WOULD I FORCE ZEROS SOME WHERE EXCEPT pin PB0 and PB1)?
    GPIO_PORTB_PCTL_R |= 0x00000011;    // Force 1's in the disired locations( Since U1rx and U1Tx are on pb0, pb1 respectively, I care about the first 8 bits of PORTB_PCTL regiser.

    GPIO_PORTB_DEN_R |= 0b11;           // Enable digital functionality on the pins PB0 and PB1.

    GPIO_PORTB_DIR_R &= 0b11111101;     // Configure PB1 to input direction  (Force 0's in the disired locations)
    GPIO_PORTB_DIR_R |= 0b1;            // Configure PB0 to output direction (Force 1's in the disired locations)
    
    cyBot_uart_init_last_half();        // Completes the UART device initialization part of configuration
	
	//Initialze scan sensors
    cyBOT_Scan_t scanStruct;                    // Struct for cyBot_Scan function.
    int scan_init = 0b111;
    cyBOT_init_Scan(scan_init);


	// YOUR CODE HERE
    char input_from_pc;
    char dest[50];
    int i = 1;
    int j = 0;

	while(1) {
	    input_from_pc = cyBot_getByte_blocking();

	    if (input_from_pc == 's') {
	        cyBOT_Scan(100, &scanStruct);

	        sprintf(dest, "%d: %d\r\n", i, scanStruct.IR_raw_val);

	        while (dest[j] != '\0') {
	            cyBot_sendByte(dest[j]);
	            j++;
	        }

	        memset(dest, 0, 50);

	        i++;
	        j = 0;
	    }
	}
	
	return 0;

}
