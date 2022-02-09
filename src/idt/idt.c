#include "idt.h"
#include "config.h"
#include "memory.h"
#include "io.h"
#include "terminal.h"

//interrupts functionality

//PIC is mapped from interrupts starting from 0x20 int number 
//so 0x20 is timer interrupt
//and 0x21 is keyboard press interrupt
//0x22 ...
//...

struct idt_desc intr_desc[MY_OS_INTERRUPT_NUM]; //table of interrupts descriptor
struct idtr_desc intr_table;


//assembly functions more precisly interrupts handler in idt.asm
extern void load_idt(struct idtr_desc* adr);
//assembly interrupts handler that are wrapper for functions written in c
extern void int0_h(void);
extern void int21_h(void);
extern void no_int(void);
extern void int20h(void);

void no_int_handler(void){
    out_byte(PIC1, PIC_EOI);    //send signal to PIC that interrupt is handled
}

void int0_handler(void){
    //handler for exception when dividing by 0
    print("\nDivide by zero\n");
    out_byte(PIC1, PIC_EOI);
}

void int0x21_handler(void){
    print("\nKey pressed\n");
    out_byte(PIC1, PIC_EOI);    //send signal to PIC that interrupt is handled
}

void int0x20_handler(void){     //timer interrupt
    //print("\nTimer interrupt\n");
    //can be used for task switching
    out_byte(PIC1, PIC_EOI);
}

void make_interrupt(uint16_t num, uint32_t* adress){
    //set interrupt num descriptor
    struct idt_desc* ptr = &intr_desc[num];
    ptr->offset_1 = (uint32_t)adress & 0x0000ffff; //lower address of interrupt handler
    ptr->selector = CODE_SEG_SELECTOR;
    ptr->zero = 0;
    ptr->attributes = 0b11101110; //1 for used interrupt, 11 for 3 priveledge lvl, 01110 for 32 bit int gate
    ptr->offset_2 = (uint32_t)adress >> 16; //higher address of interrupt handler
}

void init_intr_table(void){
    //set whole interrupt descriptor table
    memset(intr_desc, 0, sizeof(intr_desc));
    //set table structure
    intr_table.limit = sizeof(intr_desc) - 1;
    intr_table.base = (uint32_t)intr_desc;
    for(uint16_t i = 0; i < MY_OS_INTERRUPT_NUM; i++){
        //set all descriptor in table to have no handler
        make_interrupt(i, (uint32_t*)no_int);
    }

    //set some interrupts number to handler
    make_interrupt(0, (uint32_t*)int0_handler);         //set exception for dividing by 0 to interrupt number 0 
    make_interrupt(0x21, (uint32_t*)int0x21_handler);   //set keyboard pressed interrupt to interrupt number 0x21 
    make_interrupt(0x20, (uint32_t*)int20h);            //set timer inerrupt to interrupt number 0x20 
    //more interrupts to add ...

    //Loading interrupt descriptor table
    load_idt(&intr_table);  
}


