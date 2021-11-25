section .asm

global in_byte
global in_word
global out_byte
global out_word

in_byte:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp + 8]
    in al, dx
    pop ebp
    ret

in_word:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp + 8]
    in ax, dx
    pop ebp
    ret

out_byte:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp + 8]
    mov al, [ebp + 12]
    out dx, al
    pop ebp
    ret

out_word:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp + 8]
    mov eax, [ebp + 12]
    out dx, ax
    pop ebp
    ret