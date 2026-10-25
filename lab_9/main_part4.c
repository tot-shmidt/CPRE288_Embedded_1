#include "my_uart.h"
#include "cyBot_Scan.h"
#include "Timer.h"
#include "scan.h"
#include "movement.h"
#include "open_interface.h"
#include <stdio.h>
#include <stdlib.h>

void main_init(oi_t *sensor_data) {
    // Initialize Timer and UART
    timer_init();
    uart_init(115200);
    uart_interrupt_init();
    lcd_init();

    // Initialize Roomba Open Interface
    oi_init(sensor_data);

    // Initialize Scanner (Servo + PING + IR enabled)
    int scan_init = 0b111;
    cyBOT_init_Scan(scan_init);

    // Servo calibration values
    right_calibration_value = 259000;           // Servo calibration for 0 degrees
    left_calibration_value = 1225000;           // Servo calibration for 180 degrees
}


int main(void) {
    oi_t *sensor_data = oi_alloc();
    cyBOT_Scan_t scanStruct;

    main_init(sensor_data);

    int angle_increment = 2;                    // Scan every 2 degrees

    static float scan_array_US[NUM_OF_SCANS];          // Ultrasonic distance readings
    static float scan_array_IR[NUM_OF_SCANS];          // IR distance readings

    // Variables for movement
    double angle_to_turn = 0;
    int dist_to_move = 0;

    // Variables for state control
    int mode = 0;               // 0 = manual, 1 = autonomous
    int target_reached = 0;     // If 1, the cybot is within 5 cm of an object
    char command = 0;               // Should I declare this here?

    // Debug indicator that CyBot is ready
    uart_sendStr("\r\nReady:\r\n");

    while (1) {
        // 1. Check if a new character arrived. Copy it and reset the flag if yes. Also, manage the mode, if necessary.
        command = 0;    // Reset the command, so we don't go forward on a single press of w, as we can enter (mode == 0) block again.

        if (flag == 1) {
            command = uart_data;
            flag = 0;

            // If received character is t, the control mode has to be changed.
            if (command == 't') {
                mode = mode ^ 1;    // Toggle the mode.
                target_reached = 0; // Reset it, so autonomous can keep working, if needed.

                // Print message to PuTTy
                (mode) ? uart_sendStr("\r\nAutonomous mode ON.\n\r") : uart_sendStr("\r\nManual mode ON.\n\r");
            }
        }

        // 2. If manual driving mode is set:
        if (mode == 0) {
            if (command == 'w') {
                moveForward(sensor_data, 5, 100);
            } else if (command == 's') {
                moveForward(sensor_data, -5, -100);
            } else if (command == 'a') {
                turnToAngle(sensor_data, 10);
            } else if (command == 'd') {
                turnToAngle(sensor_data, -10);
            } else if (command == 'm') {
                perform_scan(scan_array_US, scan_array_IR, angle_increment, &scanStruct);
                clean_scanner_data(scan_array_US);
                clean_scanner_data(scan_array_IR);
                detect_objects(scan_array_IR, scan_array_US, &scanStruct);
                send_scan_to_putty(scan_array_IR, scan_array_US, angle_increment);
            }

        // 3. If autonomous driving mode is set:
        } else if (mode == 1 && !target_reached) {
            // Instead of keeping going autonomusly until the remaining distance is 5 cm or less, like in part 3, we
            // execute only one iteration of autonomous logic, and next iterations are controled by outer while(1) loop.

            // 0. Wait for 'h' how they want in the lab specification
            if (!wait_for_h_or_toggle()) {
                continue;   // 0 was returned, which means that 't' was received, so time to toggle.
            }

            // 1. Perform scan and save data to both arrays
            perform_scan(scan_array_US, scan_array_IR, angle_increment, &scanStruct);

            // 2. Clean the data
            clean_scanner_data(scan_array_US);
            clean_scanner_data(scan_array_IR);

            // 3. Detect objects using IR for edges and US for distance
            detect_objects(scan_array_IR, scan_array_US, &scanStruct);

            // 4. Print scan arrays and object table to PuTTY
            send_scan_to_putty(scan_array_IR, scan_array_US, angle_increment);

            // 5. Get angle and distance to smallest obj. Point the servo.
            int success = detect_smallest_obj(&scanStruct, &angle_to_turn, &dist_to_move);

            if (!success) {
                uart_sendStr("No objects found!\r\n");
                lcd_printf("No objects found!");
                continue;
            }

            // 6. Adjust angle and calculate remaining distance to the 4cm safety zone
            calculate_adjusted_angle(&angle_to_turn, &dist_to_move);

            // ~~~ Check if mode change was requested during scanning and calculation.
            if (!wait_for_h_or_toggle()) {
                continue;   // 0 was returned, which means that 't' was received, so time to toggle.
            }

            // 7. Turn, check how far we are from the 5 cm stopping zone, and move.
            turnToAngle(sensor_data, angle_to_turn);

            if (dist_to_move <= 0) {
                target_reached = 1;
            } else {
                int bumped = 0;

                if (dist_to_move > 20) {
                    bumped = moveForwardWithBump(sensor_data, 20, 100);
                } else {
                    bumped = moveForwardWithBump(sensor_data, dist_to_move, 100);
                    if (!bumped) {
                        target_reached = 1;
                    }
                }


            // 8. Do something after I reached the object's 5 cm boundary. (Print to LCD).
            if (target_reached) {
                uart_sendStr("\nArrived within 5 cm of smallest object!\r\n");
                lcd_printf("Target Reached!");
            }

        }
    }

    }
    oi_free(sensor_data);
    return 0;
}
