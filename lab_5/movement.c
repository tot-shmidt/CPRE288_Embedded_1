#include <movement.h>
#include "open_interface.h"

/*
 * Moves the robot forward for a certain distance based on centimeters parameter.
 */
void moveForward(oi_t *sensor_data, int centimeters) {
   double sum = 0;
   oi_setWheels(300, 300);

   while (sum < centimeters * 10) {         // We convert centimeters to millimeters
       oi_update(sensor_data);
       sum = sum + sensor_data->distance;
   }

   oi_setWheels(0, 0); // stop
}

/*
 * Turns the robot on certain angle in degrees.
 * Positive angle - counter-clockwise, negative angle - clockwise.
 */
void turnToAngle(oi_t *sensor, int degrees) {

}
