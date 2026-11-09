#include "Timer.h"
#include "my_ping.h"
#include "servo.h"

int main() {
    timer_init();
    ping_init();

    // Variables from servo.c
    zero_deg_match_val      = 152000;
    oneEighty_deg_match_val = 126000;

    servo_init();

    while (1) {
        timer_waitMillis(5000);
        servo_move(180);

        timer_waitMillis(5000);
        servo_move(0);
    }

    return 0;
}
