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

  .bss (NOLOAD) :
  {
    . = ALIGN(4);
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __bss_end = .;
  }
  
}
