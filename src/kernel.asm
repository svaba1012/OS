[BITS 32]

global _start
extern kernel_main

CODE_SEG equ 0x08   ;index in gdt for kernel code segment
DATA_SEG equ 0x10   ;index in gdt for kernel data segment


_start:
    mov ax, CODE_SEG    ;setting segment registers
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ;Tried something else
    mov bx, DATA_SEG
    mov ss, bx
    ;
    mov eax, 0x00200000     ;init stack
    mov ebp, eax
    mov esp, ebp

    ;A20 line enabled look for link ...
    in al, 0x92
    or al, 2
    out 0x92, al

    ;init programable interupt contoler (PIC)
    cli
    mov al, 00010001b
    out 0x20, al; seting flags in pic (programmable interrupt )
    mov al, 0x20
    out 0x21, al; seting 0x20 as starting interrupt in irq in pic
    mov al, 00000001b;
    out 0x21, al; more flags
    ;sti

    call kernel_main    ;jumping to c code, kernel.c
    jmp $



times 512-($ - $$) db   ;filling sector with zeros