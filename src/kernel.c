#include "kernel.h"
#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "terminal.h"

void kernel_main(){
    terminal_init();
    set_color(WHITE);
    init_intr_table();
    char str[40] = "Hello master at your service\n";
    enable_interrupts();
    print(str);
    return;
}

