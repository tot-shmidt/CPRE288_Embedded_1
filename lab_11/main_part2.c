#include "Timer.h"
#include "servo.h"
#include "lcd.h"
#include "button.h"


extern volatile int button_event;
extern volatile int button_num;

int main() {
    timer_init();

    // Variables from servo.c
    zero_deg_match_val      = 152000;
    oneEighty_deg_match_val = 122000;
    IntMasterEnable();

    servo_init();
    lcd_init();
    button_init();
    init_button_interrupts();

    int direction = 0;                   // 0 for counter-clockwise, 1 for clockwise.
    int new_match_val = servo_move(90);  // This stores what is returned from move_servo. Inititally go to 90 degrees.
    lcd_printf("Match: %d\nDir: Counter clock wise", new_match_val);

    while (1) {
        if (button_event == 1) {
            // Reset button event variable.
            button_event = 0;

            // If button pressed 0, it is falling edge, do nothing.
            if (button_num == 0) {
                continue;

            // Move 1 degrees
            } else if (button_num == 1) {
                if (direction == 0) {
                    new_match_val = servo_move(servo_angle + 1);
                } else {
                    new_match_val = servo_move(servo_angle - 1);
                }

            // Move 5 degrees
            } else if (button_num == 2) {
                if (direction == 0) {
                    new_match_val = servo_move(servo_angle + 5);
                } else {
                    new_match_val = servo_move(servo_angle - 5);
                }

            // Togle the direction.
            } else if (button_num == 3) {
                direction = direction ^ 1;
            // Move to 0 or 180 degrees.

            } else if (button_num == 4) {
                if (direction == 0) {
                    servo_move(180);
                } else {
                    servo_move(0);
                }
            }

            // This is not the smartest approach.
            if (direction == 0) {
                lcd_printf("Match: %d\nGoing counter-clock", new_match_val);
            } else if (direction == 1) {
                lcd_printf("Match: %d\nGoing clockwise", new_match_val);
            }


        }
    }

    return 0;
}
