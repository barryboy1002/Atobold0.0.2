#ifndef TIMER_H
#define TIMER_H


#include <stdint.h>

extern uint32_t second;
void timer_init(uint32_t frequency);
void timer_handler(void);
uint32_t timer_get_ticks();

#endif



