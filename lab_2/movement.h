#include "open_interface.h"

struct  oi_t;

// Moves the robot forward for a certain distance based on centimeters parameter
void moveForward(oi_t *sensor_data, int distanceCentimeters, short wheelsSpeed);

// Performs forward movement but with obstacle(bump) detection and go around logic
void moveForwardWithBump(oi_t *sensor_data, int distanceCentimeters, short wheelsSpeed);

// Turns the robot to a certain angle without displacement
void turnToAngle(oi_t *sensor_data, double desiredAngle);

void handleBump(oi_t *sensor_data, char sensorTriggered, double *traveled);

char checkBump(oi_t *sensor_data);
