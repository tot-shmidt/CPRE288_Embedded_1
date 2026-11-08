#include "servo.h"
#include "Timer.h"

/*
 * Configure Timer 1B (T1CCP1) on PB5.
 */
void servo_init() {
    // Enable clock on port b and timer 1.
    SYSCTL_RCGCGPIO_R  |= 0b10;
    SYSCTL_RCGCTIMER_R |= 0b10;
    timer_waitMillis(1);

    // Configure GPIO part
    GPIO_PORTB_DEN_R   |= 0b100000;
    GPIO_PORTB_PCTL_R   = (GPIO_PORTB_PCTL_R & ~0xF00000) | 0x700000;
    GPIO_PORTB_AFSEL_R |= 0b100000;

    // Timer configuration
    TIMER1_CTL_R &= ~0x100;     // Disable timer by clearing bit 8.
    TIMER1_CFG_R  = 0x4;
    TIMER1_TBMR_R = 0b1010;     // In the data sheet they set to say TBMR field to 0x2, which is 0b0010. This will erase bit 3 (TBAMS) which must be set for PWM. What do the mean?

    TIMER1_TBPR_R = 0x02;       // I calculated that for 10 ms period I need 160,000 cycles, which is 0x27100, so 0x02 to prescaler, and 0x7100 to ILR.
    TIMER1_TBILR_R = 0x7100;    // This should determine PWD period, so it does not have to be 0xFFFF necessarily.
    TIMER1_TBMATCHR_R = 0x4220; // I can send it to 0 degrees by default after initialization. Is it 0.75 ms high pulse?
    TIMER1_TBPMR_R    = 0x02;   // According to my calculations, 0.75 ms will take 12000 cycles, so I need to make PWM low at 148000 cycles.

    TIMER1_CTL_R |= 0x100;      // Enable timer to start generating PWM. Should PWM be generated throughout the entire program execution, or should I turn it on and off on demand?
    timer_waitMillis(500);      // Let the servo to reach 0 degrees.
}

int servo_move(float degrees) {

}

