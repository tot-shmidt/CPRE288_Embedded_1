#include "scan.h"
#include "sensor-data.h"
#include <math.h>
#include <stdio.h>
#include "my_uart.h"
#include <stdlib.h>
#include "cyBot_Scan.h"


// This is where we store our objects after every scan and calculation.
struct tall_object objects_array[10];
int num_objects = 0;

// I am lost here. I feel like num_objects variable is actually an index. In detect_objects, I can create an object like objects_array[0], as
// it is a valid index. But in point_to_smallest_obj I check if (num_objects > 0) but this is incorrect, as what if I have only one object, and its
// index is 0? Oh, I see. If I successfully create on object, I wll increase num_objects any way to one, right? So this is why I wrote it this way.
// Do I start from i = 1, as If I have only one object at index 0, the for loop will not execute, and min_width_idx is by default 0, that single objec?
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
void detect_objects(float scan_array_IR[], float scan_array_US[], cyBOT_Scan_t *scanStruct) {  // Does this file know about cyBOT_Scan_t ?
    // Reset the objects_array.
    num_objects = 0;

    float next_IR_distance;
    float next_US_distance;
    float limit_distance = 60;      // In cantimeters.
    float delta_limit = 10;         // In cantimeters.
    char in_object = 'f';
    char minimal_width = 2;         // In cantimeters.

    int i;
    for (i = 0; i < NUM_OF_SCANS; i++) {
        // Check if our objects_array is already full.
        if (num_objects >= 10) {
            break;
        }

        next_IR_distance = scan_array_IR[i];
        next_US_distance = scan_array_US[i];

        // Is our new distance is less than the distance limit (0.6 meters)?
        if (next_IR_distance < limit_distance) {
            // Are we during object creation?
            if (in_object == 't') {
                // Isn't the next distance much bigger or much smaller then the previous one?
                if ((next_IR_distance - scan_array_IR[i-1] < delta_limit) && (scan_array_IR[i-1] - next_IR_distance < delta_limit)) {
                    // The difference is not too big, so we are still in that same object.

                    // Try to save the smallest distance to the object. USE ULTRA-SONIC distance here.
//                    if (next_US_distance < objects_array[num_objects].distance_to_obj) {
//                        objects_array[num_objects].distance_to_obj = next_IR_distance;
//                    }

                    continue;
                }
                // Next distance differes to much! Have to finish the object.
                else {
                    // Finish the object creation.
                    objects_array[num_objects].end_angle = i * 2 - 2;
                    objects_array[num_objects].radial_width = objects_array[num_objects].end_angle - objects_array[num_objects].start_angle;

                    // New logic of finding the actual distance to object by using US sensor.
                    int mid_angle = (objects_array[num_objects].end_angle + objects_array[num_objects].start_angle) / 2;
                    objects_array[num_objects].distance_to_obj = scan_array_US[mid_angle / 2];

                    objects_array[num_objects].linear_width = linear_width_calculator(objects_array[num_objects].distance_to_obj, objects_array[num_objects].radial_width);

                    // Check if the linear_width is big enough for us so we want to save this object. If big enough - advance num_objects variable.
                    if (objects_array[num_objects].linear_width > minimal_width && num_objects < 9) {
                        num_objects++;
                    }

                    // What if the next distance differs indeed too much but it is still less than limit_distance? Should I create a new object here, as otherwise I will lost this distance reading,
                    // and I will create a new object only during next reading, creating a gap?
                    objects_array[num_objects].start_angle = i * 2;
                    //objects_array[num_objects].distance_to_obj = next_IR_distance;
                }
            }
            // We are not during object creation! Create one and start exploring it.
            else {
                // Create object based on num_objects variable as index of the array
                objects_array[num_objects].start_angle = i * 2;
                //objects_array[num_objects].distance_to_obj = next_IR_distance;

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

                // New logic of finding the actual distance to object by using US sensor.
                int mid_angle = (objects_array[num_objects].end_angle + objects_array[num_objects].start_angle) / 2;
                objects_array[num_objects].distance_to_obj = scan_array_US[mid_angle / 2];

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

        // New logic of finding the actual distance to object by using US sensor.
        int mid_angle = (objects_array[num_objects].end_angle + objects_array[num_objects].start_angle) / 2;
        objects_array[num_objects].distance_to_obj = scan_array_US[mid_angle / 2];

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
    float delta_limit = 2.0f;                       // What delta do we consider as an outlier reading
    float surface_limit = 10.0f;

    float current_distance;                         // Current value from the scan_array

    // First pass.
    int i;
    for (i = 4; i < NUM_OF_SCANS - 1; i++) {        // we process indexes from 1 to 89, as 0 and 90 don't have prev/next readings.
        current_distance = scan_array[i];

        // 1. Clean 1: Check for outlier points and normalize them by averaging it to their neighbors.
        if (((current_distance - scan_array[i+1] > delta_limit) && (current_distance - scan_array[i-1] > delta_limit)) ||
            ((scan_array[i+1] - current_distance > delta_limit) && (scan_array[i-1] - current_distance > delta_limit))) {

            current_distance = (scan_array[i-1] + scan_array[i+1]) / 2.0;
            scan_array[i] = current_distance;
        }
    }

    // Remove weired start readings.
    scan_array[0] = (scan_array[3] + scan_array[4] + scan_array[5]) / 3.0f;
    scan_array[1] = (scan_array[3] + scan_array[4] + scan_array[5]) / 3.0f;
    scan_array[2] = (scan_array[3] + scan_array[4] + scan_array[5]) / 3.0f;

    // Second pass. Smooth flat/curved object surfaces without blurring edges.
    for (i = 3; i < NUM_OF_SCANS - 1; i++) {
        float current = scan_array[i];
        float prev = scan_array[i - 1];
        float next = scan_array[i + 1];

        // Only smooth if all 3 points are already close to each other (on the same object face).
        if (fabsf(current - prev) < surface_limit && fabsf(current - next) < surface_limit) {
            scan_array[i] = (prev + current + next) / 3.0f;
        }
    }
}


/*
 * Perform ultra-sonic scans on 180 degrees range and save distances to scan_array.
 */
//void perform_scan(float scan_array_US[], float scan_array_IR[], int angle_increment, cyBOT_Scan_t* scanStruct) {
//    int i, current_angle;
//
//    cyBOT_Scan(0, scanStruct);                      // We do one scan at 0 degrees and discard the reading. This is for the sensor to settle at 0.
//    timer_waitMillis(500);                          // Wait a second before scanning so the roomba gets its rest...
//
//    float average_IR = 0;
//    for (i = 0, current_angle = 0; i < NUM_OF_SCANS; i++, current_angle += angle_increment) {
//       // Read Ultra-Sonic data scan.
//       cyBOT_Scan(current_angle, scanStruct);
//       scan_array_US[i] = scanStruct->sound_dist;      // Save measured US distance to scan_array
//       average_IR += scanStruct->IR_raw_val;           // Save first IR scan
//
//       // Read IR data scan.
//       scan_array_IR[i] = perform_single_IR_scan(current_angle, scanStruct, average_IR);
//
//       // Reset IR accumulator variable.
//       average_IR = 0;
//    }
//}
void perform_scan(float scan_array_US[], float scan_array_IR[], int angle_increment, cyBOT_Scan_t* scanStruct) {
    int i, current_angle;

    cyBOT_Scan(0, scanStruct);
    timer_waitMillis(500);

    for (i = 0, current_angle = 0; i < NUM_OF_SCANS; i++, current_angle += angle_increment) {
        // 1. Move servo to current_angle and take the first scan
        cyBOT_Scan(current_angle, scanStruct);

        float average_US = scanStruct->sound_dist;
        float average_IR = scanStruct->IR_raw_val;

        // 2. Take the remaining IR scans (and let the servo finish settling)
        scan_array_IR[i] = perform_single_IR_scan(current_angle, scanStruct, average_IR);

        // 3. Save the US distance from the first and last scans at this angle (servo is now completely still!)
        average_US += scanStruct->sound_dist;
        scan_array_US[i] = average_US / 2.0f;

        // 4. Wait 20ms so ultrasonic echoes die out before the next angle's ping
        timer_waitMillis(10);
    }
}


/*
 * Converts IR voltage to distance based on the equation of best fit.
 * Data is taken from CyBot 3
 */
float IR_volts_to_dist(float voltage) {
    if (voltage < 740.0f) {
        return 200.0f;
    }

    return 24151.0f / (voltage - 619.0f);
}


/*
 * Perform IR scan at a given anlge. Single meaning one direction, not 180 degrees cone.
 */
float perform_single_IR_scan(int angle, cyBOT_Scan_t* scanStruct, float average_voltage) {
    int precision = 2;  // Because average_voltage parameter already contains 1 value, actual precision is precision + 1.

    int i;
    for (i = 0; i < precision; i++) {
        cyBOT_Scan(angle, scanStruct);
        average_voltage += scanStruct->IR_raw_val;

        timer_waitMillis(20);    // Wait a little before every scan.
    }

    average_voltage = average_voltage / (precision + 1);

    return IR_volts_to_dist(average_voltage);
}


/**
 * Helper function for request_angle_and_dist that reads characters from PuTTY until Enter ('\r') is pressed
 * and stores them as a null-terminated string in buffer.
 */
void uart_readLine(char *buffer, int max_length) {
    int i = 0;
    char c;

    // uart_receive() in my_uart.c already echoes characters and '\r\n' to putty.
    while ((c = uart_receive()) != '\r') {
        if (i < max_length - 1) {
            buffer[i] = c;
            i++;
        }
    }

    buffer[i] = '\0'; // Null-terminate the string
}


/**
 * Prompts the user in PuTTY for a turn angle and forward distance.
 */
void request_angle_and_dist(double *turn_angle, int *distance) {
    char input_buffer[16];

    // 1. Get angle to turn
    uart_sendStr("Provide turn angle (90 is forward): ");
    uart_readLine(input_buffer, sizeof(input_buffer));
    *turn_angle = atof(input_buffer);

    // 2. Get distance to move forward
    uart_sendStr("Provide forward distance: ");
    uart_readLine(input_buffer, sizeof(input_buffer));
    *distance = atoi(input_buffer);
}


/*
 * Prints all the data to PuTTY with readings and objects table after a scan and object detection.
 */
void send_scan_to_putty(float scan_array_IR[], float scan_array_US[], int angle_increment) {
    int i;
    char buffer[100];

    // 1. Print IR Scan Data
    uart_sendStr("(IR)Degrees    Distance(cm)\r\n");

    for (i = 0; i < NUM_OF_SCANS; i++) {
        sprintf(buffer, "%-15d%.1f\r\n", i * angle_increment, scan_array_IR[i]);
        uart_sendStr(buffer);
    }
    uart_sendStr("\r\n");

    // 2. Print US Scan Data
    uart_sendStr("(US)Degrees    Distance(cm)\r\n");

    for (i = 0; i < NUM_OF_SCANS; i++) {
        sprintf(buffer, "%-15d%.1f\r\n", i * angle_increment, scan_array_US[i]);
        uart_sendStr(buffer);
    }
    uart_sendStr("\r\n");

    // 3. Print Detected Objects Table
    uart_sendStr("Object#  Angle  Distance  Width\r\n");

    for (i = 0; i < num_objects; i++) {
        int mid_angle = (objects_array[i].start_angle + objects_array[i].end_angle) / 2;
        sprintf(buffer, "%-9d%-7d%-10.1f%-5.1f\r\n",
                i + 1,
                mid_angle, // midpoint angle
                objects_array[i].distance_to_obj,
                objects_array[i].linear_width);
        uart_sendStr(buffer);
    }

    // 4. Small notice.
    uart_sendStr("\nTo get distance from bumper to object, subtract 4 from table distance");
}
