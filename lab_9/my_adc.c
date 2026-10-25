#include "Timer.h"
#include <inc/tm4c123gh6pm.h>

/*
 * Initializes PB4 (pin 58) as analog input (AIN 10) for IR sensor, to convert analog voltage to digital number with ADC0, SS0.
 */
void adc_init() {
    // *************************** System/GPIO configuration ***************************

    // Turn on the clock on the port B (RCGCPIO), and the clock for ADC0 in RCGCADC.
    SYSCTL_RCGCGPIO_R |= 0b10;  // Set bit 1 for Port B
    SYSCTL_RCGCADC_R  |= 0b01;  // Set bit 0 for ADC0

    timer_waitMillis(5);        // Wait until the port and device gets powered.

    // Select alternative function mode in (AFSEL) on PB4
    GPIO_PORTB_AFSEL_R |= 0b10000;  // Set bit 4 for PB4

    // Close digital "path" in (GPIODEN) and open analog "path" on PB4.
    GPIO_PORTB_DEN_R   &= ~0b10000;  // Clear bit 4 for PB4
    GPIO_PORTB_AMSEL_R |= 0b10000;   // Set bit 4 for PB4


    // *************************** Sample Sequencer0 configuration ***************************

//    ADC0_ACTSS_R &= ~0b01;          // Disable sample sequencer 0 until the end of configuration.
//    ADC0_EMUX_R  &= ~0x0F;          // Select a processor to be the trigger for ADC. To trigger - write to ADCPSSI.
//    ADC0_SSMUX0_R = 0xAAAAAAAA;     // Choose every input channel out of 8 to be AIN10, so I can average 8 samples.
//    ADC0_SSCTL0_R = 0x60000000;     // Mark sample eight as end of sequence and allow it to trigger raw interrupt.
//    // ADC0_IM_R  = ???             ( I am not sure if I am going to use interrupts here).
//    ADC0_CC_R     = 0x0;            // Set System Clock as a clock source for ADC0.
//    ADC0_ACTSS_R |= 0b01;           // Activate sample sequencer 0 after it was configured.

    ADC0_ACTSS_R &= ~0b01;          // Disable sample sequencer 0 until the end of configuration.
    ADC0_EMUX_R  &= ~0x0F;          // Select a processor to be the trigger for ADC. To trigger - write to ADCPSSI.

    ADC0_SSMUX0_R = 0x0A;            // Have only one input channel set out of 8 to be AIN10. Will use hardware averaging to get a better result.
    ADC0_SSCTL0_R = 0x06;            // Mark sample one as the end of the sequence and allow it to trigger raw interrupt.
    ADC0_SAC_R    = 0x05;            // Will take 32 samples and produce one average value in the fifo.
    // ADC0_IM_R  = ???             ( I am not sure if I am going to use interrupts here).
    ADC0_CC_R     = 0x0;            // Set System Clock as a clock source for ADC0.
    ADC0_ACTSS_R |= 0b01;           // Activate sample sequencer 0 after it was configured.
}

/*
 * Get one scan value
 */
int adc_read() {
    int scan_voltage = 0;                   // Here the raw valtage will be stored from a scan.
    ADC0_PSSI_R = 0b01;                     // Write 1 to begin sampling on Samle Squencer 0.

    while ((ADC0_RIS_R & 0b01) != 0b01)     // Wait until SS0 raw interrupt status becomes 1.
        ;

    ADC0_ISC_R   = 0b01;                    // Clear RIS interrupt status bit.

    scan_voltage = ADC0_SSFIFO0_R & 0xFFF;  // Do I need to mask lower 12 bits of the FIFO register?

    return scan_voltage;
}
