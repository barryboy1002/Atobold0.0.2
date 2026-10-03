start:
  xor ax, ax
  mov ss, ax
  mov sp, 0x7C00
  mov ax, 07C0h
  mov ds, ax
  mov [boot_drive], dl ;hand over the boot drive identifier incase of dl is overridden

  mov si, title_string
  call print_string

  mov si, message_string
  call print_string

  call load_kernel_from_disk
  jmp 0900h:0000;gives control to the kernel by jumping to its starting point.

load_kernel_from_disk:
  mov ax, 0900h
  mov es, ax

  mov ah, 02h ; service number , BIOS read sector function
  mov al, 60  ; number of sectors to read from disk
  mov ch, 0h  ; track number (cylinder 0)
  mov cl, 02h ; sector number (2nd sector, 1-indexed)
  mov dh, 0h  ; head number 0
  mov dl, [boot_drive] ; drive number passed by BIOS
  mov bx, 0h  ; memory offset (es:bx = 0900h:0000h = 0x9000)
  int 13h

  ;INT 13h clears the carry flag on success  and  sets its  on error.
  jc kernel_load_error

  ret

kernel_load_error:
  mov si, load_error_string
  call print_string

  jmp $

print_string:
    mov ah, 0Eh ; bios number 0Eh, sets for teletype output function

print_char:
    lodsb ; loads byte at SI into AL and increments SI

    cmp al, 0 ; 0 stored in al if at end of string
    je printing_finished

    int 10h ;bios interrupt 0x10, to print char stored in AL
    jmp print_char

printing_finished:
  ;print new line 
  mov al , 10d; ASCII code for new line
  int 10h 

  ;read current  cursor position 
  mov ah, 03h ; function to read current cursor position
  mov bh ,0 ;page number for default page
  int 10h ; now used to read  cursor postion 

  ;move cursor to beginning 
  mov ah, 02h ; fucntion to set cursor position
  mov dl, 0 ; column  number(0  for beginning of line)
  int 10h ;0x10 to set cursor pos 

  ret 

title_string db "Welcome to the Atobold0.0.0.1 Bootloader.....", 0
message_string db "Loading up the kernel for you......", 0
load_error_string db "Oh , oh there was a problem loading  the kernel", 0 
boot_drive db 0

times 510-($-$$) db 0 ; pads the rest of the bootloader with 510 bytes , aiming  for a 512-byte bootloader
dw 0xAA55 ; specifies the end of a bootloader, recognised by the bootloader
