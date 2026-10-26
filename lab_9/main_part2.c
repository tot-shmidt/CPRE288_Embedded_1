#include "Timer.h"
#include "lcd.h"
#include "cyBot_Scan.h"
#include "open_interface.h"
#include "my_uart.h"
#include "my_adc.h"
#include <stdio.h>
#include "scan.h"

int main(void) {
    // 1. Initialization
    timer_init();
    uart_init(115200);
    lcd_init();
    adc_init();

    //oi_t *sensor_data = oi_alloc();
    //oi_init(sensor_data);

    int scan_init = 0b011;  // disable IR library
    cyBOT_init_Scan(scan_init);

    char buffer[25];
    char buffer2[25];

    // 2. Request to scan
    uart_sendStr("\r\nADC device is ready to produce values:\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n\r\n");

    while (1) {
        uart_sendStr("Press [s] to scan: ");

        char input = uart_receive();

        if (input == 's') {
            uart_sendStr("\r\n");

            float voltage = adc_read();
            float distance = IR_volts_to_dist(voltage);

            // 1. Print both lines cleanly to the LCD
            lcd_printf("Read: %.0f\nDist: %.1f cm", voltage, distance);

            sprintf(buffer, "Read: %.0f\r\n", voltage);
            sprintf(buffer2, "Distance: %.1f cm\r\n\r\n", distance);

            uart_sendStr(buffer);
            uart_sendStr(buffer2);

            timer_waitMillis(300);
        }
    }


    return 0;
}
