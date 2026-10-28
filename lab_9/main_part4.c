#include "my_uart.h"
#include "cyBot_Scan.h"
#include "Timer.h"
#include "scan.h"
#include "movement.h"
#include "open_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include "my_adc.h"
#include "lcd.h"


void main_init(oi_t *sensor_data) {
    // Initialize Timer and UART
    timer_init();
    uart_init(115200);
    uart_interrupt_init();
    lcd_init();
    adc_init();

    // Initialize Roomba Open Interface
    oi_init(sensor_data);

    // Initialize Scanner (Servo + PING + IR enabled)
    int scan_init = 0b011;
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


    // Debug indicator that CyBot is ready (ONE '\n' at the end, NO leading '\r\n'!)
    // This satisfies Python's very first cybot.readline() when the script connects.
    uart_sendStr("Ready (Manual Mode ON):\n");

    while (1) {
        command = 0;

        // 1. Check if a new character arrived
        if (flag == 1) {
            command = uart_data;
            flag = 0;

            if (command == 't') {
                mode = mode ^ 1;
                target_reached = 0;

                if (mode == 0) {
                    uart_sendStr("Manual mode ON.\r\n");
                    uart_sendStr("END\n");
                    lcd_printf("Manual Mode");
                    continue;
                } else {
                    uart_sendStr("Autonomous mode ON.\r\n");
                    lcd_printf("Autonomous Mode");
                }
            }
        }

        // 2. Manual Driving Mode (Each command sends 1 line ending in '\n' back to Python)
        if (mode == 0) {
            if (command == 'w') {
                moveForward(sensor_data, 5, 100);
                uart_sendStr("Moved forward 5 cm.\n");
            } else if (command == 's') {
                moveForward(sensor_data, -5, -100);
                uart_sendStr("Moved backward 5 cm.\n");
            } else if (command == 'a') {
                turnToAngle(sensor_data, 10);
                uart_sendStr("Turned left 10 degrees.\n");
            } else if (command == 'd') {
                turnToAngle(sensor_data, -10);
                uart_sendStr("Turned right 10 degrees.\n");
            } else if (command == 'm') {
                perform_scan(scan_array_US, scan_array_IR, angle_increment, &scanStruct);
                clean_scanner_data(scan_array_US);
                clean_scanner_data(scan_array_IR);
                detect_objects(scan_array_IR, scan_array_US, &scanStruct);
                send_scan_to_putty(scan_array_IR, scan_array_US, angle_increment); // Ends with "END\n"
            }

        // 3. Autonomous Driving Mode
        } else if (mode == 1 && !target_reached) {
            // 1. Perform scan, clean, detect, and send table to Python (ends with "END\n")
            perform_scan(scan_array_US, scan_array_IR, angle_increment, &scanStruct);
            clean_scanner_data(scan_array_US);
            clean_scanner_data(scan_array_IR);
            detect_objects(scan_array_IR, scan_array_US, &scanStruct);

            // 2. Point servo to smallest object
            int success = detect_smallest_obj(&scanStruct, &angle_to_turn, &dist_to_move);

            if (!success) {
                uart_sendStr("No objects found! Switching to Manual mode.\r\n");
                uart_sendStr("END\n");
                lcd_printf("No objects found!");
                mode = 0;
                continue;
            }

            // 3. Adjust angle and calculate remaining distance
            calculate_adjusted_angle(&angle_to_turn, &dist_to_move);

            // 4. If already within 5 cm, turn to face it and finish!
            if (dist_to_move <= 0) {
                turnToAngle(sensor_data, angle_to_turn);
                target_reached = 1;
                uart_sendStr("Arrived within 5 cm of smallest object!\r\n");
                uart_sendStr("END\n");
                lcd_printf("Target Reached!");
                continue;
            }

            // 5. Send scan table (which ends with "END\n") so Python prints the whole table
            //    and prompts the user to enter 'h' or 't'!
            send_scan_to_putty(scan_array_IR, scan_array_US, angle_increment);

            // 6. Wait for user to send 'h' (proceed) or 't' (switch back to Manual)
            if (!wait_for_h_or_toggle()) {
                continue; // 't' was pressed; top of while(1) will switch to Manual Mode
            }

            // 7. Turn and move toward the smallest object
            turnToAngle(sensor_data, angle_to_turn);

            int bumped = 0;
            if (dist_to_move > 20) {
                bumped = moveForwardWithBump(sensor_data, 20, 100);
            } else {
                bumped = moveForwardWithBump(sensor_data, dist_to_move, 100);
                if (!bumped) {
                    target_reached = 1;
                }
            }

            // 8. If we reached the target on this move, finish and tell Python!
            if (target_reached) {
                uart_sendStr("Arrived within 5 cm of smallest object!\r\n");
                uart_sendStr("END\n");
                lcd_printf("Target Reached!");
                mode = 0;
            }
            // Otherwise, the while(1) loop repeats Step 1 (re-scans and sends the new table + "END\n"!)
        }
    }
    oi_free(sensor_data);
    return 0;
}
