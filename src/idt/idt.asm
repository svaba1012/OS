section .asm


global load_idt
global int0h
global enable_interrupts
global disable_interrupts
global no_int
global int21h
global int20h

extern int0_handler
extern int0x21_handler
extern int0x20_handler
extern no_int_handler

enable_interrupts:
    push ebp
    mov ebp, esp
    sti
    pop ebp
    ret

disable_interrupts:
    push ebp
    mov ebp, esp
    cli
    pop ebp
    ret

;load idt drom adr
;extern void load_idt(struct idtr_desc* adr);
load_idt:
    push ebp
    mov ebp, esp
    mov ebx, [ebp + 8]
    lidt [ebx]
    pop ebp
    ret


;wrapper interrupt handlers that call certain function in c 
int0h:
    cli
    pushad
    call int0_handler
    popad
    sti
    iret

int21h:
    cli
    pushad
    call int0x21_handler
    popad
    sti
    iret

int20h:
    cli
    pushad
    call int0x20_handler
    popad
    sti
    iret


no_int:
    cli
    pushad
    call no_int_handler
    popad
    sti
    iret