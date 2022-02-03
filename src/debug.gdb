add-symbol-file ./build/kernelfull.o 0x100000

b fat16_close

target remote | qemu-system-i386 -hda ./bin/os.bin -S -gdb stdio