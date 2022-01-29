all:
	nasm -f elf ./loop.asm -o ./build/loop.asm.o
	i686-elf-gcc -g -T ./linker.ld -o ./loop.bin -ffreestanding -O0 -nostdlib -fpic -g ./build/loop.asm.o

clean: 
	rm -rf ./build/*.o
	rm -rf ./*.bin