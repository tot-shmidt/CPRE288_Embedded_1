#ifndef MY_PING_H
#define MY_PING_H

void ping_init();

int ping_read(int *cycles_num, float *time_ms);

void ping_interrupt_handler();


#endif
