#include "paging.h"
#include "kheap.h"
#include "config.h"

extern void load_paging_directory(uint32_t* directory);

uint32_t* current_directory;

struct paging_4gb_chunk* set_4gb_chunk(uint8_t flags){
    uint32_t* directory = (uint32_t*)kzalloc(PAGING_PAGE_SIZE);
    uint32_t* page_table;
    uint32_t offset = 0;
    struct paging_4gb_chunk* page_chunk;
    if(directory == NULL){
        //code for non allocated directory
        ;
    }
    for(uint32_t i = 0; i < PAGING_PAGE_NUM; i++){
        page_table = (uint32_t*)kzalloc(PAGING_PAGE_SIZE);
        if(page_table == NULL){
            ;
            //code for non allocated page table
        }
        for(uint32_t j = 0; j < PAGING_PAGE_NUM; j++){
            page_table[j] = (offset + j * PAGING_PAGE_SIZE) | flags;
        }
        offset += PAGING_ONE_TABLE_MEM_COVERAGE;
        directory[i] = (uint32_t)page_table | flags | PAGING_PAGE_WRITEABLE;
    }
    page_chunk = kzalloc(sizeof(struct paging_4gb_chunk));
    if(page_chunk == NULL){
        //code for unallocated page_chunk
    }
    page_chunk->directory_entry = directory; 
    return page_chunk;
}

uint32_t* paging_get_directory_from_4gb_chunk(struct paging_4gb_chunk* chunk){
    return chunk->directory_entry;
}

void paging_switch(uint32_t* directory){
    load_paging_directory(directory);
    current_directory = directory;
}

void physical_addr_map_to_vitual(uint32_t* directory, uint32_t virt, uint32_t phys){
    uint32_t dir_index;
    uint32_t table_index;
    if(virt % PAGING_PAGE_SIZE != 0){
        ;//code for unalligned adress    
    }
    if(phys % PAGING_PAGE_SIZE != 0){
        ;//code for unalligned adress    
    }
    dir_index = virt / PAGING_ONE_TABLE_MEM_COVERAGE;
    table_index = (virt % PAGING_ONE_TABLE_MEM_COVERAGE) / PAGING_PAGE_SIZE;
    uint32_t* table;
    table = (uint32_t*)(directory[dir_index] & 0xfffff000);
    table[table_index] = phys;
}

void pagging_free_4gb_chunk(struct paging_4gb_chunk* chunk){
    uint32_t* page_dir = chunk->directory_entry;
    for(int32_t i = 0; i < PAGING_PAGE_NUM; i++){
        kfree((uint32_t*)(page_dir[i] & 0xfffff000));
    }
    kfree(page_dir);
    kfree(chunk);
}