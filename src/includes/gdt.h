#ifndef GDT_H
#define GDT_H
#include <stdint.h>

//global descriptor table functionality for setting new and loading it

//structure of one descriptor that describes one segment
struct gdt
{
    uint16_t segment;
    uint16_t base_first;
    uint8_t base;
    uint8_t access;
    uint8_t high_flags;
    uint8_t base_24_31_bits;
}__attribute__((packed));

//human readable gdt struct thats transform to struct gdt via  gdt_structured_to_gdt function
struct gdt_structured 
{
    uint32_t base;      //start address of segment
    uint32_t limit;     //size of segment
    uint8_t type;       //type of segments (flags)
};

void gdt_load(struct gdt* gdt, int size);
void gdt_structured_to_gdt(struct gdt* gdt, struct gdt_structured* structured_gdt, int total_entires);
#endif