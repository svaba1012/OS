[BITS 32]

global _start
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    ;A20 line enabled
    in al, 0x92
    or al, 2
    out 0x92, al

    mov ax, CODE_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ;Tried something else
    mov bx, DATA_SEG
    mov ss, bx
    ;
    mov ebp, 0x200000
    mov esp, ebp 
    call kernel_main
    jmp $

    times 512-($ - $$) db 0