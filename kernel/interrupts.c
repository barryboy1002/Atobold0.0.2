#include "kernel/interrupts.h"

#include "kernel/drivers/serial.h"
#include "kernel/drivers/vga_text.h"
#include "kernel/drivers/timer.h"


/* IDT gate: present, DPL 0, 32-bit interrupt gate. Interrupt gates
   clear IF on entry, so a handler can never nest with an IRQ. */
#define GATE_PRESENT_32_INT 0x8E
#define KERNEL_CODE_SELECTOR 0x08



#define PIC1 0x20 /* address for the master PIC */
#define PIC2 0xA0 /* address for  the slave PIC */ 

#define PIC1_COMM PIC1
#define PIC1_DATA PIC1 + 1
#define PIC2_COMM PIC2
#define PIC2_DATA PIC2 + 1

#define ICW1_ICW4 0x01 /* indicates that ICW4 WILL BE PRESENT */ 
#define ICW1_SINGLE 0x02 /* Single (cascade) mode */ 
#define ICW1_INTERVAL4 0x04 /*Call address interval 4 */ 
#define ICW1_LEVEL 0x08 /* Level triggered edge mode */ 
#define ICW1_INIT 0x10 /* Initialisation -required */ 

#define ICW4_8086 0x01 /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0X02 /*Auto (normal) EOI */ 
#define ICW4_BUF_SLAVE 0x08 /*Buffered model/slave*/
#define ICW4_BUF_MASTER 0x0C /*Buffered model/master*/

#define CASCADE_IRQ 2

#define PIC_EOI 0x20



typedef struct {
  uint16_t offset_lo;
  uint16_t selector;
  uint8_t  zero;
  uint8_t  flags;
  uint16_t offset_hi;
} __attribute__((packed)) idt_entry;

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idt_register;

static idt_entry idt[256];
static idt_register idtr;

/* the 32 exception stubs from isr.asm, in vector order */
extern uint32_t isr_stub_table[32];
extern uint32_t irq_stub_table[16];

static const char *const exception_names[32] = {
  "DIVIDE BY ZERO",         "DEBUG",                "NMI",
  "BREAKPOINT",             "OVERFLOW",             "BOUND RANGE",
  "INVALID OPCODE",         "DEVICE NOT AVAILABLE", "DOUBLE FAULT",
  "COPROCESSOR OVERRUN",    "INVALID TSS",          "SEGMENT NOT PRESENT",
  "STACK FAULT",            "GENERAL PROTECTION",   "PAGE FAULT",
  "RESERVED",               "X87 FPU ERROR",        "ALIGNMENT CHECK",
  "MACHINE CHECK",          "SIMD FP",              "VIRTUALIZATION",
  "CONTROL PROTECTION",     "RESERVED",             "RESERVED",
  "RESERVED",               "RESERVED",             "RESERVED",
  "RESERVED",               "RESERVED",             "RESERVED",
  "RESERVED",               "RESERVED"
};

static void idt_set_gate(uint8_t vector, uint32_t handler)
{
  idt[vector].offset_lo = handler & 0xFFFF;
  idt[vector].selector  = KERNEL_CODE_SELECTOR;
  idt[vector].zero      = 0;
  idt[vector].flags     = GATE_PRESENT_32_INT;
  idt[vector].offset_hi = handler >> 16;
}



void interrupts_init(void)
{
  for (uint8_t i = 0; i < 32; i++) {
    idt_set_gate(i, isr_stub_table[i]);
  }
  for (uint8_t i = 0; i < 16; i++){
    uint8_t j = i + 32;
    idt_set_gate(j, irq_stub_table[i]);
  }

  idtr.limit = sizeof(idt) - 1;
  idtr.base  = (uint32_t)idt;
  __asm__ volatile ("lidt %0" : : "m"(idtr));
  pic_remap(0x20,0x28);
  __asm__ volatile("sti");
}

/* ---------------- exception reporter ---------------- */

/* the frame built by isr_common in isr.asm, from low address up */
typedef struct {
  uint32_t gs, fs, es, ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pusha order */
  uint32_t vector, error_code;
  uint32_t eip, cs, eflags;
  /* the esp field is the pusha snapshot: it points at `vector` */
} interrupt_frame;

void pic_send_eoi(uint8_t irq){
  if(irq >= 8)
    outb(PIC2_COMM, PIC_EOI);
  outb(PIC1_COMM,PIC_EOI);
}

static char *append_str(char *p, const char *s)
{
  while (*s != '\0') {
    *p++ = *s++;

  }
  return p;
}

static char *append_hex32(char *p, uint32_t value)
{
  static const char digits[] = "0123456789ABCDEF";
  for (int shift = 28; shift >= 0; shift -= 4) {
    *p++ = digits[(value >> shift) & 0xF];
  }
  return p;
} 

/* one line to both outputs: the screen and COM1 */
static void report(vga_text *screen, const char *line)
{
  vga_text_write(screen, line);
  serial_write(line);
}

