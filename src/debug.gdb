add-symbol-file ./build/kernelfull.o 0x100000

b task_switch

target remote | qemu-system-i386 -hda ./bin/os.bin -S -gdb stdio