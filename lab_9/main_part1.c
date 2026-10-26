#include "Timer.h"
#include "lcd.h"
#include "cyBot_Scan.h"
#include "open_interface.h"
#include "my_uart.h"
#include "my_adc.h"
#include <stdio.h>


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

    // 2. Request to scan
    uart_sendStr("\r\nADC device is ready to produce values:\r\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\r\n\r\n");

    while (1) {
        uart_sendStr("Press [s] to scan: ");

        char input = uart_receive();

        if (input == 's') {
            uart_sendStr("\r\n");

            float voltage = adc_read();

            sprintf(buffer, "Read: %.1f", voltage);

            lcd_printf(buffer);

            uart_sendStr(buffer);
            uart_sendStr("\r\n\r\n");

            timer_waitMillis(300);
        }
    }


    return 0;
}
