#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/* Install the 32 CPU exception gates and load the IDT.
   Hardware interrupts stay masked; this only catches faults, so a
   kernel bug produces a readable report on the screen and on COM1
   instead of a silent triple-fault reboot. */
void interrupts_init(void);

#endif
