# Atobold debug helpers.
#
# The boot chain starts in real mode and switches to 32-bit protected
# mode (kernel/basic_kernel.asm, enter_protected). The right way to
# decode instructions differs between the two:
#
#   real mode:      linear address = cs * 16 + ip
#   protected mode: linear address = eip (flat 4 GiB segments)
#
# Switch decoding with the `rm` / `pm` commands after each transition.
# This file deliberately does NOT connect to QEMU (connecting here used
# to cause double-connection errors when it got sourced twice); use
# `make gdb`, or start QEMU with -s -S yourself and then run:
#   target remote :1234

set pagination off

# NOTE on real-mode disassembly: gdb versions differ. On gdb < 16,
# `set architecture i8086` decodes 16-bit code correctly. On gdb 16.x
# the i8086 setting is accepted but x/i still prints 32-bit operand
# sizes and can merge instructions — the ADDRESSES (cs*16+pc) remain
# correct either way. For a byte-exact 16-bit listing use:
#   objdump -D -b binary -m i8086 --adjust-vma=0x7c00 build/boot.bin

# --- mode switches -----------------------------------------------------

define rm
  set architecture i8086
end
document rm
Decode 16-bit real-mode code. Linear address = cs*16 + ip.
end

define pm
  set architecture i386
end
document pm
Decode 32-bit protected-mode code. Linear address = eip (flat segments).
end

# --- disassembly helpers -----------------------------------------------

define xi
  x/20i $pc
end
document xi
Disassemble 20 instructions at the current position (protected mode).
end

define xirm
  x/20i (($cs * 16) + $pc)
end
document xirm
Disassemble 20 instructions at the current position (real mode).
end

define sii
  stepi
  x/10i $pc
end
document sii
Step one instruction, then show 10 (protected mode).
end

define siirm
  stepi
  x/10i (($cs * 16) + $pc)
end
document siirm
Step one instruction, then show 10 (real mode).
end

# --- GDT inspection ----------------------------------------------------

define gdt
  # needs symbols: run gdb on build/kernel.elf
  x/24xb &gdt_start
  monitor info registers
end
document gdt
Dump the 24 GDT bytes from the symbol in kernel.elf, plus QEMU's own
"GDT= base limit" line as a cross-check. Notes so the values do not
look like bugs: the limit reads 23 (0x17) for a 24-byte table because
the field stores size-1, and the data descriptor's access byte reads
0x93 in memory (source says 0x92) because the CPU sets the Accessed
bit when it loads the descriptor. Both are normal.
end
