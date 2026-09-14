/// Simple 'Hello, world' program
/**
 * This program prints "Hello, world" to the LCD screen
 * @author Chad Nelson
 * @date 06/26/2012
 *
 * updated: phjones 9/3/2019
 * Describtion: Added timer_init call, and including Timer.h
 */

#include "Timer.h"
#include "lcd.h"
#include <string.h>


int main (void) {
    /******************************************
     *   Inititalization and message choice   *
     ******************************************/
	timer_init();                                           // Initialize Timer, needed before any LCD screen fucntions can be called and enables time functions (e.g. timer_waitMillis)
	lcd_init();                                             // Initialize the the LCD screen.  This also clears the screen.
	char message[] = "I love shrimp!";   // Choose your output string


	/**************************************************************************************************
	 * Create white spaced message string with the screen width(20 cells) of whitespaces on both ends  *
	 **************************************************************************************************/
	size_t messageLength = sizeof(message) - 1;
	char whitespacedMessage[41 + sizeof(message) - 1];     // Allocate memory for the output string

	int i;
	for (i = 0; i < 41 + messageLength; i++)         // Set all elements of whitespacedMessage to white space
	    whitespacedMessage[i] = ' ';

	whitespacedMessage[40 + messageLength] = '\0';   // Set last element to be zero termination byte

	memcpy(&whitespacedMessage[20], &message[0], messageLength);    // Copy message in the middle of allocated memory. (pointer to destination memory, pointer to source memory, number of bytes to copy size_t)

	/***************************************************************************************************
	 * Each while loop will constitute full rotation of the banner from appearance to disappearance.
	 * In for loop I will grab 20 bytes each time, and our white space offset from the left will allow
	 * first character to appear on the right side of the LCD monitor.
	 **************************************************************************************************/
	while (1) {
	    char *start = whitespacedMessage;   // The pointer starts at the first whitespace of whitespacedMessage

	    // Allocate memory for the output string which will be sent to printf. Set last element to '\0'
	    char outputMessage[21];

	    outputMessage[20] = '\0';

	    // 'MessageLength + 20' is required number of shifts to happen so the message can appear with 1 character and then fully disappear
	    for (i = 0; i < messageLength + 20; i++) {
	        memcpy(outputMessage, &start[i], 20);
	        lcd_printf(outputMessage);

	        timer_waitMillis(300);
	    }
	}

	return 0;
}
