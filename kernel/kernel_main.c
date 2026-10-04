#include "vga_text.h"
#include "serial.h"
#include "interrupts.h"


static vga_text terminal;
void terminal_setup()
{
  vga_text_init(&terminal);
  serial_init();
  interrupts_init();

  /* one boot banner on both outputs: the screen and COM1 */
  vga_text_writeline(&terminal, "Atobold 0.0.2");
  serial_write("Atobold 0.0.2\n");

#ifdef CRASH_DEMO
  /* deliberate fault so the exception reporter can be seen and tested;
     'make test-panic' builds with this defined */
  __asm__ volatile ("ud2");
#endif
}

void kernel_main(void)
{
  terminal_setup();

  /* nothing to do yet; halt instead of spinning (the old busy loop
     pinned a host CPU core at 100% under QEMU) */
  for (;;) {
    __asm__ volatile ("hlt");
  }
}
