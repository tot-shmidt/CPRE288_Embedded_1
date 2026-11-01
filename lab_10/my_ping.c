#include "my_ping.h"
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"
#include "Timer.h"

volatile int ping_done = 0;                 // Flag for ISR to increment.
volatile unsigned int rising_time = 0;
volatile unsigned int falling_time = 0;

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
    TIMER3_CTL_R &= ~0x100;       // Disable timer by clearing bit 8.
    TIMER3_CFG_R  = 0x4;          // Selects the 16-bit timer configuration.
    TIMER3_TBMR_R = 0b00111;      // Select capture mode, edge-time mode, and timer counts down.
    TIMER3_CTL_R |= 0xC00;        // Select both edges as trigger by writing 11 to bits 11:10.
    TIMER3_TBPR_R = 0xFF;         // Load the prescaler which works like linear extension is current mode.
    TIMER3_TBILR_R = 0xFFFF;      // Start counting from a  maximum 16-bit value, going down.

    // Interrupt configuration (Interrupt number 36)
    TIMER3_IMR_R |= 0x400;                      // Enable Capture Mode Event Interrupt Mask.
    TIMER3_ICR_R  = 0x400;                      // Clear any stale interrupt flags before enabling NVIC.
    NVIC_EN1_R = 0b1 << (36 % 32);              // Enable interrupt 36 in. NVIC
    IntRegister(52, ping_interrupt_handler);    // Register the ISR.
}

int ping_read() {
    // Reset GPIO to output to send initial signal
    TIMER3_CTL_R &= ~0x100;             // Disable timer so it does not capture triggers (Do I need this, considering that the pin is output now)?
    GPIO_PORTB_AFSEL_R &= ~0b1000;      // Disable alternative function
    GPIO_PORTB_DIR_R |= 0b1000;         // Set pin pb3 as output
    ping_done = 0;                      // Reset interrupt flag variable

    // Send the low-high-low signal to trigger the sensor.
    GPIO_PORTB_DATA_R &= ~0b1000;       // Clear pin 3 to low first.
    timer_waitMicros(20);               // Let the pin to stay low for 20 microseconds.

    GPIO_PORTB_DATA_R |= 0b1000;        // Send high signal to the sensor.
    timer_waitMicros(5);                // Trigger for 5 microseconds, as indicated on the datasheet.
    GPIO_PORTB_DATA_R &= ~0b1000;       // Make the signal low back.

    // Reconfiguring to input capture mode, to catch rising and falling edges.
    GPIO_PORTB_DIR_R &= ~0b1000;        // Set pin pb3 as input.
    GPIO_PORTB_AFSEL_R |= 0b1000;       // Enable alternative function which is already configured.
    TIMER3_ICR_R = 0x400;               // Clear the interrupt status.
    TIMER3_CTL_R |= 0x100;              // Enable timer.

    // Time capture and processing block
    while (ping_done != 2)                                   // Wait until ISR increases done to 2.
        ;

    unsigned int time_difference = (rising_time - falling_time) & 0xFFFFFF;   // If here, then 2 edges were captured.


    return 0;
}


void ping_interrupt_handler() {
    TIMER3_ICR_R = 0x400;       // Clear interrupt status bit.

    // Check the flag and save time accordingly. First goes rising edge, then falling.
    if (ping_done == 0) {
        rising_time = TIMER3_TBR_R;
    } else if (ping_done == 1) {
        falling_time = TIMER3_TBR_R;
    }

    ping_done++;                // Increment the flag.
}



