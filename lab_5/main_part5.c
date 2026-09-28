#include "cyBot_uart.h"
#include "cyBot_Scan.h"
#include <stdio.h>
#include "Timer.h"
#include "scan.h"
#include "movement.h"
#include "open_interface.h"
#include <stdlib.h>


void request_angle_and_dist(double *turn_angle, int *distance) {
    char message[] = "Provide turn angle: ";
    int i = 0;

    // 1. Get angle to turn
    while (message[i] != '\0') {
        cyBot_sendByte(message[i]);
        i++;
    }

    // Start parsing PyTTy's characters to get an integer
    char crazy_angle_array[5];      // Crazy because I need to parse this String to get a number because Putty
                                    // can't read 67 as one byte, so...
    char crazy_digit;

    // Form a string from which I will create an angle double.
    i = 0;
    while ((crazy_digit = cyBot_getByte()) != '\n') {
        crazy_angle_array[i] = crazy_digit;
        i++;
    }

    crazy_angle_array[i] = '\0';

    // Conver our digit string to a double and put into dereferenced pointer.
    *turn_angle = atof(crazy_angle_array);

    cyBot_sendByte('\r');
    cyBot_sendByte('\n');

    // 2. Get distance to move forward.
    char message1[] = "Provide forward distance: ";
    i = 0;

    while (message1[i] != '\0') {
        cyBot_sendByte(message[i]);
        i++;
    }

    // Form a distance string from which I will create a distance integer.
    char crazy_distance_array[5];

    i = 0;
    while ((crazy_digit = cyBot_getByte()) != '\n') {
        crazy_distance_array[i] = crazy_digit;
        i++;
    }

    crazy_distance_array[i] = '\0';

    *distance = atoi(crazy_distance_array);

    cyBot_sendByte('\r');
    cyBot_sendByte('\n');
}

int main(void) {
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    timer_init();
    cyBot_uart_init();

    cyBOT_Scan_t scanStruct;                    // Struct for cyBot_Scan function.
    int scan_init = 0b111;                      // Do I need to enable IR?
    cyBOT_init_Scan(scan_init);                 // Initialize features of the scanner.

    right_calibration_value = 227500;           // Servo calibration for 0 degrees
    left_calibration_value = 1230250;           // Servo calibration for 180 degrees

    int angle_increment = 2;                    // How often do we scan. Every 2 degrees in this case.
    float scan_array[91];                       // Array where we will store readings from scanStruct.

    // Debug
    cyBot_sendByte('A');

    // Variables for movement
    double angle_to_turn = 0;
    int dist_to_move = 0;

    while (1) {
        char input = cyBot_getByte();

        if (input == 'm') {
            // 1. Perform scan and save data to scan_array.
            perform_scan(scan_array, angle_increment, &scanStruct);

            // 2. Clean the data.
            clean_scanner_data(scan_array);

            // 3. Detect objects from the data and calculate their radial and liner width
            detect_objects(scan_array);

            // 4. Print whole array and object structs to the terminal/putty, depending on the phase of development
            send_scan_to_putty(scan_array, angle_increment);

            // 5. Turn to angle, providing input in degrees
            request_angle_and_dist(&angle_to_turn, &dist_to_move);
            calculate_adjusted_angle(&angle_to_turn);
            turnToAngle(sensor_data, angle_to_turn);

            // 6. Move distance forward
            moveForward(sensor_data, dist_to_move, 150);
        }
    }

    oi_free(sensor_data);
    return 0;
}
