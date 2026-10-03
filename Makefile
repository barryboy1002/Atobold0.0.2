# Atobold 0.0.2 — build
#
# Default toolchain is the i686-elf cross compiler in ~/opt/cross.
# Override for your environment:
#   make CROSS=              host gcc/ld/objcopy from PATH (needs 32-bit support)
#   make CROSS=i686-elf-     cross tools from PATH

CROSS    ?= $(HOME)/opt/cross/bin/i686-elf-
CC       := $(CROSS)gcc
LD       := $(CROSS)ld
OBJCOPY  := $(CROSS)objcopy
NASM     ?= nasm
QEMU     ?= qemu-system-i386

BUILD_DIR  := build
KERNEL_DIR := kernel
BOOT_SRC   := bootloader/bootloader.asm
ENTRY_SRC  := $(KERNEL_DIR)/basic_kernel.asm
LDSCRIPT   := $(KERNEL_DIR)/linker.ld

C_SRCS := $(KERNEL_DIR)/kernel_main.c $(KERNEL_DIR)/vga_text.c \
          $(KERNEL_DIR)/serial.c $(KERNEL_DIR)/interrupts.c
C_OBJS := $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))

# The bootloader reads this many 512-byte sectors of kernel at 0x9000.
# Keep in sync with `mov al, 60` in bootloader/bootloader.asm.
KERNEL_SECTORS ?= 60

CFLAGS := -m32 -ffreestanding -nostdlib -fno-pie -fno-stack-protector \
          -fno-asynchronous-unwind-tables -Wall -Wextra -g

ifdef CRASH_DEMO
# 'make test-panic' builds a kernel that faults on purpose after boot,
# to demonstrate and verify the exception reporter.
CFLAGS += -DCRASH_DEMO
endif

.PHONY: all run test test-panic clean

all: $(BUILD_DIR)/kernel.img

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# flat binary, not an object file
$(BUILD_DIR)/boot.bin: $(BOOT_SRC) | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(BUILD_DIR)/basic_kernel.o: $(ENTRY_SRC) | $(BUILD_DIR)
	$(NASM) -f elf32 -g -F dwarf $< -o $@

$(BUILD_DIR)/isr.o: $(KERNEL_DIR)/isr.asm | $(BUILD_DIR)
	$(NASM) -f elf32 -g -F dwarf $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# basic_kernel.o must come first: its `start` label has to land on the
# 0x9000 entry address the bootloader jumps to.
$(BUILD_DIR)/kernel.elf: $(LDSCRIPT) $(BUILD_DIR)/basic_kernel.o $(BUILD_DIR)/isr.o $(C_OBJS) | $(BUILD_DIR)
	$(LD) -m elf_i386 -T $(LDSCRIPT) $(BUILD_DIR)/basic_kernel.o $(BUILD_DIR)/isr.o $(C_OBJS) -o $@

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel.elf
	$(OBJCOPY) -O binary $< $@
	@size=$$(stat -c %s $@); \
	max=$$(( $(KERNEL_SECTORS) * 512 )); \
	if [ $$size -gt $$max ]; then \
	  echo "kernel.bin is $$size bytes but the bootloader only loads $$max"; \
	  echo "(KERNEL_SECTORS=$(KERNEL_SECTORS); keep in sync with bootloader/bootloader.asm)"; \
	  exit 1; \
	fi; \
	echo "kernel.bin: $$size / $$max bytes"

$(BUILD_DIR)/kernel.img: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin | $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	dd if=$(BUILD_DIR)/boot.bin of=$@ bs=512 conv=notrunc status=none
	dd if=$(BUILD_DIR)/kernel.bin of=$@ bs=512 seek=1 conv=notrunc status=none

run: all
	$(QEMU) -drive format=raw,file=$(BUILD_DIR)/kernel.img -serial stdio

test: all
	tools/boot_test.sh $(BUILD_DIR)/kernel.img

# boot a kernel that faults on purpose and verify the report, then
# restore the normal image. Command-line variables like CROSS= are
# passed through to the sub-makes automatically.
test-panic:
	$(MAKE) --no-print-directory clean
	$(MAKE) --no-print-directory CRASH_DEMO=1 all
	tools/boot_test.sh --panic $(BUILD_DIR)/kernel.img
	$(MAKE) --no-print-directory clean
	$(MAKE) --no-print-directory all

clean:
	rm -rf $(BUILD_DIR)
