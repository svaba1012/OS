KERNEL_ASM = ./src/kernel.asm
BOOT_ASM = ./src/boot/boot.asm
KERNEL_OBJ = ./build/kernel.o
KER_FULL_OBJ = ./build/kernelfull.o
BOOT_BIN = ./bin/boot.bin
OS_BIN = ./bin/os.bin
KERNEL_BIN = ./bin/kernel.bin
LINKER_LD = ./src/linker.ld

all:$(BOOT_BIN) $(KERNEL_BIN)
	rm -rf $(OS_BIN)
	dd if=$(BOOT_BIN) >> $(OS_BIN)
	dd if=$(KERNEL_BIN) >> $(OS_BIN)
	dd if=/dev/zero bs=512 count=100 >> $(OS_BIN)
	

$(KERNEL_BIN): $(KERNEL_OBJ)
	i686-elf-ld -g -relocatable $(KERNEL_OBJ) -o $(KER_FULL_OBJ)
	i686-elf-gcc -T $(LINKER_LD) -o $(KERNEL_BIN) -ffreestanding -O0 -nostdlib $(KER_FULL_OBJ)  

$(BOOT_BIN): $(BOOT_ASM)
	nasm -f bin $(BOOT_ASM) -o $(BOOT_BIN)

$(KERNEL_OBJ): $(KERNEL_ASM)
	nasm -f elf -g $(KERNEL_ASM) -o $(KERNEL_OBJ)

clean:
	rm -rf ./bin/*.bin
	rm -rf ./build/*.o