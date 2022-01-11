#include "paging.h"
#include "kheap.h"
#include "config.h"
#include "status.h"

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

int32_t physical_addr_map_to_vitual(uint32_t* directory, uint32_t virt, uint32_t phys, int32_t flags){
    int32_t res = 0;
    uint32_t dir_index;
    uint32_t table_index;
    if(virt % PAGING_PAGE_SIZE != 0){
        res = -EINVARG;//code for unalligned adress    
        goto out;
    }
    if(phys % PAGING_PAGE_SIZE != 0){
        res = -EINVARG;//code for unalligned adress    
        goto out;    
    }
    dir_index = virt / PAGING_ONE_TABLE_MEM_COVERAGE;
    table_index = (virt % PAGING_ONE_TABLE_MEM_COVERAGE) / PAGING_PAGE_SIZE;
    uint32_t* table;
    table = (uint32_t*)(directory[dir_index] & 0xfffff000);
    directory[dir_index] = (int32_t)table | flags;
    table[table_index] = phys;
    out:
    return res;
}

void pagging_free_4gb_chunk(struct paging_4gb_chunk* chunk){
    uint32_t* page_dir = chunk->directory_entry;
    for(int32_t i = 0; i < PAGING_PAGE_NUM; i++){
        kfree((uint32_t*)(page_dir[i] & 0xfffff000));
    }
    kfree(page_dir);
    kfree(chunk);
}


int32_t pagging_map_to(uint32_t* directory, uint32_t virt, uint32_t phys, uint32_t phys_end, int32_t flags){
    int32_t res = 0;
    if(virt % PAGING_PAGE_SIZE){
        res = -EINVARG;
        goto out;
    }
    if(phys % PAGING_PAGE_SIZE){
        res = -EINVARG;
        goto out;
    }
    if(phys_end % PAGING_PAGE_SIZE){
        res = -EINVARG;
        goto out;
    }
    if(phys_end < phys){
        res = -EINVARG;
        goto out;
    }
    uint32_t total_bytes = phys_end - phys;
    uint32_t total_pages = total_bytes / PAGING_PAGE_SIZE;
    for(int32_t i = 0; i < total_pages; i++){
        res = physical_addr_map_to_vitual(directory, virt, phys, flags);
        if(res < 0){
            goto out;
        }
        virt += PAGING_PAGE_SIZE;
        phys += PAGING_PAGE_SIZE;
    }
    out:

    return res;
}

uint32_t address_page_allign_end(uint32_t addr){
    if(addr % PAGING_PAGE_SIZE){
        return addr;
    }
    return (addr / PAGING_PAGE_SIZE + 1) * PAGING_PAGE_SIZE;
}

uint32_t address_page_allign_start(uint32_t addr){
    return (addr / PAGING_PAGE_SIZE) * PAGING_PAGE_SIZE;
}