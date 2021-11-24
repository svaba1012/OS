#ifndef IDT_H
#define IDT_H
#include <stdint.h>


struct idt_desc
{
    uint16_t offset_1; //0-15 bits of interrupt handler adress
    uint16_t selector; //code segment selector pointing to gdt code seg
    uint8_t zero;   //all 0 by default
    uint8_t attributes; //types and atrributes
    uint16_t offset_2; //16-31 bits of interrupt handler adress
}__attribute__((packed));

struct  idtr_desc
{
    uint16_t limit; //size of interrupt descriptor table
    uint32_t base; //base adress of interrupt desc table
}__attribute__((packed));

void init_intr_table(void);


#endif