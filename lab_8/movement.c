#include <movement.h>
#include "open_interface.h"
#include <math.h>
#include <stdio.h>
#include "my_uart.h"

#define PI 3.14159265358979323846

/*
 * Moves the robot forward for a certain distance based on centimeters parameter.
 */
void moveForward(oi_t *sensor_data, int distanceCentimeters, short wheelsSpeed) {
   double traveled = 0;                              // This accumulates how much the Roomba has traveled during current function call.
   oi_setWheels(wheelsSpeed, wheelsSpeed);

   // Forward movement
   if (distanceCentimeters >= 0) {
       while (traveled < distanceCentimeters * 10) {     // We convert centimeters to millimeters
           oi_update(sensor_data);
           traveled = traveled + sensor_data->distance;
       }
   }

   // Backward movement
   else if (distanceCentimeters < 0) {
      while (traveled > distanceCentimeters * 10) {     // We convert centimeters to millimeters
          oi_update(sensor_data);
          traveled = traveled + sensor_data->distance;
      }
  }

   oi_setWheels(0, 0); // stop moving, the robot is at its destination
}

/*
 * This function performs forward movement but with obstacle(bump) detection and go around logic.
 */
int moveForwardWithBump(oi_t *sensor_data, int distanceCentimeters, short wheelsSpeed) {
    double traveled = 0;        // Is this in millimeters?
    double distanceMillimeters = distanceCentimeters * 10.0;
    char bumpReading;

    oi_setWheels(150, 150);

    while (traveled < distanceMillimeters) {    // We convert centimeters to millimeters
        oi_update(sensor_data);

        // Check bump
        bumpReading = checkBump(sensor_data);

        // Handle bumps or proceed with forward direction
        if (bumpReading =='\0') {
            traveled = traveled + sensor_data->distance;
        } else {
            // Stop moving as we bumped.
            oi_setWheels(0, 0);

            if (bumpReading == 'r') {
                uart_sendStr("\r\nRight bumper bumped!\r\n");
                handleBump(sensor_data, 'r', &traveled);
            } else if (bumpReading == 'l') {
                uart_sendStr("\r\nLeft bumper bumped!\r\n");
                handleBump(sensor_data, 'l', &traveled);
            }

            return 1;
        }
    }

    // If here, a roomba could finish the whole distance without bumping anything.
    oi_setWheels(0, 0);
    return 0;
}


/*
 * Checks both bump sensors and returns a char. 'r' - right bump sensor triggered, 'l' - left sensor triggered.
 */
char checkBump(oi_t *sensor_data) {

    if (sensor_data->bumpRight) {
        return 'r';
    } else if (sensor_data->bumpLeft) {
        return 'l';
    } else {
        return '\0';
    }
}

/*
 * Bump avoidance logic
 */
void handleBump(oi_t *sensor_data, char sensorTriggered, double *traveled) {
    int backupDistance = -15;
    short backupSpeed = -100;
    double desiredAngle = 90;
    int lateralDistance = 25;
    short lateralSpeed = 150;

    // Backup for backupDistance
    moveForward(sensor_data, backupDistance, backupSpeed);

    // Turn to the correct side
    if (sensorTriggered == 'r') {
        turnToAngle(sensor_data, desiredAngle);              // perform counter-clockwise turn
    } else if (sensorTriggered == 'l') {
        turnToAngle(sensor_data, -desiredAngle);             // perform clockwise turn
    } else {
        printf("%s", "Something went crazy if we are here!");
    }

    // Move laterally
    moveForward(sensor_data, lateralDistance, lateralSpeed);

    // Turn to the initial orientation
    if (sensorTriggered == 'r') {
        turnToAngle(sensor_data, -desiredAngle);            // perform clockwise turn
    } else if (sensorTriggered == 'l') {
        turnToAngle(sensor_data, desiredAngle);             // perform counter-clockwise turn
    } else {
        printf("%s", "Something went crazy if we are here!");
    }

    // We subtract 150 millimeters from traveled distance, compensating 15cm back up movement
    *traveled = *traveled - 150;
}

/*
 * Turns the robot on certain angle in degrees.
 * Positive angle - counter-clockwise, negative angle - clockwise.
 */
void turnToAngle(oi_t *sensor_data, double desiredAngle) {
    double totalAngle = 0;           // This will accumulate the change in angle until it reaches degrees

    // Set the wheels according to the direction of the turn.
    if (desiredAngle < 0) {          // Desired angle is negative: clockwise turn
        oi_setWheels(-50, 50);       // Right wheel, left wheel
    } else if (desiredAngle > 0 ) {  // Desired angle is positive: counter-clockwise turn
        oi_setWheels(50, -50);
    } else {
        return;
    }

    // After we set the wheels, we can work with angles as absolute values.
    double desiredAngle_abs = fabs(desiredAngle);

    // Perform the turn while total_angle is less then degrees parameter. We perform calculation with absolute
    // value as at this point it does not impact the direction of our turn.
    while (fabs(totalAngle) < desiredAngle_abs) {
        oi_update(sensor_data);
        totalAngle = totalAngle + sensor_data->angle;
    }

    oi_setWheels(0, 0);              // Stop turning
}

/*
 * This function translates the angle from sensor to an actual angle that the roomba has to turn.
 * It is designed to take the angle directly from the sensor's table, it assumes the number I type is based on the sensor's coordinate system.
 */
void calculate_adjusted_angle(double *sensor_angle, int *dist_to_move) {
    int offset_dist = 13;    // Physical distance from the center of roomba to the sensor(cm).
    int bumper_radius = 17;  // 13 cm (center to sensor) + 4 cm (sensor to front bumper)
    int safety_margin = 4;   // extra 4 cm before touching the object. Does not have to match above.

    double sensor_angle_rad = *sensor_angle * (PI / 180.0);

    // Calculate X and Y coordinates of the object. Center of the roomba is at (0,0), and the sensor is at (0, 14)
    double object_x = (*dist_to_move) * cos(sensor_angle_rad);
    double object_y = offset_dist + ((*dist_to_move) * sin(sensor_angle_rad));

    double new_angle_rad = atan2(object_y, object_x);
    double new_angle_degrees = (new_angle_rad * (180.0 / PI)) - 90.0;
    *sensor_angle = new_angle_degrees;

    // Distance from Roomba center (0,0) to object, minus the Roomba's front bumper radius
    *dist_to_move = (int) (sqrt((object_x * object_x) + (object_y * object_y)) - bumper_radius - safety_margin);

    // Guard against negative distance.
    if (*dist_to_move < 0) {
        *dist_to_move = 0;
    }
}

