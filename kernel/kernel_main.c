#include "vga_text.h"


static vga_text terminal;
void terminal_setup()
{
  vga_text_init(&terminal);
  vga_text_putchar(&terminal, 'A');
}

void kernel_main(void)
{
  terminal_setup();

  for (;;);
}
