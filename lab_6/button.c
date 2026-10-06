/*
 * button.c
 *
 *  Created on: Jul 18, 2016
 *      Author: Eric Middleton, Zhao Zhang, Chad Nelson, & Zachary Glanz.
 *
 *  @edit: Lindsey Sleeth and Sam Stifter on 02/04/2019
 *  @edit: Phillip Jone 05/30/2019: Mearged Spring 2019 version with Fall 2018
 */
 


//The buttons are on PORTE 3:0
// GPIO_PORTE_DATA_R -- Name of the memory mapped register for GPIO Port E, 
// which is connected to the push buttons
#include "button.h"
#include "lcd.h"
#include "cyBot_uart.h"
#include <stdio.h>

// Global varibles
volatile int button_event;
volatile int button_num;

/**
 * Initialize PORTE and configure bits 0-3 to be used as inputs for the buttons.
 */
void button_init() {
	static uint8_t initialized = 0;

	//Check if already initialized
	if(initialized){
		return;
	}
	
	// Reading: To initialize and configure GPIO PORTE, visit pg. 656 in the 
	// Tiva datasheet.
	
	// Follow steps in 10.3 for initialization and configuration. Some steps 
	// have been outlined below.
	
	// Ignore all other steps in initialization and configuration that are not 
	// listed below. You will learn more about additional steps in a later lab.

	// 1) Turn on PORTE system clock, do not modify other clock enables
	SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R | 0x00000010;

	// 2) Set the buttons as inputs, do not modify other PORTE wires
	GPIO_PORTE_DIR_R = GPIO_PORTE_DIR_R & 0xF0;
	
	// 3) Enable digital functionality for button inputs, do not modify other PORTE enables
	GPIO_PORTE_DEN_R = GPIO_PORTE_DEN_R | 0x0F;

	
	initialized = 1;
}



/**
 * Initialize and configure PORTE interupts
 */
void init_button_interrupts() {
    // In order to configure GPIO ports to detect interrupts, you will need to visit pg. 656 in the Tiva datasheet.
    // Notice that you already followed some steps in 10.3 for initialization and configuration of the GPIO ports in the function button_init().
    // Additional steps for setting up the GPIO port to detect interrupts have been outlined below.

    // 1) Mask the bits for pins 0-3 to configure edge sensing.
    GPIO_PORTE_IM_R &= 0xF0;

    // 2) Set pins 0-3 to use edge sensing
    GPIO_PORTE_IS_R &= 0xF0;

    // 3) Set pins 0-3 to use both edges. We want to update the LCD when a button is pressed, and when the button is released.
    GPIO_PORTE_IBE_R |= 0x0F;

    // 4) Clear the interrupts
    GPIO_PORTE_ICR_R = 0x0F;

    // 5) Unmask the bits for pins 0-3
    GPIO_PORTE_IM_R |= 0x0F;

    // 6) Enable GPIO port E interrupt
    NVIC_EN0_R |= 0x00000010;           // 0b0000 .. 0001 0000 - PORTE has interrupt number 4, which is the fifth bit.

    // Bind the interrupt to the handler.
    IntRegister(INT_GPIOE, gpioe_handler2);
}


/**
 * Interrupt handler -- executes when a GPIO PortE hardware event occurs (i.e., for this lab a button is pressed)
 */
// For Part 1 of lab 6
void gpioe_handler1() {
    // Clear the Interrupt status of the handler.
    GPIO_PORTE_ICR_R = 0x0F;

    button_num = button_getButton();

    button_event = 1;
}

// For Part 2 of lab 6
char prefix_msg[] = "Button pressed: ";
char final_msg[20];
int i = 0;

void gpioe_handler2() {
    // Clear the Interrupt status of the handler.
    GPIO_PORTE_ICR_R = 0x0F;

    button_num = button_getButton();

    if (button_num != 0) {
        sprintf(final_msg, "%s%d", prefix_msg, button_num);

        while (final_msg[i] != '\0') {
            cyBot_sendByte(final_msg[i]);
            i++;
        }

        cyBot_sendByte('\r');
        cyBot_sendByte('\n');

        i = 0;
    }
}




/**
 * Returns the position of the rightmost button being pushed.
 * @return the position of the rightmost button being pushed. 4 is the rightmost button, 1 is the leftmost button.  0 indicates no button being pressed
 */
uint8_t button_getButton() {
    uint8_t buttonPressed = 0;

    if ((GPIO_PORTE_DATA_R & 0b00000001) == 0) {
        buttonPressed = 1;
    }

    if ((GPIO_PORTE_DATA_R & 0b00000010) == 0) {
        buttonPressed = 2;
    }

    if ((GPIO_PORTE_DATA_R & 0b00000100) == 0) {
        buttonPressed = 3;
    }

    if ((GPIO_PORTE_DATA_R & 0b00001000) == 0) {
        buttonPressed = 4;
    }

    return buttonPressed;
}





