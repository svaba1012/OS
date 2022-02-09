#include "heap.h"
#include "config.h"
#include "status.h"
#include "memory.h"
#include "terminal.h"

//heap and memory allocation functionality
//heap table elements value descibe if heap blocks in memory are taken or are free
//heap is divided in 4KB blocks, i. table element in array describes i. block in the heap 



int heap_create(struct heap* heap, void* start, void* end, struct heap_table* table){
    //create new heap
    if(((uint32_t)start % MY_OS_HEAP_BLOCK_SIZE != 0) && ((uint32_t)start % MY_OS_HEAP_BLOCK_SIZE != 0)){
        //check if the starting and end adresses are aligned
        return -EINVARG;
    }
    size_t heap_block_num = ((size_t)(end - start) / MY_OS_HEAP_BLOCK_SIZE);
    if( heap_block_num != table->total){
        return -EINVARG;
    }
    //set heap struct memory to 0
    memset(heap, 0, sizeof(struct heap));
    heap->table = table;
    heap->start_adress = start;
    //set whole heap memory area to 0
    memset(start, 0, MY_OS_HEAP_BYTES_SIZE);
    //set all heap table elements to value MY_OS_HEAP_BLOCK_FREE which represent that all heap blocks are free
    memset(table->entries, MY_OS_HEAP_BLOCK_FREE, heap_block_num);
    return 0;
}

void* heap_malloc_blocks(struct heap* heap, uint32_t num_of_blocks){
    //finds num_of_blocks blocks in a row in a heap by looking into heap table and returns pointer to start
    //of the allocated memory
    struct heap_table* table;
    table = heap->table;
    size_t table_size = table -> total;
    HEAP_BLOCK_TABLE_ENTRY* table_entry = table->entries;
    
    uint32_t free_blocks_in_row;
    uint32_t free_mem_start_block_index = -1;
    void* ptr_to_allocated_mem = NULL; //set to return value

    if(num_of_blocks == 0){
        return NULL;
    }
    for(uint32_t i = 0; i < table_size; i++){ //iterating over heap_table
        free_blocks_in_row = 0;
        while(table_entry[i] == MY_OS_HEAP_BLOCK_FREE){ //finding first minimum free blocks
            free_blocks_in_row++;
            if(free_blocks_in_row >= num_of_blocks){ //found enough free blocks in a row
                free_mem_start_block_index = i - free_blocks_in_row + 1; //index of first block in found row
                //marking blocks as taken
                if(num_of_blocks == 1){ //if there is only 1 block for allocating
                    table_entry[free_mem_start_block_index] = MY_OS_HEAP_BLOCK_TAKEN_ONLY;
                }else{ //if there are more than 1
                    table_entry[free_mem_start_block_index] = MY_OS_HEAP_BLOCK_TAKEN_FIRST; //setting first
                    uint32_t end_index = free_mem_start_block_index + num_of_blocks -1; 
                    for(uint32_t j = free_mem_start_block_index + 1; j < end_index; j++){//setting midle
                        table_entry[j] = MY_OS_HEAP_BLOCK_TAKEN_MIDLE;
                    }
                    table_entry[end_index] = MY_OS_HEAP_BLOCK_TAKEN_LAST; //setting last
                }
                ptr_to_allocated_mem = heap->start_adress + free_mem_start_block_index * MY_OS_HEAP_BLOCK_SIZE;
                //calculating ptr to allocated 
                return ptr_to_allocated_mem;
            }
            i++;
        }
    }
    return ptr_to_allocated_mem;
}

void* heap_malloc(struct heap* heap, size_t bytes){
    uint32_t num_of_allocated_blocks;
    num_of_allocated_blocks = bytes / MY_OS_HEAP_BLOCK_SIZE;
    if(bytes % MY_OS_HEAP_BLOCK_SIZE > 0){
        num_of_allocated_blocks++;
    }
    //calculating number of minimum heap blocks needed for allocation and passing it to heap_malloc_blocks func
    return heap_malloc_blocks(heap, num_of_allocated_blocks);
}

void heap_free(struct heap* heap, void* ptr){
    void* start_addr = heap->start_adress;
    struct heap_table* table = heap ->table;
    void* end_addr = start_addr + MY_OS_HEAP_BYTES_SIZE;
    HEAP_BLOCK_TABLE_ENTRY* table_entry = table->entries;
    uint32_t index_of_alloc_mem;
    if((ptr > end_addr) || (ptr < start_addr)){
        return;//nothing to free
    }
    if( (uint32_t)(ptr) % MY_OS_HEAP_BLOCK_SIZE){
        return;//addr isn't alligned
    }
    index_of_alloc_mem = (uint32_t)(ptr - start_addr) / MY_OS_HEAP_BLOCK_SIZE;//calc start block of alloc mem
    //marking those blocks as free 
    if(table_entry[index_of_alloc_mem] == MY_OS_HEAP_BLOCK_TAKEN_ONLY){//if there is only one
        table_entry[index_of_alloc_mem] = MY_OS_HEAP_BLOCK_FREE;
        return;
    }
    if(table_entry[index_of_alloc_mem] == MY_OS_HEAP_BLOCK_TAKEN_FIRST){//if there are more
        table_entry[index_of_alloc_mem++] = MY_OS_HEAP_BLOCK_FREE;
        for(; table_entry[index_of_alloc_mem] == MY_OS_HEAP_BLOCK_TAKEN_MIDLE; index_of_alloc_mem++){
            table_entry[index_of_alloc_mem] = MY_OS_HEAP_BLOCK_FREE;
        }
        if(table_entry[index_of_alloc_mem] == MY_OS_HEAP_BLOCK_TAKEN_LAST){
            table_entry[index_of_alloc_mem] = MY_OS_HEAP_BLOCK_FREE;
        }
    }
    return;
}

