#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "open_interface.h"

struct  oi_t;

// Moves the robot forward for a certain distance based on centimeters parameter
void moveForward(oi_t *sensor_data, int distanceCentimeters, short wheelsSpeed);

// Performs forward movement but with obstacle(bump) detection and go around logic
void moveForwardWithBump(oi_t *sensor_data, int distanceCentimeters, short wheelsSpeed);

// Turns the robot to a certain angle without displacement.
void turnToAngle(oi_t *sensor_data, double desiredAngle);

/*  Since the sensor's center is different form the robots center, and robots turns based on its center,
 *  we need to re-adjust angle from the sensor to be the angle from the center of the robot.
 *  There is a distance from robot's to sensor's centers. */
void calculate_adjusted_angle(double* desiredAngle);

void handleBump(oi_t *sensor_data, char sensorTriggered, double *traveled);

char checkBump(oi_t *sensor_data);


#endif
