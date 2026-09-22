#include "cyBot_uart.h"
#include "cyBot_Scan.h"
#include <stdio.h>
#include "Timer.h"
#include "sensor-data.h"

#define NUM_OF_SCANS 91


// ~~~~~~~~~~~~~~~~~~~~~~~~~ DATA STRUCTURES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
 * This represents an object detected in detect_objects();
 */
struct tall_object {
    int start_angle;
    int end_angle;
    int radial_width;
    float linear_width;
    float distance_to_obj;
};

struct tall_object* objects_array[10];


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/**
 * This function detects objects, their distances and widths.
 */
void detect_objects(float scan_array[]) {
    float next_distance;
    float limit_distance = 2.0;
    float delta_limit = 0.15;
    char in_object = 'f';

    int i;
    for (i = 0; i < NUM_OF_SCANS; i++) {
        next_distance = scan_array[i];

        // Is our new distance is less than the distance limit (2 meters)?
        if (next_distance < limit_distance) {
            // Are we during object creation?
            if (in_object == 't') {
                // Isn't the next distance much bigger or smaller then the previous one?
                if ((next_distance - scan_array[i-1] < delta_limit) || (scan_array[i-1] - next_distance < delta_limit)) {

                }
                // Next distance differes to much!
                else {

                }
            }
            // We are not during object creation! Create one and start exploring it.
            else {
                // TO-DO: create object on the heap.
            }
        }
        // New distance is greater than distance limit!
        else {
            // Are we during object creation?
            if (in_object == 't') {
                // Finish the object if it is a valid one

            }
            // We are not during object creation!
            else {
                // Not an object and beyond distance limit, so just continue to next iteration.
                continue;
            }
        }
    }
}

/*
 * Print the array from sensor_data.h in the required format to a terminal.
 */
void display_scan_to_terminal() {
    int angle_increase = 2;

    printf("%s %s", "Angle(Degrees)", "Distance(m)\n");

    int i, angle;
    for (i = 0, angle = 0; i < NUM_OF_SCANS; i++, angle += angle_increase) {
        printf("%-14d %.2f\n", angle, sensor_data_array[i]);
    }
}

/**
 * Clean data which we got from a call to perform_scan()
 */
void clean_scanner_data(float scan_array[]) {
    float delta_limit = 0.15;                           // What delta do we consider as an outlier reading

    float current_distance;                             // Current value from the scan_array

    int i;
    for (i = 1; i < NUM_OF_SCANS - 1; i++) {            // we process indexes from 1 to 89, as 0 and 90 don't have prev/next readings.
        current_distance = scan_array[i];

        // 1. Clean 1: Check for outlier points and normalize them by averaging it to their neighbors
        if (((current_distance - scan_array[i+1] > delta_limit) && (current_distance - scan_array[i-1] > delta_limit)) ||
            ((scan_array[i+1] - current_distance > delta_limit) && (scan_array[i-1] - current_distance > delta_limit))) {

            current_distance = (scan_array[i-1] + scan_array[i+1]) / 2.0;
            scan_array[i] = current_distance;
        }
    }
}


/*
 * Perform ultra-sonic scans on 180 degrees range and save distances to scan_array.
 */
void perform_scan(float scan_array[], int angle_increment, cyBOT_Scan_t* scanStruct) {
    int i, current_angle;

    cyBOT_Scan(0, scanStruct);                      // We do one scan at 0 degrees and discard the reading. This is for the sensor to settle at 0.

    for (i = 0, current_angle = 0; i < NUM_OF_SCANS; i++, current_angle += angle_increment) {
       cyBOT_Scan(current_angle, scanStruct);       // Perform scan

       *(scan_array + i) = scanStruct->sound_dist;  // Save measured distance to scan_array
    }
}

void send_scan_to_putty(float scan_array[], int angle_increment) {
    // Print header
    char header[] = "Degrees    Distance(cm)\r\n";
    int i = 0;

    while (header[i] != '\0') {
        cyBot_sendByte(header[i]);
        i++;
    }

    // Print data
    char destination[50];

    for (i = 0; i < NUM_OF_SCANS; i++) {
        sprintf(destination, "%-11d%.1f\r\n", i * angle_increment, *(scan_array + i));

        int j = 0;
        while (destination[j] != '\0') {
            cyBot_sendByte(destination[j]);
            j++;
        }
    }
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MAIN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(void) {
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

            // 5. Point the sensor to the object with the smallest width.


        }
    }

    return 0;
}
