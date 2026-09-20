#include "cyBot_uart.h"
#include "cyBot_Scan.h"
#include <stdio.h>
#include "Timer.h"

void perform_scan(float scan_array[], int angle_increment, cyBOT_Scan_t* scanStruct) {
    int num_of_scans = (180 / angle_increment) + 1;   // We will have 90 scans total.

    int i, current_angle;

    for (i = 0, current_angle = 0; i < num_of_scans; i++, current_angle += angle_increment) {
       cyBOT_Scan(current_angle, scanStruct);     // Perform scan

       *(scan_array + i) = scanStruct->sound_dist;  // Save measured distance to scan_array
    }
}

void send_scan_to_putty(float scan_array[], int angle_increment) {
    int num_of_scans = (180 / angle_increment) + 1;

    // Print header
    char header[] = "Degrees    Distance(cm)\r\n";
    int i = 0;

    while (header[i] != '\0') {
        cyBot_sendByte(header[i]);
        i++;
    }

    // Print data
    char destination[50];

    for (i = 0; i < num_of_scans; i++) {
        sprintf(destination, "%-11d%.1f\r\n", i * angle_increment, *(scan_array + i));

        int j = 0;
        while (destination[j] != '\0') {
            cyBot_sendByte(destination[j]);
            j++;
        }
    }
}

int main(void) {
    timer_init();
    cyBot_uart_init();

    cyBOT_Scan_t scanStruct;                    // Struct for cyBot_Scan function.
    int scan_init = 0b111;                      // Do I need to enable IR?
    cyBOT_init_Scan(scan_init);                 // Initialize features of the scanner.

    int angle_increment = 2;                    // How often do we scan. Every 2 degrees in this case.
    float scan_array[91];    // Array where we will store readings from scanStruct.

    // Debug
    cyBot_sendByte('A');

    while (1) {
        char input = cyBot_getByte();

        if (input == 'm') {
            perform_scan(scan_array, angle_increment, &scanStruct);
            send_scan_to_putty(scan_array, angle_increment);
        }
    }



    return 0;
}
