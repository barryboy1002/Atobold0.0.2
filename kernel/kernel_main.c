void kernel_main(void)
{
  volatile char * vga = (volatile char * )0xB8000;

  //signal that we have reached c 
  vga[0] = 'c';
  vga[1] = 0x02;

  for (;;); 
}
