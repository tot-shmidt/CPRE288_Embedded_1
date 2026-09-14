#include "open_interface.h"

struct  oi_t;


// Moves the robot forward for a certain distance based on centimeters parameter
void moveForward(oi_t *sensor, int centimeters);

// Turns the robot to a certain angle without displacement
void turnToAngle(oi_t *sensor, int degrees);
