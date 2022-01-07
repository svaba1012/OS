#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

#define PAGING_PAGE_SIZE 4096
#define PAGING_CACHE_DISABLED       0b00010000
#define PAGING_WRITE_THROUGH        0b00001000
#define PAGING_PAGE_ACCESS_FOR_ALL  0b00000100
#define PAGING_PAGE_WRITEABLE       0b00000010
#define PAGING_PAGE_PRESENT         0b00000001
#define PAGING_ONE_TABLE_MEM_COVERAGE 0x400000
#define PAGING_PAGE_NUM 1024

struct paging_4gb_chunk{
    uint32_t* directory_entry;
};



void enable_paging();
void paging_switch(uint32_t* directory);
struct paging_4gb_chunk* set_4gb_chunk(uint8_t flags);
uint32_t* paging_get_directory_from_4gb_chunk(struct paging_4gb_chunk* chunk);
void physical_addr_map_to_vitual(uint32_t* directory, uint32_t virt, uint32_t phys);//expecting phys addr with flags
void pagging_free_4gb_chunk(struct paging_4gb_chunk* chunk);



#endif