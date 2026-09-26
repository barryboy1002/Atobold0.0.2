[bits 16]
start:
  mov ax , cs
  mov ds, ax

  mov si, hello_string
  call print_string

  jmp enter_protected

print_string:
  mov ah , 0eh 

print_char:
  lodsb

  cmp al, 0
  je done
  int 10h 
  jmp print_char

done:
  ret 

enter_protected:
  cli ;disable the hardware interrupts
  lgdt [gdtr] ;load the GDT register with start address of GDT
  mov eax , cr0 
  or eax ,1 ; set protection enable bit in control register 0 (cr0)
  mov cr0 , eax

  CODE_SEG equ gdt_code - gdt_start
  jmp  CODE_SEG:p_mode_main

[bits 32]
p_mode_main:
  mov ax, 10h
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  mov esp, 0x9000

hang:
  jmp hang

hello_string db "Hello world!, I am  Atobold0.0.2", 0


gdt_start:
gdt_null:
  dq 0

gdt_code:
  dw 0xFFFF; limit
  dw 0x0000; base low
  db 0x00 ;base middle
  db 0x9A ;access
  db 0xCF ; flags limit
  db 0x00; low address
gdt_data:
  dw 0xFFFF
  dw 0x0000
  db 0x00
  db 0x92
  db 0xCF
  db 0x00

gdt_end:
gdtr:
  dw gdt_end - gdt_start -1
  dd gdt_start

