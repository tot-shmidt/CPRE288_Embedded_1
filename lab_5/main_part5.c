#include "cyBot_uart.h"
#include "cyBot_Scan.h"
#include <stdio.h>
#include "Timer.h"
#include "scan.h"
#include "movement.h"
#include "open_interface.h"

void request_angle(double *turn_angle) {
    char message[] = "Provide turn angle: ";
    int i = 0;

    // Send to putty angle request prompt
    while (message[i] != '\0') {
        cyBot_sendByte(message[i]);
        i++;
    }

    // Get the angle.
    *turn_angle = cyBot_getByte();

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
            request_angle(&angle_to_turn);
            calculate_adjusted_angle(&angle_to_turn);
            turnToAngle(sensor_data, angle_to_turn);


            // 6. Move distance
        }
    }

    oi_free(sensor_data);
    return 0;
}
