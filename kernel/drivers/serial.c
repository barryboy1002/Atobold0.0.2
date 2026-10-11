#include "serial.h"

#include <stddef.h>
#include <stdint.h>

/* COM1 registers (base 0x3F8). Offset 0 is the data register, which
   doubles as the divisor-latch low byte while the DLAB bit (bit 7 of
   the line control register, offset 3) is set. */
#define COM1       0x3F8
#define REG_DATA   0 /* DLAB=0: tx/rx buffer, DLAB=1: divisor low  */
#define REG_INTR   1 /* DLAB=0: interrupt enable, DLAB=1: divisor hi */
#define REG_FIFO   2
#define REG_LINE   3
#define REG_MODEM  4
#define REG_STATUS 5 /* line status; bit 5 = transmitter holding empty */

static int serial_ok;

static void outb(uint16_t port, uint8_t value)
{
  __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static uint8_t inb(uint16_t port)
{
  uint8_t value;
  __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
  return value;
}

void serial_init(void)
{
  /* Presence check: loop one byte through the UART in loopback mode.
     If it does not come back there is no UART on this port, and we
     skip all further writes instead of hanging on a status bit that
     never sets. */
  outb(COM1 + REG_MODEM, 0x1E);
  outb(COM1 + REG_DATA, 0xAE);
  if (inb(COM1 + REG_DATA) != 0xAE) {
    serial_ok = 0;
    return;
  }
  serial_ok = 1;

  outb(COM1 + REG_INTR, 0x00);  /* no UART interrupts; we poll        */
  outb(COM1 + REG_LINE, 0x80);  /* DLAB on                            */
  outb(COM1 + REG_DATA, 0x01);  /* divisor 1 -> 115200 baud           */
  outb(COM1 + REG_INTR, 0x00);
  outb(COM1 + REG_LINE, 0x03);  /* 8 data bits, no parity, 1 stop bit */
  outb(COM1 + REG_FIFO, 0xC7);  /* FIFO on, cleared, 14-byte trigger  */
  outb(COM1 + REG_MODEM, 0x03); /* DTR + RTS                          */
}

void serial_putchar(char c)
{
  if (!serial_ok) {
    return;
  }
  if (c == '\n') {
    serial_putchar('\r'); /* terminals expect CR LF */
  }
  while (!(inb(COM1 + REG_STATUS) & 0x20)) {
    /* wait for the transmitter holding register to drain */
  }
  outb(COM1 + REG_DATA, (uint8_t)c);
}

void serial_write(const char * string)
{
  for (size_t i = 0; string[i] != '\0'; i++) {
    serial_putchar(string[i]);
  }
}
