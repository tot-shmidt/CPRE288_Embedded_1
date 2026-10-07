///////////////////////////////////////////////////////////////////////
// Name: Simple-CyBot-echo.c                                         //
// Author: Phillip Jones (phjones@iastate.edu)                       //
// Date: 09/26/2023                                                  //
// Description: Simple Cybot program that is an Echo'ish program for //
//              communicating with the Starter Client Socket code.   //
///////////////////////////////////////////////////////////////////////

#include "Timer.h"
#include "lcd.h"
#include "cyBot_uart.h"

int main (void) {

  timer_init(); // Initialize Timer, needed before any LCD screen fucntions can be called 
	              // and enables time functions (e.g. timer_waitMillis)
  lcd_init();   // Initialize the the LCD screen.  This also clears the screen. 



  cyBot_uart_init_clean();  // Clean UART initialization, before running your UART GPIO init code

  // configuring the (GPIO) part of UART initialization
  SYSCTL_RCGCGPIO_R |= 0b10;          // Activating port B, as UART is on PB0, PB1
  timer_waitMillis(1);                // Small delay before accessing device after turning on clock

  GPIO_PORTB_AFSEL_R |= 0b11;         // Configure pin 0 and 1 to use Alternate Functions(p.1351) instead of data register.
  GPIO_PORTB_PCTL_R &= 0x11111100;    // Force 0's in the disired locations( WHAT DO THEY MEAN? WHY WOULD I FORCE ZEROS SOME WHERE EXCEPT pin PB0 and PB1)?
  GPIO_PORTB_PCTL_R |= 0x00000011;    // Force 1's in the disired locations( Since U1rx and U1Tx are on pb0, pb1 respectively, I care about the first 8 bits of PORTB_PCTL regiser.

  GPIO_PORTB_DEN_R |= 0b11;           // Enable digital functionality on the pins PB0 and PB1.

  GPIO_PORTB_DIR_R &= 0b11111101;     // Configure PB1 to input direction  (Force 0's in the disired locations)
  GPIO_PORTB_DIR_R |= 0b1;            // Configure PB0 to output direction (Force 1's in the disired locations)

  cyBot_uart_init_last_half();        // Completes the UART device initialization part of configuration





  char my_data;       // Variable to get bytes from Client
  char command[100];  // Buffer to store command from Client
  int index = 0;      // Index position within the command buffer

  // Write to LCD so that we know the program is running
  lcd_printf("Running");

  while(1)
  {

    index = 0;  // Set index to the beginning of the command buffer
    my_data = cyBot_getByte_blocking(); // Get first byte of the command from the Client

    // Get the rest of the command until a newline byte (i.e., '\n') received
    while(my_data != '\n' )
    {
      command[index] = my_data;  // Place byte into the command buffer
      index++;
      my_data = cyBot_getByte_blocking(); // Get the next byte of the command
    }

    command[index] = '\n';  // place newline into command in case one wants to echo the full command back to the Client
    command[index+1] = 0;   // End command C-string with a NULL byte so that functions like printf know when to stop printing

    lcd_printf("Got: %s", command);  // Print received command to the LCD screen

    // Send a response to the Client (Starter Client expects the response to end with \n)
    // In this case I am just sending back the first byte of the command received and a '\n'
    cyBot_sendByte(command[0]);

    // Only send a '\n' if the first byte of the command is not a '\n',
    // to avoid sending back-to-back '\n' to the client
    if(command[0] != '\n')
    {
      cyBot_sendByte('\n');
    }

  }
 
  return 0;
}
