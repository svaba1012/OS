#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "terminal.h"
#include "kheap.h"

void kernel_main(){
    terminal_init();
    set_color(WHITE);
    kheap_int();
    init_intr_table();
    char str[40] = "Hello master at your service\n";
    enable_interrupts();
    print(str);
    return;
}

