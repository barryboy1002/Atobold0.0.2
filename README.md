 # Atobold 0.0.2

    A small 32-bit x86 educational operating system kernel built from first principles.

    The boot pipeline uses a 512-byte BIOS boot sector that loads the kernel payload into memory at `0x9000`, establishes a Global Descriptor Table (GDT),
  transitions the CPU from 16-bit real mode into 32-bit protected mode, and transfers execution to a freestanding C kernel with an Interrupt Descriptor Table
  (IDT), CPU exception reporting, serial debugging (COM1), and VGA console output.

    ---

    ## Current State & Subsystem Architecture

    * **Bootloader**: 512-byte BIOS stage (`bootloader/bootloader.asm`) that reads kernel sectors via INT 13h.
    * **CPU Transition**: 16-bit entry stub, GDT initialization, and protected-mode jump (`kernel/basic_kernel.asm`).
    * **Interrupt Subsystem**: IDT setup and CPU exception reporting with register state dumps (`kernel/interrupts.c`, `kernel/isr.asm`).
    * **Driver Subsystem**:
      * **Serial (`COM1`)**: Polled 16550 UART driver for early headless console output and panic dumps (`kernel/serial.c`).
      * **VGA Text**: Memory-mapped 80x25 text console buffer at `0xB8000` (`kernel/drivers/vga_text.c`).

    ---

    ## Project Roadmap

    Atobold is progressing through low-level operating system milestones:

    1. **Driver & Hardware Interfacing**:
       * Complete modular driver tree under `kernel/drivers/`.
       * Remap the 8259 PIC to avoid conflicts with CPU exception vectors (`0x20`–`0x2F`).
       * Programmable Interval Timer (PIT) IRQ 0 handler for system ticks and timing.
       * PS/2 Keyboard IRQ 1 driver.
    2. **Memory Management**:
       * Parse BIOS memory maps (E820) to determine physical memory topology.
       * Physical Memory Manager (PMM): Page frame allocator (bitmap or free-list).
       * Virtual Memory Manager (VMM): Two-level x86 paging (Page Directory and Page Tables) with higher-half kernel mapping.
    3. **Tasking & User Space**:
       * Task State Segment (TSS) initialization.
       * Ring 3 privilege level separation.
       * Preemptive multitasking and context switching.
       * System call interface via software interrupts.

    ---

    ## Building & Running

    ### Prerequisites
    * `make`, `nasm`
    * Cross compiler: `i686-elf-gcc`, `i686-elf-ld`, `i686-elf-objcopy` (or host GCC with `-m32` multilib support)
    * `qemu-system-i386`

    ### Commands
    ```bash
    make                 # Build bootable raw disk image (build/kernel.img)
    make run             # Run in QEMU with COM1 redirected to stdio
    make test            # Headless boot smoke-test
    make test-panic      # Build fault-injecting test kernel and verify register dump
    make gdb             # Launch QEMU with GDB stub attached (see .gdbinit)
    make clean           # Remove build artifacts
  ──────
  ## Layout

    bootloader/
      bootloader.asm      # 16-bit BIOS MBR bootloader
    kernel/
      basic_kernel.asm    # 16-bit stub, GDT, protected mode switch
      kernel_main.c       # C kernel entry point
      serial.c            # COM1 UART 16550 serial driver
      isr.asm             # Low-level CPU exception stubs
      interrupts.c        # IDT configuration and panic register dump
      linker.ld           # Kernel linker script (entry at 0x9000)
      drivers/
        vga_text.c        # 80x25 VGA buffer driver
        vga_text.h        # VGA interface and text color definitions
    tools/
      boot_test.sh        # Automated headless test harness
  ──────
  ## AI & Tooling Disclaimer

  Large Language Models (LLMs) and AI tools are utilized solely as interactive reference documentation, search engines, and analytical sounding boards (e.g.,
  assessing architectural trade-offs, debugging compiler behaviors, and reviewing hardware specs). All kernel implementation code, build system
  configurations, and architectural designs are authored and verified directly by the project developer.
  ──────
  ## License

  TBD


    ---
