#ifndef SERIAL_H
#define SERIAL_H

/* Polled COM1 (0x3F8) output for early debugging: it works before and
   without VGA, in headless QEMU (-serial stdio), and over a real serial
   cable. */

void serial_init(void);
void serial_putchar(char c);
void serial_write(const char * string);

#endif
