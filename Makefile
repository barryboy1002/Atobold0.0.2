BUILD_DIR = build
KERNEL_DIR= kernel
BOOT_FILE = bootloader/bootloader.asm 
KERNEL_FILE = $(KERNEL_DIR)/basic_kernel.asm
KERNEL_FILE_C = $(KERNEL_DIR)/kernel_main.c
VGA_FILE = $(KERNEL_DIR)/vga_text.c
LINKER = $(KERNEL_DIR)/linker.ld

CC = ~/opt/cross/bin/i686-elf-gcc
LD = ~/opt/cross/bin/i686-elf-ld
OBJCOPY = ~/opt/cross/bin/i686-elf-objcopy
.PHONY: all build clean always

all : build always 

build: always $(BOOT_FILE) $(KERNEL_FILE) $(KERNEL_FILE_C) $(VGA_FILE) $(LINKER)
		nasm -f bin $(BOOT_FILE) -o $(BUILD_DIR)/bootstrap.o
		nasm -f elf32 -g -F dwarf $(KERNEL_FILE)  -o $(BUILD_DIR)/kernel.o
		$(CC)  -m32  -ffreestanding -nostdlib -c $(KERNEL_FILE_C)  -o $(BUILD_DIR)/kernel_main.o
		$(CC) -m32 -ffreestanding -nostdlib -c $(VGA_FILE) -o $(BUILD_DIR)/vga_text.o

		$(LD) -m elf_i386 -T $(LINKER) \
			$(BUILD_DIR)/kernel.o\
			$(BUILD_DIR)/kernel_main.o \
			$(BUILD_DIR)/vga_text.o \
			-o $(BUILD_DIR)/kernel.elf

		$(OBJCOPY) -O binary \
			$(BUILD_DIR)/kernel.elf \
			$(BUILD_DIR)/kernel.bin 
		
		dd if=/dev/zero of=$(BUILD_DIR)/kernel.img \
			bs=512 \
			count=2880
		
		dd if=$(BUILD_DIR)/bootstrap.o \
			of=$(BUILD_DIR)/kernel.img \
			bs=512 \
			conv=notrunc
		dd seek=1 if=$(BUILD_DIR)/kernel.bin \
			of=$(BUILD_DIR)/kernel.img \
			bs=512 \
			conv=notrunc 
		
		qemu-system-i386 \
			-drive format=raw,file=$(BUILD_DIR)/kernel.img


always:
	mkdir -p $(BUILD_DIR)
clean:
	rm -f *.o

