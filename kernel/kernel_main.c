#include "vga_text.h"


static vga_text terminal;
void terminal_setup()
{
  vga_text_init(&terminal);
  vga_text_clear(&terminal);
  vga_text_write(&terminal, "Hello from C");
}

void kernel_main(void)
{
  volatile char * vga = (volatile char * )0xB8000;

  //signal that we have reached c 
  vga[0] = 'c';
  vga[1] = 0x02;
  terminal_setup();  



  for (;;); 
}
