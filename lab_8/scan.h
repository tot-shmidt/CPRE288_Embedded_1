/*
 * This is our scanning "API" developed in lab 4.
 */
#ifndef SCAN_H_
#define SCAN_H_

#include "open_interface.h"
#include "cyBot_Scan.h"
#include "my_uart.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~ DATA STRUCTURES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Returns angle and distance to smalles object. Points the sensor servo to the smallest width object.
int detect_smallest_obj(cyBOT_Scan_t *scanStruct, double *middle_angle, int *dist_to_move);

// This function detects objects, their distances and widths.
void detect_objects(float scan_array_IR[], float scan_array_US[], cyBOT_Scan_t *scanStruct);

// Print the array from sensor_data.h in the required format to a terminal.
void display_scan_to_terminal(float scan_array_IR[], float scan_array_US[], int angle_increment);

// Clean data which we got from a call to perform_scan().
void clean_scanner_data(float scan_array[]);

// Perform ultra-sonic scans on 180 degrees range and save distances to scan_array.
void perform_scan(float scan_array_US[], float scan_array_IR[], int angle_increment, cyBOT_Scan_t* scanStruct);

// Perform IR scan at a given angle. Not 180 degrees.
float perform_single_IR_scan(int angle, cyBOT_Scan_t* scanStruct, float average_voltage);

// Sends to column data (angle - distance) and table of objects to PuTTy.
void send_scan_to_putty(float scan_array_IR[], float scan_array_US[], int angle_increment);

// Prompts the user in PuTTY for a turn angle and forward distance.
void request_angle_and_dist(double *turn_angle, int *distance);


#endif
