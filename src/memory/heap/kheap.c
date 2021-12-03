#include "heap.h"
#include "kheap.h"
#include "config.h"
#include "terminal.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;


void kheap_int(){
    int32_t is_error;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*) MY_OS_KHEAP_TABLE_START_ADRESS;
    kernel_heap_table.total = MY_OS_HEAP_BYTES_SIZE / MY_OS_HEAP_BLOCK_SIZE;
    void* kheap_start = (void*) MY_OS_KHEAP_START_ADRESS;
    void* kheap_end = (void*) (MY_OS_KHEAP_START_ADRESS + MY_OS_HEAP_BYTES_SIZE);
    is_error = heap_create(&kernel_heap, kheap_start, kheap_end, &kernel_heap_table);
    if(is_error < 0){
        print("Error in creating heap\n");
        while(1){
            ;
        }
    } 
}

void* kmalloc(size_t bytes){
    return heap_malloc(&kernel_heap, bytes);
}

void kfree(void* ptr){
    heap_free(&kernel_heap, ptr);
}