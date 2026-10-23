; isr.asm — CPU exception entry stubs (vectors 0-31) for Atobold.
;
; Every stub normalizes the stack to:
;     [vector][error code][EIP][CS][EFLAGS]
; Exceptions 8, 10-14 and 17 push their own error code; every other
; exception pushes a dummy 0, so the C handler always sees one uniform
; frame. (On CPUs with CET/SEV features, vectors 21/29/30 would also
; push an error code, but this kernel enables none of those features.)
; isr_common then saves all registers and calls fault_handler, which
; prints the report and never returns.

[bits 32]

extern fault_handler
extern irq_handler

global inb
global outb
global io_wait


%macro ISR_NOERR 1
global isr%1
isr%1:
    push dword 0            ; dummy error code
    push dword %1           ; exception vector
    jmp  isr_common
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    push dword %1           ; error code was already pushed by the CPU
    jmp  isr_common
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

%macro IRQ 2
irq%1:
  push dword 0 
  push dword %2
  jmp irq_common
%endmacro

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

outb:
  mov dx, [esp+4]
  mov al, [esp+8]
  out dx , al 
  ret

inb:
  mov dx, [esp+4]
  in al, dx
  movzx eax , al 
  ret

io_wait:
  mov al, 0
  out 0x80, al
  ret

irq_common: 
    cld                     ; the C ABI requires DF clear
    pusha                   ; saves EAX ECX EDX EBX ESP EBP ESI EDI
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10            ; known flat data selectors
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp                ; argument: pointer to the interrupt frame
    call irq_handler      ; NOTE: relies on the -O0 build (no SSE/
    add esp, 4             ; aligned-move codegen), so the frame does
                          ; not need forced 16-byte stack alignment
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret


isr_common:
    cld                     ; the C ABI requires DF clear
    pusha                   ; saves EAX ECX EDX EBX ESP EBP ESI EDI
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10            ; known flat data selectors
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp                ; argument: pointer to the interrupt frame
    call fault_handler      ; NOTE: relies on the -O0 build (no SSE/
                            ; aligned-move codegen), so the frame does
                            ; not need forced 16-byte stack alignment
.hang:                      ; fault_handler never returns; safety net
    cli
    hlt
    jmp .hang

; C-visible table: isr_stub_table[i] is the address of the stub for
; vector i. interrupts.c installs all 32 gates from it in one loop.
global isr_stub_table
isr_stub_table:
%assign v 0
%rep 32
    dd isr %+ v
    %assign v v+1
%endrep

global irq_stub_table
irq_stub_table:
%assign v 0
%rep 16
  dd irq %+ v 
  %assign v v+1 
%endrep
