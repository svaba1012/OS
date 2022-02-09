#include "heap.h"
#include "kheap.h"
#include "config.h"
#include "terminal.h"
#include "memory.h"

//functions for kernel heap

struct heap kernel_heap;    //global kernel heap struct
struct heap_table kernel_heap_table;    //global kernel heap table


void kheap_int(){
    int32_t is_error;
    //setting kernel heap table to const values
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*) MY_OS_KHEAP_TABLE_START_ADRESS;
    kernel_heap_table.total = MY_OS_HEAP_BYTES_SIZE / MY_OS_HEAP_BLOCK_SIZE;
    void* kheap_start = (void*) MY_OS_KHEAP_START_ADRESS;
    void* kheap_end = (void*) (MY_OS_KHEAP_START_ADRESS + MY_OS_HEAP_BYTES_SIZE);
    is_error = heap_create(&kernel_heap, kheap_start, kheap_end, &kernel_heap_table);//create heap for kernel
    if(is_error < 0){
        panic("kheap_int():Error in creating heap\n");
    } 
}

void* kzalloc(size_t bytes){ //kernel malloc which set memory to 0
    void* ptr = kmalloc(bytes);
    memset(ptr, 0x00, bytes);
    return ptr;
}

void* kmalloc(size_t bytes){    //kernel malloc
    return heap_malloc(&kernel_heap, bytes);
}

void kfree(void* ptr){  //kernel free
    heap_free(&kernel_heap, ptr);
}