#include "Timer.h"
#include "lcd.h"
#include "my_ping.h"
#include <stdio.h>


int main() {
    timer_init();
    lcd_init();
    ping_init();
    IntMasterEnable();  // What is this?

    char output_buff[60];

    int cycles_num;
    float time_ms;

    while (1) {
        int distance_cm = ping_read(&cycles_num, &time_ms);

        sprintf(output_buff, "cm: %d\nmillis: %d\ncycles: %d", distance_cm, time_ms, cycles_num);

        // Print to the LCD
        lcd_clear();
        lcd_printf("%s", output_buff);

        timer_waitMillis(2000);
    }

    return 0;
}
