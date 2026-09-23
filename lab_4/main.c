#include "cyBot_uart.h"
#include "cyBot_Scan.h"
#include <stdio.h>
#include "Timer.h"
#include "sensor-data.h"
#include <math.h>

#define NUM_OF_SCANS 91
#define PI 3.1415926


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

struct tall_object objects_array[10];
int num_objects = 0;


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void point_to_smallest_obj(cyBOT_Scan_t *scanStruct) {
    if (num_objects > 0) {
        int min_width_idx = 0;

        int i;
        for (i = 1; i < num_objects; i++) {
            if (objects_array[min_width_idx].linear_width > objects_array[i].linear_width) {
                min_width_idx = i;
            }
        }

        int turn_angle = (objects_array[min_width_idx].start_angle + objects_array[min_width_idx].end_angle) / 2;

        cyBOT_Scan(turn_angle, scanStruct);
    }
}

/**
 * Helper function for detect_objects to calculate linear width of an object.
 */
float linear_width_calculator(float side_length, int angle_degrees) {
    float angle_radians = (angle_degrees * (PI / 180.0f)) / 2.0f;            // This is a half of the angle in radians. We need this to use with sine function in isosceles triangle.

    return 2 * (sinf(angle_radians) * side_length);                          // We consider a half of the isosceles triangle as a right triangle. Find oposit katet, and multiply it by 2 to get the full side.
}


/**
 * This function detects objects, their distances and widths.
 */
void detect_objects(float scan_array[]) {
    // Reset the objects_array.
    num_objects = 0;

    float next_distance;
    float limit_distance = 200;     // In cantimeters.
    float delta_limit = 15;         // In cantimeters.
    char in_object = 'f';
    char minimal_width = 2;         // In cantimeters.

    int i;
    for (i = 0; i < NUM_OF_SCANS; i++) {
        // Check if our objects_array is already full.
        if (num_objects >= 10) {
            break;
        }

        next_distance = scan_array[i];

        // Is our new distance is less than the distance limit (2 meters)?
        if (next_distance < limit_distance) {
            // Are we during object creation?
            if (in_object == 't') {
                // Isn't the next distance much bigger or much smaller then the previous one?
                if ((next_distance - scan_array[i-1] < delta_limit) && (scan_array[i-1] - next_distance < delta_limit)) {
                    // The difference is not too big, so we are still in that same object.
                    continue;
                }
                // Next distance differes to much! Have to finish the object.
                else {
                    // Finish the object creation.
                    objects_array[num_objects].end_angle = i * 2 - 2;
                    objects_array[num_objects].radial_width = objects_array[num_objects].end_angle - objects_array[num_objects].start_angle;
                    objects_array[num_objects].linear_width = linear_width_calculator(objects_array[num_objects].distance_to_obj, objects_array[num_objects].radial_width);

                    // Check if the linear_width is big enough for us so we want to save this object. If big enough - advance num_objects variable.
                    if (objects_array[num_objects].linear_width > minimal_width && num_objects < 9) {
                        num_objects++;
                    }

                    // What if the next distance differs indeed too much but it is still less than limit_distance? Should I create a new object here, as otherwise I will lost this distance reading,
                    // and I will create a new object only during next reading, creating a gap?
                    objects_array[num_objects].start_angle = i * 2;
                    objects_array[num_objects].distance_to_obj = next_distance;
                }
            }
            // We are not during object creation! Create one and start exploring it.
            else {
                // Create object based on num_objects variable as index of the array
                objects_array[num_objects].start_angle = i * 2;
                objects_array[num_objects].distance_to_obj = next_distance;

                in_object = 't';
            }
        }
        // New distance is greater than distance limit!
        else {
            // Are we during object creation?
            if (in_object == 't') {
                // Finish the object if it is a valid one
                objects_array[num_objects].end_angle = i * 2 - 2;
                objects_array[num_objects].radial_width = objects_array[num_objects].end_angle - objects_array[num_objects].start_angle;
                objects_array[num_objects].linear_width = linear_width_calculator(objects_array[num_objects].distance_to_obj, objects_array[num_objects].radial_width);

                // Check if the linear_width is big enough for us so we want to save this object. If big enough - advance num_objects variable.
                if (objects_array[num_objects].linear_width > minimal_width && num_objects < 9) {
                    num_objects++;
                }

                // We are not in object creation any more.
                in_object = 'f';
            }
            // We are not during object creation!
            else {
                // Not an object and beyond distance limit, so just continue to next iteration.
                continue;
            }
        }
    }


    // What if during last 91st iteration we are still in-object? We have to finish it creation, otherwise we would have it being not completed.
    if (in_object == 't') {
        objects_array[num_objects].end_angle = 180;
        objects_array[num_objects].radial_width = 180 - objects_array[num_objects].start_angle;
        objects_array[num_objects].linear_width = linear_width_calculator(objects_array[num_objects].distance_to_obj, objects_array[num_objects].radial_width);

        if (objects_array[num_objects].linear_width > minimal_width && num_objects < 9) {
            num_objects++;
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
    float delta_limit = 15;                             // What delta do we consider as an outlier reading

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
    char header1[] = "Degrees    Distance(cm)\r\n";
    int i = 0;

    while (header1[i] != '\0') {
        cyBot_sendByte(header1[i]);
        i++;
    }

    // 1. Print two columns data: Degree | Distance
    char destination1[50];

    for (i = 0; i < NUM_OF_SCANS; i++) {
        sprintf(destination1, "%-11d%.1f\r\n", i * angle_increment, *(scan_array + i));

        int j = 0;
        while (destination1[j] != '\0') {
            cyBot_sendByte(destination1[j]);
            j++;
        }
    }

    cyBot_sendByte('\r');
    cyBot_sendByte('\n');

    // 2. Print Table with objects data

    // Print the header of the table
    char header2[] = "Object#  Angle  Distance  Width\r\n";

    i = 0;
    while (header2[i] != '\0') {
        cyBot_sendByte(header2[i]);
        i++;
    }

    // Print data about each object
    char destination2[100];

    for (i = 0; i < num_objects; i++) {
        sprintf(destination2, "%-9d%-7d%-10.1f%-5.1f\r\n", i + 1, objects_array[i].start_angle, objects_array[i].distance_to_obj, objects_array[i].linear_width);

        int j = 0;
        while (destination2[j] != '\0') {
            cyBot_sendByte(destination2[j]);
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
            point_to_smallest_obj(&scanStruct);
        }
    }

    return 0;
}
