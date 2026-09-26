BUILD_DIR = build
KERNEL_DIR= kernel
BOOT_FILE = bootloader/bootloader.asm 
KERNEL_FILE = $(KERNEL_DIR)/basic_kernel.asm 
LINKER = $(KERNEL_DIR)/linker.ld

.PHONY: all build clean always

all : build 

build: $(BOOT_FILE) $(KERNEL_FILE)
		nasm -f bin $(BOOT_FILE) -o $(BUILD_DIR)/bootstrap.o
		nasm -f elf32 -g -F dwarf $(KERNEL_FILE) -o $(BUILD_DIR)/kernel.o
		ld -m elf_i386 -T $(LINKER) 	$(BUILD_DIR)/kernel.o -o $(BUILD_DIR)/kernel.elf
		objcopy -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin 
		dd if=$(BUILD_DIR)/bootstrap.o of=$(BUILD_DIR)/kernel.img
		dd seek=1 conv=sync if=$(BUILD_DIR)/kernel.o of=$(BUILD_DIR)/kernel.img bs=512
		qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/kernel.img
						

always:
	mkdir -p $(BUILD_DIR)
clean:
	rm -f *.o

