#ifndef CONFIG_H
#define CONFIG_H

#define CODE_SEG_SELECTOR 0x08
#define DATA_SEG_SELECTOR 0x10
#define MY_OS_INTERRUPT_NUM 512

#define MY_OS_HEAP_BLOCK_SIZE 4096
#define MY_OS_HEAP_BYTES_SIZE 104857600
#define MY_OS_KHEAP_START_ADRESS 0x01000000 //free usable memory
#define MY_OS_KHEAP_TABLE_START_ADRESS 0x00007E00 //free unused block of about 480kB memory in RAM

#define MY_OS_HEAP_BLOCK_FREE 0x00
#define MY_OS_HEAP_BLOCK_TAKEN_FIRST 0xc1
#define MY_OS_HEAP_BLOCK_TAKEN_MIDLE 0x81
#define MY_OS_HEAP_BLOCK_TAKEN_ONLY 0x41
#define MY_OS_HEAP_BLOCK_TAKEN_LAST 0x01

#define MY_OS_PATH_MAX_LEN 150



#endif