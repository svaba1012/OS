section .asm

global tss_load

tss_load:
    push ebp
    mov ebp, esp
    mov ax, [ebp+8] ; TSS Segment
    ltr ax  ;load task switch seg
    pop ebp
    ret