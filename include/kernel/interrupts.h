#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
/* Install the 32 CPU exception gates and load the IDT.
   Hardware interrupts stay masked; this only catches faults, so a
   kernel bug produces a readable report on the screen and on COM1
   instead of a silent triple-fault reboot. */
void interrupts_init(void);
void pic_remap(int offset1, int offset2);

void pic_send_eoi(uint8_t irq);
void cli(void);
extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);
extern void io_wait();



#endif
