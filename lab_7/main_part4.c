#include "my_uart.h"
#include "cyBot_Scan.h"
#include <stdio.h>
#include "Timer.h"
#include "scan.h"


int main(void) {
    timer_init();
    uart_init(115200);

    cyBOT_Scan_t scanStruct;                    // Struct for cyBot_Scan function.
    int scan_init = 0b111;                      // Do I need to enable IR?
    cyBOT_init_Scan(scan_init);                 // Initialize features of the scanner.

    right_calibration_value = 227500;           // Servo calibration for 0 degrees
    left_calibration_value = 1230250;           // Servo calibration for 180 degrees

    int angle_increment = 2;                    // How often do we scan. Every 2 degrees in this case.

    float scan_array_US[91];                    // Array where we will store readings from scanStruct.
    float scan_array_IR[91];                    // Array for infra-red sensor readings.

    // Debug
    uart_sendChar('A');

    while (1) {
        char input = uart_receive();

        if (input == 'm') {
            // 1. Perform scan and save data to scan_array.
            perform_scan(scan_array_US, scan_array_IR, angle_increment, &scanStruct);

            // 2. Clean the data.
            clean_scanner_data(scan_array_US);
            clean_scanner_data(scan_array_IR);

            // 3. Detect objects from the data and calculate their radial and liner width
            detect_objects(scan_array_IR, scan_array_US);

            // 4. Print whole array and object structs to the terminal/putty, depending on the phase of development
            send_scan_to_putty(scan_array_IR, scan_array_US, angle_increment);

            // 5. Point the sensor to the object with the smallest width.
            point_to_smallest_obj(&scanStruct);
        }
    }

    return 0;
}
