/*
 * This is our scanning "API" developed in lab 4.
 */

#include "open_interface.h"
#include "cyBot_Scan.h"
#include "cyBot_uart.h"


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

// Points the sensor servo to the smallest width object.
void point_to_smallest_obj(cyBOT_Scan_t *scanStruct);

// This function detects objects, their distances and widths.
void detect_objects(float scan_array[]);

// Print the array from sensor_data.h in the required format to a terminal.
void display_scan_to_terminal();

// Clean data which we got from a call to perform_scan().
void clean_scanner_data(float scan_array[]);


// Perform ultra-sonic scans on 180 degrees range and save distances to scan_array.
void perform_scan(float scan_array[], int angle_increment, cyBOT_Scan_t* scanStruct);

// Sends to column data (angle - distance) and table of objects to PuTTy.
void send_scan_to_putty(float scan_array[], int angle_increment);
