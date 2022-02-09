section .asm

global tss_load

tss_load:           ;load task switch segment 
    push ebp
    mov ebp, esp
    mov ax, [ebp+8] ; TSS Segment
    ltr ax
    pop ebp
    ret