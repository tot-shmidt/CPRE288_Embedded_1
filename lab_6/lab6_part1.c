#include "button.h"
#include "Timer.h"
#include "lcd.h"

extern volatile int button_event;
extern volatile int button_num;

int main() {
    button_init();
    init_button_interrupts();
    lcd_init();

    // When no button has been pressed, 0 should be displayed. How do I handle this?
    lcd_printf("Button pressed: 0");

    while (1) {
        if (button_event == 1) {
            lcd_printf("Button pressed: %d", button_num);
            button_event = 0;
        }

        /* I would add part bellow if I didn't have both edges interrupt to change output to 0 after a button
         * is released. In our case, with both edges we are getting 0 for free, as ISR is being triggered
         * after a button is released, and button_getButton() gets 0, and it gets printted in if statement.
         *
        else if (button_num != 0 && button_getButton() == 0) {
            buton_num = 0;
            lcd_printf("Button pressed: 0");
        } */
    }

    return 0;
}
