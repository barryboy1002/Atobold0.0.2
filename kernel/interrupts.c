#include "interrupts.h"

#include "serial.h"
#include "vga_text.h"

#include <stdint.h>

/* IDT gate: present, DPL 0, 32-bit interrupt gate. Interrupt gates
   clear IF on entry, so a handler can never nest with an IRQ. */
#define GATE_PRESENT_32_INT 0x8E
#define KERNEL_CODE_SELECTOR 0x08

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
  for (int i = 0; i < 32; i++) {
    idt_set_gate((uint8_t)i, isr_stub_table[i]);
  }

  idtr.limit = sizeof(idt) - 1;
  idtr.base  = (uint32_t)idt;
  __asm__ volatile ("lidt %0" : : "m"(idtr));
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
  vga_text screen;
  vga_text_init(&screen);

  uint32_t cr0, cr2, cr3;
  __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
  __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
  __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));

  char line[96];
  char *p;

  vga_text_set_color(&screen, VGA_COLOR_WHITE, VGA_COLOR_RED);
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
