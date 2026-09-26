BUILD_DIR = build
BOOT_FILE = bootloader/bootloader.asm 
KERNEL_FILE = kernel/basic_kernel.asm 

.PHONY: all build clean always

all : build 

build: $(BOOT_FILE) $(KERNEL_FILE)
		nasm -f bin $(BOOT_FILE) -o $(BUILD_DIR)/bootstrap.o
		nasm -f bin $(KERNEL_FILE) -o $(BUILD_DIR)/kernel.o
		dd if=$(BUILD_DIR)/bootstrap.o of=$(BUILD_DIR)/kernel.img
		dd seek=1 conv=sync if=$(BUILD_DIR)/kernel.o of=$(BUILD_DIR)/kernel.img bs=512
		qemu-system-x86_64 $(BUILD_DIR)/kernel.img
						

always:
	mkdir -p $(BUILD_DIR)
clean:
	rm -f *.o

