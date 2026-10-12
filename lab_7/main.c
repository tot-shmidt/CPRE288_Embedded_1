/**
 * lab7_template_extra_help.c
 *
 * Description: This is file is meant for those that would like a little
 *              extra help with formatting their code.
 *
 */
#define _PART1 1
#define _PART2 0
#define _PART3 0

#include "timer.h"
#include "lcd.h"
#include "my_uart.h"
#include <string.h>


// Adding global volatile varibles for communcating between and your Interupt Service Routine, and your non-interupt code.
volatile  char uart_data;  // Your UART interupt code can place read data here
volatile  char flag;       // Your UART interupt can update this flag to indicate that it has placed new data in uart_data


void main() {
  timer_init();
  lcd_init();
  uart_init(115200);        // 115200 is baud rate.

// Receive and display text
#if _PART1
  char buffer[21];
  int index = 0;    // From 0 to 20
  char symbol;
  char debug_msg[20];

  while (1) {
      symbol = uart_receive();

      // Enter was received. Print everything you have and start over.
      if (symbol == '\r') {
          buffer[index] = '\0';
          lcd_clear();
          lcd_printf("%s", buffer);

          index = 0;

      // Regular character was received.
      } else {
          buffer[index] = symbol;

          // If I am not further then 19th character, which has index 18.
          if (index < 19) {
              // Print char to lcd and its (index + 1) and advance the buffer.
              sprintf(debug_msg, "Symbol: %c, #%d ", symbol, index + 1);
              lcd_printf(debug_msg);

              index++;
          // If I am adding 20th character to the buffer, it is time to print all together and reset.
          } else if (index == 19) {
              lcd_clear();
              buffer[index + 1] = '\0';
              lcd_printf(buffer);

              index = 0;

          }
      }
  }



#endif

// Echo Received Character
#if _PART2
    // TODO

#endif


// Receive characters using interrupts.
#if _PART3
 // TODO

#endif

}
