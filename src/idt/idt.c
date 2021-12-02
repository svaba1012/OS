#include "idt.h"
#include "config.h"
#include "memory.h"
#include "io.h"
#include "terminal.h"

struct idt_desc intr_desc[MY_OS_INTERRUPT_NUM];
struct idtr_desc intr_table;

extern void load_idt(struct idtr_desc* adr);
extern void int0_h(void);
extern void int21_h(void);
extern void no_int(void);
extern void int20h(void);

void no_int_handler(void){
    out_byte(PIC1, PIC_EOI);    
}

void int0_handler(void){
    print("\nDivide by zero\n");
    out_byte(PIC1, PIC_EOI);
}

void int0x21_handler(void){
    print("\nKey pressed\n");
    out_byte(PIC1, PIC_EOI);
}

void int0x20_handler(void){
    print("\nTimer interrupt\n");
    //can be used for task switching
    out_byte(PIC1, PIC_EOI);
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
    for(uint16_t i = 0; i < MY_OS_INTERRUPT_NUM; i++){
        make_interrupt(i, (uint32_t*)no_int);
    }

    make_interrupt(0, (uint32_t*)int0_handler);
    make_interrupt(0x21, (uint32_t*)int0x21_handler);
    make_interrupt(0x20, (uint32_t*)int20h);
    //more interrupts to add ...

    //Loading interrupt descriptor table
    load_idt(&intr_table);
}