__attribute__((noreturn))
void fault_handler(interrupt_frame *frame)
{
  /* a crashing kernel cannot trust its own globals: use a local
     terminal over the VGA buffer, always from a cleared screen */
  static vga_text screen;
  vga_text_init(&screen);

  uint32_t cr0, cr2, cr3;
  __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
  __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
  __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));

  char line[96];
  char *p;

  vga_text_set_color( &screen, VGA_COLOR_WHITE, VGA_COLOR_RED);
  report(&screen,
         "                              ATOBOLD KERNEL PANIC"
         "                              \n\n");
  vga_text_set_color(&screen, VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

  p = append_str(line, "EXCEPTION ");
  p = append_hex32(p, frame->vector);
  p = append_str(p, ": ");
  p = append_str(p, exception_names[frame->vector]);
  p = append_str(p, "\n\n");
  *p = '\0';
  report(&screen, line);

  p = append_str(line, "EAX=");
  p = append_hex32(p, frame->eax);
  p = append_str(p, "  EBX=");
  p = append_hex32(p, frame->ebx);
  p = append_str(p, "  ECX=");
  p = append_hex32(p, frame->ecx);
  p = append_str(p, "  EDX=");
  p = append_hex32(p, frame->edx);
  p = append_str(p, "\n");
  *p = '\0';
  report(&screen, line);

  p = append_str(line, "ESI=");
  p = append_hex32(p, frame->esi);
  p = append_str(p, "  EDI=");
  p = append_hex32(p, frame->edi);
  p = append_str(p, "  EBP=");
  p = append_hex32(p, frame->ebp);
  p = append_str(p, "  ESP=");
  p = append_hex32(p, frame->esp);
  p = append_str(p, "\n");
  *p = '\0';
  report(&screen, line);

  p = append_str(line, "CR0=");
  p = append_hex32(p, cr0);
  p = append_str(p, "  CR2=");
  p = append_hex32(p, cr2);
  p = append_str(p, "  CR3=");
  p = append_hex32(p, cr3);
  p = append_str(p, "  EFLAGS=");
  p = append_hex32(p, frame->eflags);
  p = append_str(p, "\n");
  *p = '\0';
  report(&screen, line);

  p = append_str(line, "CS=");
  p = append_hex32(p, frame->cs);
  p = append_str(p, "  EIP=");
  p = append_hex32(p, frame->eip);
  p = append_str(p, "  ERROR=");
  p = append_hex32(p, frame->error_code);
  p = append_str(p, "\n");
  *p = '\0';
  report(&screen, line);

  p = append_str(line, "DS=");
  p = append_hex32(p, frame->ds);
  p = append_str(p, "  ES=");
  p = append_hex32(p, frame->es);
  p = append_str(p, "  FS=");
  p = append_hex32(p, frame->fs);
  p = append_str(p, "  GS=");
  p = append_hex32(p, frame->gs);
  p = append_str(p, "\n");
  *p = '\0';
  report(&screen, line);

  if (frame->vector == 14) {
    p = append_str(line, "PAGE FAULT at ");
    p = append_hex32(p, cr2);
    p = append_str(p, ": ");
    p = append_str(p, (frame->error_code & 0x2) ? "write" : "read");
    p = append_str(p,
      (frame->error_code & 0x4) ? " from user" : " from kernel");
    p = append_str(p,
      (frame->error_code & 0x1) ? ", page present" : ", page not present");
    p = append_str(p, "\n");
    *p = '\0';
    report(&screen, line);
  }

  report(&screen, "\nSystem halted.\n");

  for (;;) {
    __asm__ volatile ("hlt");
  }
}

void irq_handler(interrupt_frame  * frame){
  /* handle my hardware interrupts*/
  switch(frame->vector-32){
    case 0:
      timer_handler();
      break;
    case 1:
      serial_write("keyboard interrupt");
      break;
    default:
      serial_write( "I probably am not handling that yet");
      break;
  }

  pic_send_eoi(frame->vector - 32);

}


void pic_remap(int offset1, int offset2)
{ /*save state of enabled Irqs*/
  uint8_t a1 = inb(PIC1_DATA);
  uint8_t a2 = inb(PIC2_DATA);
  
 /* initialisation of the PIC controller giving them specific vector offsets */
  outb(PIC1_COMM, ICW1_INIT | ICW1_ICW4);  /*starting the initialisation sequence (in cascade mode)*/
  io_wait();
  outb(PIC2_COMM, ICW1_INIT | ICW1_ICW4);
  io_wait();

  /* set the vector ofsets fo the PIC*/ 
  outb(PIC1_DATA, offset1);
  io_wait();
  outb(PIC2_DATA, offset2); 
  io_wait();

  /*Connect master  and slave through IRQ2*/
  outb (PIC1_DATA,  1 << CASCADE_IRQ);
  io_wait();
  outb (PIC2_DATA, CASCADE_IRQ);
  io_wait();

  /*select 8086 interrrupt mode*/
  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  /*unmask PICS*/
  outb(PIC1_DATA, a1);
  outb(PIC2_DATA, a2);
}
