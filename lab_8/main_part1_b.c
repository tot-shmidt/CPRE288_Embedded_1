#include "my_uart.h"
#include "cyBot_Scan.h"
#include "Timer.h"
#include "scan.h"
#include "movement.h"
#include "open_interface.h"
#include <stdio.h>
#include <stdlib.h>


int main(void) {
    // Initialize Timer and UART
    timer_init();
    uart_init(115200);

    // Initialize Roomba Open Interface
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);


    // Initialize Scanner (Servo + PING + IR enabled)
    cyBOT_Scan_t scanStruct;
    int scan_init = 0b111;
    cyBOT_init_Scan(scan_init);

    // Servo calibration values
    right_calibration_value = 259000;           // Servo calibration for 0 degrees
    left_calibration_value = 1225000;           // Servo calibration for 180 degrees

    int angle_increment = 2;                    // Scan every 2 degrees
    float scan_array_US[NUM_OF_SCANS];          // Ultrasonic distance readings
    float scan_array_IR[NUM_OF_SCANS];          // IR distance readings

    // Variables for movement
    double angle_to_turn = 0;
    int dist_to_move = 0;

    // Debug indicator that CyBot is ready
    uart_sendChar('A');

    while (1) {
        char input = uart_receive();

        if (input == 'm') {
            // 1. Perform scan and save data to both arrays
            perform_scan(scan_array_US, scan_array_IR, angle_increment, &scanStruct);

            // 2. Clean the data
            clean_scanner_data(scan_array_US);
            clean_scanner_data(scan_array_IR);

            // 3. Detect objects using IR for edges and US for distance
            detect_objects(scan_array_IR, scan_array_US, &scanStruct);

            // 4. Print scan arrays and object table to PuTTY
            send_scan_to_putty(scan_array_IR, scan_array_US, angle_increment);

            // 5. Point the sensor to the object with the smallest width
            point_to_smallest_obj(&scanStruct);

            // 6. Request angle and distance from PuTTY, adjust for sensor offset, and turn
            request_angle_and_dist(&angle_to_turn, &dist_to_move);
            calculate_adjusted_angle(&angle_to_turn, &dist_to_move);
            turnToAngle(sensor_data, angle_to_turn);

            // 7. Move distance forward (swap with moveForwardWithBump if obstacles are present)
            moveForward(sensor_data, dist_to_move, 100);
        }
    }

    oi_free(sensor_data);
    return 0;
}
