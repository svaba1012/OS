#ifndef CONFIG_H
#define CONFIG_H

/*here are defined some constants needed by most of the files
*/

//offset in gdt for kernel code segment 
#define CODE_SEG_SELECTOR 0x08
//offset in gdt for kernel data segment
#define DATA_SEG_SELECTOR 0x10
//offset in gdt for user code segment
#define USER_CODE_SEG_SELECTOR 0x1b
//offset in gdt for user data segment
#define USER_DATA_SEG_SELECTOR 0x23

//max number of interruputs in this os
#define MY_OS_INTERRUPT_NUM 512

//heap constants
//can be moved to heap.h
#define MY_OS_HEAP_BLOCK_SIZE 4096
//whole heap size
#define MY_OS_HEAP_BYTES_SIZE 104857600
//address of start of the heap
#define MY_OS_KHEAP_START_ADRESS 0x01000000 //free usable memory
//address of heap table
#define MY_OS_KHEAP_TABLE_START_ADRESS 0x00007E00 //free unused block of about 480kB memory in RAM
#define MY_OS_HEAP_BLOCK_FREE 0x00
#define MY_OS_HEAP_BLOCK_TAKEN_FIRST 0xc1
#define MY_OS_HEAP_BLOCK_TAKEN_MIDLE 0x81
#define MY_OS_HEAP_BLOCK_TAKEN_ONLY 0x41
#define MY_OS_HEAP_BLOCK_TAKEN_LAST 0x01

//max file path name for the string
#define MY_OS_PATH_MAX_LEN 150

//max number of filesystems that os can support
#define MY_OS_MAX_FILESYSTEMS 12
//max number of opened files
#define MY_OS_MAX_FILE_DESCRIPTORS 512

//number of segments in global descriptor table
#define MY_OS_NUM_OF_SEGMENTS 6

//virtual address of program code
#define MY_OS_PROGRAM_VIRTUAL_ADDRESS 0x400000
//virtual address of program stack
#define MY_OS_USER_STACK_VIRTUAL_ADDRESS 0x3FF000
#define MY_OS_USER_STACK_SIZE 1024 * 16


#define MY_OS_MAX_USER_ALLOCATIONS 1024
#define MY_OS_MAX_PROCESSES 16

#endif