#include "my_ping.h"
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"
#include "Timer.h"

/*
 * Configure PB3 GPIO and TIMER3_CCP_1 (timer B), which "lives" on this pin.
 */
void ping_init() {
    // Enable clock on port b and timer 3.
    SYSCTL_RCGCGPIO_R |= 0b10;
    SYSCTL_RCGCTIMER_R |= 0b1000;
    timer_waitMillis(1);

    // Configure GPIO part
    GPIO_PORTB_DEN_R |= 0b1000;
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & ~0xF000) | 0x7000;

    // Timer configuration


}

int ping_read() {

}
