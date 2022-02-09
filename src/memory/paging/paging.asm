[BITS 32]

section .asm

global enable_paging
global load_paging_directory

enable_paging:      ;switch from segment protected mode to virtual protected mode
    push ebp
    mov ebp, esp
    mov eax, cr0    ;set most significant bit in cr0 reg to enable pagging
    or eax, 0x80000000  
    mov cr0, eax
    pop ebp
    ret

load_paging_directory:  ;argument of the function is addres of pagging directory table 
    push ebp
    mov ebp, esp
    mov eax, [ebp + 8]
    mov cr3, eax        ;moving addres of pagging directory table to register cr3 load pagging directory
                        ;processor will look in cr3 to locate pagging table
    pop ebp
    ret