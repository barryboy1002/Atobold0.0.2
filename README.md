# Atobold 0.0.2

A small x86 educational kernel: a 512-byte BIOS boot sector loads a
kernel at 0x9000, switches the CPU to 32-bit protected mode, and runs
a C kernel with VGA text output, serial (COM1) debug output, and CPU
exception reporting.

## Building

You need `make`, `nasm`, and either the i686-elf cross toolchain or a
host gcc with 32-bit support (`gcc-multilib` on Debian/Ubuntu). QEMU
is needed to run the kernel and the tests.

    make                 # build build/kernel.img
    make run             # boot it in QEMU (COM1 on the terminal)
    make test            # headless boot test: banner on COM1 and on screen
    make test-panic      # boot a build that faults on purpose, check the report
    make gdb             # boot paused with a gdb stub, attach gdb (see .gdbinit)
    make clean

The default toolchain is the i686-elf cross compiler in ~/opt/cross.
Override it with the CROSS variable:

    make CROSS=              # host gcc/ld/objcopy from PATH (needs -m32 support)
    make CROSS=i686-elf-     # cross tools from PATH

The bootloader loads 60 sectors (30 KiB) of kernel; the build fails
with a clear message if the kernel outgrows that. KERNEL_SECTORS in
the Makefile must stay in sync with `mov al, 60` in
bootloader/bootloader.asm.

## Layout

    bootloader/bootloader.asm   boot sector: prints, loads the kernel, jumps
    kernel/basic_kernel.asm     16-bit entry stub, GDT, protected-mode switch
    kernel/kernel_main.c        C entry point
    kernel/vga_text.c           80x25 VGA text output
    kernel/serial.c             polled COM1 output
    kernel/isr.asm              CPU exception entry stubs
    kernel/interrupts.c         IDT setup and exception reporter
    kernel/linker.ld            links the kernel at 0x9000

## Debugging

- Serial: `make run` shows COM1 on the terminal. Everything the kernel
  prints goes there too, including crash reports.
- Crashes: any CPU exception prints a register dump to the screen and
  to COM1, then halts. `make test-panic` shows what that looks like.
- gdb: `make gdb` boots QEMU paused with a gdb stub and attaches gdb
  with kernel symbols. `.gdbinit` defines `rm`/`pm` mode switches and
  a `gdt` inspection command.

## License

Not chosen yet — worth deciding before accepting outside contributions.
