#include "../includes/idt.h"
#include "../includes/config.h"
#include "../includes/kernel.h"
#include "../includes/memory.h"

struct idt_desc intr_desc[MY_OS_INTERRUPT_NUM];
struct idtr_desc intr_table;

extern void load_idt(struct idtr_desc* adr);

void int0_handler(void){
    terminal_print_str("\nDivide by zero\n");
}

void make_interrupt(uint16_t num, uint32_t* adress){
    struct idt_desc* ptr = &intr_desc[num];
    ptr->offset_1 = (uint32_t)adress & 0x0000ffff;
    ptr->selector = CODE_SEG_SELECTOR;
    ptr->zero = 0;
    ptr->attributes = 0b11101110; //1 for used interrupt, 11 for 3 priveledge lvl, 01110 for 32 bit int gate
    ptr->offset_2 = (uint32_t)adress >> 16;
}



void init_intr_table(void){
    _memset(intr_desc, 0, sizeof(intr_desc));
    intr_table.limit = sizeof(intr_desc) - 1;
    intr_table.base = (uint32_t)intr_desc;
    make_interrupt(0, (uint32_t*)int0_handler);
    
    //more interrupts to add ...
    //Loading interrupt descriptor table
    load_idt(&intr_table);
}


