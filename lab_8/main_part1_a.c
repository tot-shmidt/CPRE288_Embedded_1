#include <stdio.h>
#include <stdlib.h>
#include "sensor-data.h"
#include <math.h>

#define NUM_OF_SCANS 91
#define PI 3.1415926

// This represents an object detected in detect_objects();
struct tall_object {
    int start_angle;
    int end_angle;
    int radial_width;
    float linear_width;
    float distance_to_obj;
};

struct tall_object objects_array[10];
int num_objects = 0;


void display_scan_to_terminal(float scan_array_IR[], float scan_array_US[], int angle_increment) {
    int i;
    char buffer[100];

    // 1. Print IR Scan Data
    printf("(IR)Degrees    Distance(cm)\n");

    for (i = 0; i < NUM_OF_SCANS; i++) {
        sprintf(buffer, "%-15d%.1f\n", i * angle_increment, scan_array_IR[i]);
        printf(buffer);
    }
    printf("\n");

    // 2. Print US Scan Data
    printf("(US)Degrees    Distance(cm)\n");

    for (i = 0; i < NUM_OF_SCANS; i++) {
        sprintf(buffer, "%-15d%.1f\n", i * angle_increment, scan_array_US[i]);
        printf(buffer);
    }
    printf("\n");

    // 3. Print Detected Objects Table
    printf("Object#  Angle  Distance  Width\n");

    for (i = 0; i < num_objects; i++) {
        int mid_angle = (objects_array[i].start_angle + objects_array[i].end_angle) / 2;
        sprintf(buffer, "%-9d%-7d%-10.1f%-5.1f\r\n",
                i + 1,
                mid_angle, // midpoint angle
                objects_array[i].distance_to_obj,
                objects_array[i].linear_width);
        printf(buffer);
    }

    // 4. Small notice.
    printf("\nTo get distance from bumper to object, subtract 4 from table distance\n");
}


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


float linear_width_calculator(float side_length, int angle_degrees) {
    float angle_radians = (angle_degrees * (PI / 180.0f)) / 2.0f;            // This is a half of the angle in radians. We need this to use with sine function in isosceles triangle.

    return 2 * (sinf(angle_radians) * side_length);                          // We consider a half of the isosceles triangle as a right triangle. Find oposit katet, and multiply it by 2 to get the full side.
}

void detect_objects(float scan_array_IR[], float scan_array_US[]) {
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


int main(void) {
    int angle_increment = 2;   // Scan every 2 degrees

    while (1) {
        printf("Type in something, silly: ");
        char input = getchar();

        if (input == 'm') {
            // 1. Clean the data
            clean_scanner_data(scan_array_US);
            clean_scanner_data(scan_array_IR);

            // 3. Detect objects using IR for edges and US for distance
            detect_objects(scan_array_IR, scan_array_US);

            // 4. Print scan arrays and object table to PuTTY
            display_scan_to_terminal(scan_array_IR, scan_array_US, angle_increment);
        }
    }

    return 0;
}
