#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "terminal.h"
#include "kheap.h"
#include "disk.h"
#include "paging.h"
#include "pathparser.h"
#include "disk_streamer.h"
#include "file.h"
#include "gdt.h"
#include "config.h"
#include "memory.h"
#include "tss.h"


struct paging_4gb_chunk* kernel_chunk;
struct disk_streamer* stream;

struct tss tss;
struct gdt gdt_real[MY_OS_NUM_OF_SEGMENTS];
struct gdt_structured gdt_structured[MY_OS_NUM_OF_SEGMENTS] = {
    {.base = 0x00, .limit = 0x00, .type = 0x00},                // NULL Segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9a},           // Kernel code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},            // Kernel data segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf8},              // User code segment
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},             // User data segment
    {.base = (uint32_t)&tss, .limit=sizeof(tss), .type = 0xE9}      // TSS Segment
};

char buf[512] = "I loved only you, the only one that is truly kind,I loved only you, the only one that is truly kind,I loved only you, the only one that is truly kind,I loved only you, the only one that is truly kind,I loved only you, the only one that is truly kind,I loved only you, the only one that is truly kind,I loved only you, the only one that is truly kind,I loved only you, the only one that is truly kind,I loved only you, the only one that is truly kind,I loved only you, the only one that is truly kind, only you  ";

void kernel_main(){
    //Set the terminal
    terminal_init();
    set_color(WHITE);

    //Set global descriptor table
    
    memset(gdt_real, 0x00, sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real, gdt_structured, MY_OS_NUM_OF_SEGMENTS);

    // Load the gdt
    gdt_load(gdt_real, sizeof(gdt_real));

    //set the heap and enable memory allocation
    kheap_int();
    
    //set the interrupt table 
    init_intr_table();


    // Setup the TSS
    memset(&tss, 0x00, sizeof(tss));
    tss.esp0 = 0x600000;
    tss.ss0 = DATA_SEG_SELECTOR;

    // Load the TSS
    tss_load(0x28);
    
    //find the first disk, set the filesystem and check which filesystem is on disk 0
    disk_init();
    //char buf_help[100];
    
    
    
    filesystem_init();
    fs_resolve(get_disk_by_index(0));

    //set and enable virtual memory (paging)
    kernel_chunk = set_4gb_chunk(PAGING_PAGE_WRITEABLE | PAGING_PAGE_PRESENT | PAGING_PAGE_ACCESS_FOR_ALL);
    paging_switch(paging_get_directory_from_4gb_chunk(kernel_chunk));
    enable_paging();
    
    //enable interrupts
    enable_interrupts();
    char str[40] = "Hello master at your service\n";
    print(str);
    
    return;
}

