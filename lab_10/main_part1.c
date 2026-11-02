#include "Timer.h"
#include "lcd.h"
#include "my_ping.h"


int main() {
    timer_init();
    lcd_init();
    ping_init();
    IntMasterEnable();  // What is this?

    while (1) {
        int time_cycles = ping_read();

        lcd_clear();
        lcd_printf("%d", time_cycles);

        timer_waitMillis(2000);
    }

    return 0;
}
