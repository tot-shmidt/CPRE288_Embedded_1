#include "open_interface.h"
#include "movement.h"
#include "timer.h"
#include "cyBot_uart.h"

void main() {
     oi_t *sensor_data = oi_alloc();
     oi_init(sensor_data);


     /* THIS IS PART 3 OF LAB 2:

     int distance = 50;          // How far to go in centimeters
     double turnAngle = -90.0;   // How much to turn. Negative value will turn clockwise.
     short speed = 150;

     int i = 0;

     while (i < 4) {
         moveForward(sensor_data, distance, speed);
         timer_waitMillis(1000);

         turnToAngle(sensor_data, turnAngle);
         timer_waitMillis(1000);

         i++;
     }
     */

     /* THIS IS PART 4 OF LAB 2: */

//     int distance = 200;                                        // Move 2 meters forward
//     short wheelsSpeed = 150;
//     moveForwardWithBump(sensor_data, distance, wheelsSpeed);
//
//     oi_free(sensor_data);

     /* THIS IS PART 5 OF LAB 2: */
     cyBot_uart_init();
     lcd_init();

     char input;
     char message[] = "Got an m\n";

     while (1) {
        input = cyBot_getByte();

        lcd_printf("\n          %c\n", input);

        if (input == 'w') {
            moveForward(sensor_data, 5, 250);
        } else if (input == 's') {
            moveForward(sensor_data, -5, -250);
        } else if (input == 'a') {
            turnToAngle(sensor_data, 10);
        } else if (input == 'd') {
            turnToAngle(sensor_data, -10);
        } else if (input == 'm') {
            char i = 0;

            while (message[i] != '\0') {
                cyBot_sendByte(*(message + i));
                i++;
            }
        }


     }

     oi_free(sensor_data);
}
