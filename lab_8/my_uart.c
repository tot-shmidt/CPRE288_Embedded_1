#include "my_uart.h"
#include "Timer.h"
#include "driverlib/interrupt.h"
#include <inc/tm4c123gh6pm.h>


#define REPLACE_ME 0x00

volatile char uart_data = 0;
volatile char flag = 0;


void uart_init(int baud) {
    SYSCTL_RCGCGPIO_R  |= 0b10;                                 // enable clock GPIOB (page 340)
    SYSCTL_RCGCUART_R  |= 0b10;                                 // enable clock UART1 (page 344)
    timer_waitMillis(1);                                        // This is necessary according to the data sheet

    GPIO_PORTB_AFSEL_R |= 0b11;                                 // sets PB0 and PB1 as peripherals (page 671)
    GPIO_PORTB_PCTL_R  |= 0x11;                                 // pmc0 and pmc1       (page 688)  also refer to page 650. PB0: U1Rx 1 ; PB1: U1Tx 1.
    GPIO_PORTB_DEN_R   |= 0b11;                                 // enables pb0 and pb1
    GPIO_PORTB_DIR_R    = (GPIO_PORTB_DIR_R | 0b10) & ~0b01;    // sets pb0 as input, pb1 as output

    //compute baud values [UART clock= 16 MHz] 
    float clock_rate = 16000000.0f;

    int fbrd;
    int ibrd;

    float brd = clock_rate / (16.0f * baud);
    ibrd = (int) brd;                               // Extract integer part of brd
    fbrd = (int) ((brd - ibrd) * 64.0f + 0.5f);     // Extract first decimal part, then multiply and add magic numbers

    UART1_CTL_R &= ~0b01;                 // disable UART1 (page 918)
    UART1_IBRD_R = ibrd;                  // write integer portion of BRD to IBRD
    UART1_FBRD_R = fbrd;                  // write fractional portion of BRD to FBRD
    UART1_LCRH_R = 0b01110000;            // write serial communication parameters (page 916) * 8bit and no parity
    UART1_CC_R   = 0x0;                   // use system clock as clock source (page 939)
    UART1_CTL_R |= 0b001100000001;        // enable UART1 and RXE and TXE bits.
}

void uart_sendChar(char data) {
    // Make sure transmit FIFO is not full. Check TXFF bit (5) is not set.
    while ((UART1_FR_R & 0x20) != 0)
        ;

    UART1_DR_R = data;
}

char uart_receive(void) {
    // Make sure receive FIFO is not empty. check that RXFE bit (4) is not set.
    while ((UART1_FR_R & 0x10) != 0)      // Loop until RXFE bit becomes 0.
        ;
 
    char symbol = (UART1_DR_R & 0xFF);    // By assigning 32 bits into char, implicit casting happens, and 8 lower bits are only preserved.

    if (symbol == '\r') {
        uart_sendChar('\n');
        uart_sendChar('\r');
    } else {
        uart_sendChar(symbol);
    }

    return symbol;
}

void uart_sendStr(const char *data) {
    int i = 0;

    while (data[i] != '\0') {
        uart_sendChar(data[i]);
        i++;
    }
}

// _PART3


void uart_interrupt_init() {
    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    // Note: NVIC register descriptions are found in chapter 3.4
    NVIC_EN0_R |= 0b1000000; //enable uart1 interrupts - page 104 (Interrupt number 6)

    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= 0b1010000; //enable interrupt on receive - page 924

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    IntRegister(INT_UART1, uart_interrupt_handler); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number
}


void uart_interrupt_handler() {
    // STEP1: Check the Masked Interrup Status
    if ((UART1_MIS_R & 0b1010000) != 0) {
        //STEP2:  Copy the data
        uart_data = UART1_DR_R & 0xFF;

        uart_sendChar(uart_data);   // Echo the data back

        //STEP3:  Clear the interrup
        UART1_ICR_R = 0b1010000;

        flag = 1;
    }
}
