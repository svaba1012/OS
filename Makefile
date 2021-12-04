BUILD_DIR = ./build/
BIN_DIR = ./bin/
SRC_DIR = ./src/
INCLUDES = -I./src/includes
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc
#KERNEL_ASM = ./src/kernel.asm
BOOT_ASM = ./src/boot/boot.asm

KERNEL_ASM_OBJ = ./build/kernel.asm.o
KER_FULL_OBJ = ./build/kernelfull.o
KERNEL_OBJ = ./build/kernel.o
#KERNEL_C = ./src/kernel.c
#IDT_C = ./src/idt/idt.c
IDT_O = ./build/idt/idt.o
#IDT_ASM = ./src/idt/idt.asm
IDT_ASM_O = ./build/idt/idt.asm.o
#MEMORY_C = ./src/memory/memory.c
MEMORY_O = ./build/memory/memory.o
#IO_ASM = ./src/io/io.asm
IO_ASM_O = ./build/io/io.asm.o
TERMINAL_O = ./build/io/terminal/terminal.o
HEAP_O = ./build/memory/heap/heap.o
KHEAP_O = ./build/memory/heap/kheap.o

BOOT_BIN = ./bin/boot.bin
OS_BIN = ./bin/os.bin
KERNEL_BIN = ./bin/kernel.bin

LINKER_LD = ./src/linker.ld



OBJ_KERNEL_FILES = $(KERNEL_ASM_OBJ) $(KERNEL_OBJ) $(IDT_ASM_O) $(IDT_O) $(MEMORY_O) $(IO_ASM_O) $(TERMINAL_O) $(HEAP_O) $(KHEAP_O)
#OBJ_KERNEL_FILES += $(wildcard ./build/*.o)
#OBJ_KERNEL_FILES += $(wildcard ./build/*/*.o)
#OBJ_KERNEL_FILES += $(wildcard ./build/*/*.asm.o)

.PHONY: all clean

all:$(BOOT_BIN) $(KERNEL_BIN)
	rm -rf $(OS_BIN)
	dd if=$(BOOT_BIN) >> $(OS_BIN)
	dd if=$(KERNEL_BIN) >> $(OS_BIN)
	dd if=/dev/zero bs=512 count=100 >> $(OS_BIN)
	

$(KERNEL_BIN): $(OBJ_KERNEL_FILES)
	i686-elf-ld -g -relocatable $^ -o $(KER_FULL_OBJ)
	i686-elf-gcc $(FLAGS) -T $(LINKER_LD) -o $@ $(KER_FULL_OBJ)  

$(BOOT_BIN): $(BOOT_ASM)
	nasm -f bin $^ -o $@

#$(KERNEL_ASM_OBJ): $(KERNEL_ASM)
#	nasm -f elf -g $^ -o $@

#$(IDT_ASM_O): $(IDT_ASM)
#	nasm -f elf -g $^ -o $@

#$(KERNEL_OBJ): $(KERNEL_C)
#i686-elf-gcc $(INCLUDES) $(FLAGS) -c $^ -o $@

#$(IDT_O): $(IDT_C)
#i686-elf-gcc $(INCLUDES) $(FLAGS) -c $^ -o $@

#$(MEMORY_O): $(MEMORY_C)
#i686-elf-gcc $(INCLUDES) $(FLAGS) -c $^ -o $@

#$(IO_ASM_O): $(IO_ASM)
#	nasm -f elf -g $^ -o $@

$(BUILD_DIR)%.o: $(SRC_DIR)%.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -c $< -o $@

$(BUILD_DIR)%.asm.o: $(SRC_DIR)%.asm
	nasm -f elf -g $< -o $@
clean:
	rm -rf ./bin/*.bin
	rm -rf ./build/*.o
	rm -rf ./build/*/*.o
	rm -rf ./build/*/*/*.o
	

