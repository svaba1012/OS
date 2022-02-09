section .asm

global in_byte
global in_word
global out_byte
global out_word

;input byte from port number and return it
;uint8_t in_byte(uint16_t port)
in_byte:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp + 8]
    in al, dx
    pop ebp
    ret

;input word from port number and return it
;uint16_t in_word(uint16_t port)
in_word:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp + 8]
    in ax, dx
    pop ebp
    ret

;output byte to port number
;void out_byte(uint16_t port, uint8_t byte)
out_byte:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp + 8]
    mov al, [ebp + 12]
    out dx, al
    pop ebp
    ret

;output word to port number
;void out_byte(uint16_t port, uint16_t word)
out_word:
    push ebp
    mov ebp, esp
    xor eax, eax
    mov edx, [ebp + 8]
    mov eax, [ebp + 12]
    out dx, ax
    pop ebp
    ret