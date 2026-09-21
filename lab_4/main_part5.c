#include "sensor-data.h"
#include <stdio.h>

int main(void) {
    int sensor_arr_length = 91;
    int angle_increase = 2;

    // 1. Print the log file itself in the required format
    printf("%s %s", "Angle(Degrees)", "Distance(m)\n");

    int i, angle;
    for (i = 0, angle = 0; i < sensor_arr_length; i++, angle += angle_increase) {
        printf("%-14d %.2f\n", angle, sensor_data_array[i]);
    }

    return 0;
}
