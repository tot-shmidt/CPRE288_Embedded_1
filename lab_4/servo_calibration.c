#include "CyBot_Scan.h"
#include "cyBot_uart.h"
#include "Timer.h"
#include "lcd.h"

int main(void) {
    // bot 12: 0 degrees: 243250 ; degrees 180: 1209250
    timer_init();
    lcd_init();
    cyBOT_init_Scan(0b111);
    //cyBOT_SERVO_cal();

    right_calibration_value = 243250;
    left_calibration_value = 1209250;

    int i;
    cyBOT_Scan_t scan;

    for (i=0; i <= 180; i++) {
        cyBOT_Scan(i, &scan);
    }

    return 0;
}
