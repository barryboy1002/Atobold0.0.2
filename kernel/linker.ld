ENTRY(start)

SECTIONS
{
  . =  0x9000;

  .text :
  {
    *(.text*)  
    *(.rodata*)
  }

  .data :
  {
    *(.data*)
  }

  .bss :
  {
    *(.bss*)
  }
  
}
