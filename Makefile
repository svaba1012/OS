KERNEL_ASM = ./src/kernel.asm
BOOT_ASM = ./src/boot/boot.asm
KERNEL_OBJ = ./build/kernel.o
KER_FULL_OBJ = ./build/kernelfull.o
BOOT_BIN = ./bin/boot.bin
OS_BIN = ./bin/os.bin
KERNEL_BIN = ./bin/kernel.bin
LINKER_LD = ./src/linker.ld
INCLUDES = -I./src/includes
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
KERNEL_C_OBJ = ./build/kernel.c.o
KERNEL_C = ./src/kernel.c
IDT_C = ./src/idt/idt.c
IDT_O = ./build/idt/idt.o
IDT_ASM = ./src/idt/idt.asm
IDT_ASM_O = ./build/idt/idt.asm.o
MEMORY_C = ./src/memory/memory.c
MEMORY_O = ./build/memory/memory.o

OBJ_KERNEL_FILES = $(KERNEL_OBJ) $(KERNEL_C_OBJ) $(IDT_ASM_O) $(IDT_O) $(MEMORY_O)


all:$(BOOT_BIN) $(KERNEL_BIN)
	rm -rf $(OS_BIN)
	dd if=$(BOOT_BIN) >> $(OS_BIN)
	dd if=$(KERNEL_BIN) >> $(OS_BIN)
	dd if=/dev/zero bs=512 count=100 >> $(OS_BIN)
	

$(KERNEL_BIN): $(OBJ_KERNEL_FILES)
	i686-elf-ld -g -relocatable $(OBJ_KERNEL_FILES) -o $(KER_FULL_OBJ)
	i686-elf-gcc $(FLAGS) -T $(LINKER_LD) -o $(KERNEL_BIN) -ffreestanding -O0 -nostdlib $(KER_FULL_OBJ)  

$(BOOT_BIN): $(BOOT_ASM)
	nasm -f bin $(BOOT_ASM) -o $(BOOT_BIN)

$(KERNEL_OBJ): $(KERNEL_ASM)
	nasm -f elf -g $(KERNEL_ASM) -o $(KERNEL_OBJ)

$(IDT_ASM_O): $(IDT_ASM)
	nasm -f elf -g $(IDT_ASM) -o $(IDT_ASM_O)

$(KERNEL_C_OBJ): $(KERNEL_C)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c $(KERNEL_C) -o $(KERNEL_C_OBJ)

$(IDT_O): $(IDT_C)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c $(IDT_C) -o $(IDT_O)

$(MEMORY_O): $(MEMORY_C)
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c $(MEMORY_C) -o $(MEMORY_O)

clean:
	rm -rf ./bin/*.bin
	rm -rf ./build/*.o
	rm -rf ./build/idt/*.o
	rm -rf ./build/memory/*.o