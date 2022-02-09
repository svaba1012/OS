#include "paging.h"
#include "kheap.h"
#include "config.h"
#include "status.h"

//functionality of pagging. All pages are 4KB in size.

//assembly function for loading pagging directory to control register
extern void load_paging_directory(uint32_t* directory); 

uint32_t* current_directory;    //this is global variable for storing current directory that is loaded

struct paging_4gb_chunk* set_4gb_chunk(uint8_t flags){
    //seting linear virtual adress space (every virtual adress is same as physical)
    int32_t res = 0;
    //allocate space for pagging directory
    uint32_t* directory = (uint32_t*)kzalloc(PAGING_PAGE_SIZE);
    uint32_t* page_table;
    uint32_t offset = 0;
    struct paging_4gb_chunk* page_chunk;
    if(directory == NULL){
        res = -ENOMEM;
        goto out;
    }
    for(uint32_t i = 0; i < PAGING_PAGE_NUM; i++){
        page_table = (uint32_t*)kzalloc(PAGING_PAGE_SIZE);
        if(page_table == NULL){
            res = -ENOMEM;
            goto out;
        }
        for(uint32_t j = 0; j < PAGING_PAGE_NUM; j++){
            page_table[j] = (offset + j * PAGING_PAGE_SIZE) | flags; //setting page table elements 
        }
        offset += PAGING_ONE_TABLE_MEM_COVERAGE;
        directory[i] = (uint32_t)page_table | flags | PAGING_PAGE_WRITEABLE; //set directory elements to page tables
    }
    page_chunk = kzalloc(sizeof(struct paging_4gb_chunk));
    if(page_chunk == NULL){
        //code for unallocated page_chunk
    }
    page_chunk->directory_entry = directory; //saving directoy addres in structure
    out:
    if(res < 0){    //free pagging table if error
        pagging_free_4gb_chunk(page_chunk);
    } 
    return page_chunk;
}

uint32_t* paging_get_directory_from_4gb_chunk(struct paging_4gb_chunk* chunk){
    return chunk->directory_entry;
}

void paging_switch(uint32_t* directory){
    //switch page table and load it
    load_paging_directory(directory);
    current_directory = directory;
}

int32_t physical_addr_map_to_vitual(uint32_t* directory, uint32_t virt, uint32_t phys, int32_t flags){
    //set in pagging table that virt virtual addres points to phys physical addres
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
    dir_index = virt / PAGING_ONE_TABLE_MEM_COVERAGE; //index in directory of tables
    table_index = (virt % PAGING_ONE_TABLE_MEM_COVERAGE) / PAGING_PAGE_SIZE;    //index in table
    uint32_t* table;
    table = (uint32_t*)(directory[dir_index] & 0xfffff000); //get the table addres
    //directory[dir_index] = (uint32_t)table | flags;
    table[table_index] =  phys | flags; //set new phys addres of phys
    out:
    return res;
}

void pagging_free_4gb_chunk(struct paging_4gb_chunk* chunk){
    //free allocated memory for pagging 
    uint32_t* page_dir = chunk->directory_entry;
    for(int32_t i = 0; i < PAGING_PAGE_NUM; i++){
        kfree((uint32_t*)(page_dir[i] & 0xfffff000));
    }
    kfree(page_dir);
    kfree(chunk);
}


int32_t pagging_map_to(uint32_t* directory, uint32_t virt, uint32_t phys, uint32_t phys_end, int32_t flags){
    //map physical addresses in range of phys-phys_end to virtual in linear order
    int32_t res = 0;
    //check for alignement
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
    uint32_t total_pages = total_bytes / PAGING_PAGE_SIZE;  //number of mem pages

    for(int32_t i = 0; i < total_pages; i++){
        //map all pages
        res = physical_addr_map_to_vitual(directory, virt, phys, flags);
        if(res < 0){
            goto out;
        }
        //change virt and phys to value of next page
        virt += PAGING_PAGE_SIZE;
        phys += PAGING_PAGE_SIZE;
    }
    out:

    return res;
}

uint32_t address_page_allign_end(uint32_t addr){
    //allign address to end of the page
    if(addr % PAGING_PAGE_SIZE == 0){
        return addr;
    }
    return (addr / PAGING_PAGE_SIZE + 1) * PAGING_PAGE_SIZE;
}

uint32_t address_page_allign_start(uint32_t addr){
    //allign address to start of the page
    return (addr / PAGING_PAGE_SIZE) * PAGING_PAGE_SIZE;
}

uint32_t pagging_get_flags(uint32_t* directory, uint32_t virt){
    //get flags from some virtual address 
    uint32_t flags = 0;
    uint32_t directory_index = virt / PAGING_ONE_TABLE_MEM_COVERAGE;
    flags = directory[directory_index] & 0xfff;
    return flags;
}

uint32_t pagging_get_phys_addr(uint32_t* directory, uint32_t virt){
    //get value of physical addr for virt virtual address
    uint32_t phys = 0;
    uint32_t directory_index = virt / PAGING_ONE_TABLE_MEM_COVERAGE;
    uint32_t page_index = (virt % PAGING_ONE_TABLE_MEM_COVERAGE) / PAGING_PAGE_SIZE;
    uint32_t* table = (uint32_t*)(directory[directory_index] & 0xfffff000);
    phys = table[page_index] & 0xfffff000;
    return phys;
}