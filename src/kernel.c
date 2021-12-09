#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "terminal.h"
#include "kheap.h"
#include "disk.h"
#include "paging.h"
#include "pathparser.h"

struct path_root* test_root;
struct paging_4gb_chunk* kernel_chunk;

void kernel_main(){
    terminal_init();
    set_color(WHITE);
    
    kheap_int();
    
    init_intr_table();
    
    disk_init();
    

    kernel_chunk = set_4gb_chunk(PAGING_PAGE_WRITEABLE | PAGING_PAGE_PRESENT | PAGING_PAGE_ACCESS_FOR_ALL);
    paging_switch(paging_get_directory_from_4gb_chunk(kernel_chunk));
    enable_paging();
    
    enable_interrupts();
    char str[40] = "Hello master at your service\n";
    print(str);
    return;
}

