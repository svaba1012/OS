section .asm

global ata_lba_write
global ata_lba_read

;write num_of_sec sectors starting with lba index from buf addres
;extern void ata_lba_write(uint32_t lba, uint32_t num_of_sec, void* buf)

ata_lba_write:
    push ebp
    mov ebp, esp
    pushfd
    mov eax, [ebp + 8]   ;starting sector to write to
    mov ecx, [ebp + 12]  ;number of sectors to write to
    mov edi, [ebp + 16]  ;adress from where to copy data to disk
    and eax, 0x0FFFFFFF
    push eax
    push ebx
    push ecx
    push edx
    push edi
     
    mov ebx, eax         ; Save LBA in EBX
 
    mov edx, 0x01F6      ; Port to send drive and bit 24 - 27 of LBA
    shr eax, 24          ; Get bit 24 - 27 in al
    or al, 11100000b     ; Set bit 6 in al for LBA mode
    out dx, al
 
    mov edx, 0x01F2      ; Port to send number of sectors
    mov al, cl           ; Get number of sectors from CL
    out dx, al
 
    mov edx, 0x1F3       ; Port to send bit 0 - 7 of LBA
    mov eax, ebx         ; Get LBA from EBX
    out dx, al
 
    mov edx, 0x1F4       ; Port to send bit 8 - 15 of LBA
    mov eax, ebx         ; Get LBA from EBX
    shr eax, 8           ; Get bit 8 - 15 in AL
    out dx, al
 
 
    mov edx, 0x1F5       ; Port to send bit 16 - 23 of LBA
    mov eax, ebx         ; Get LBA from EBX
    shr eax, 16          ; Get bit 16 - 23 in AL
    out dx, al
 
    mov edx, 0x1F7       ; Command port
    mov al, 0x30         ; Write with retry.
    out dx, al
 
.still_going:  in al, dx
    test al, 8           ; the sector buffer requires servicing.
    jz .still_going      ; until the sector buffer is ready.
 
    mov eax, 256         ; to read 256 words = 1 sector
    xor bx, bx
    mov bl, cl           ; write CL sectors
    mul bx
    mov ecx, eax         ; ECX is counter for OUTSW
    mov edx, 0x1F0       ; Data port, in and out
    mov esi, edi
    rep outsw            ; out
 
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    popfd
    pop ebp
    ret


;read num_of_sec sectrors starting with lba index to buf addres
;return 0 if read is successfull otherwise return 1
;extern int32_t ata_lba_read(uint32_t lba, uint32_t num_of_sec, void* buf)

ata_lba_read:
    push ebp
    mov ebp, esp
    pushfd
    mov eax, [ebp + 8]   ;starting sector to read from
    mov ecx, [ebp + 12]  ;number of sectors to read 
    mov edi, [ebp + 16]  ;adress from where to read to
    and eax, 0x0FFFFFFF
    push eax
    push ebx
    push ecx
    push edx
    push edi

again_read:
    mov ebx, eax         ; Save LBA in EBX
 
    mov edx, 0x01F6      ; Port to send drive and bit 24 - 27 of LBA
    shr eax, 24          ; Get bit 24 - 27 in al
    or al, 11100000b     ; Set bit 6 in al for LBA mode
    out dx, al
 
    mov edx, 0x01F2      ; Port to send number of sectors
    mov al, cl           ; Get number of sectors from CL
    out dx, al
 
    mov edx, 0x1F3       ; Port to send bit 0 - 7 of LBA
    mov eax, ebx         ; Get LBA from EBX
    out dx, al
 
    mov edx, 0x1F4       ; Port to send bit 8 - 15 of LBA
    mov eax, ebx         ; Get LBA from EBX
    shr eax, 8           ; Get bit 8 - 15 in AL
    out dx, al
 
 
    mov edx, 0x1F5       ; Port to send bit 16 - 23 of LBA
    mov eax, ebx         ; Get LBA from EBX
    shr eax, 16          ; Get bit 16 - 23 in AL
    out dx, al
 
again_read1:
    mov edx, 0x1F7       ; Command port
    mov al, 0x20         ; Read with retry.
    out dx, al
 
.still_going1:  in al, dx
    cmp eax, 0x50           ; error code that makes infinite loop
    je  failed_to_read      ; in that case jump over the reading
    test al, 8           ; the sector buffer requires servicing.
    jz .still_going1      ; until the sector buffer is ready.
 
    mov eax, 256         ; to write 256 words = 1 sector
    xor bx, bx
    mov bl, cl           ; write CL sectors
    mul bx
    mov ecx, eax         ; ECX is counter for OUTSW
    mov edx, 0x1F0       ; Data port, in and out
    mov esi, edi
    rep insw            ; in
 
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    popfd
    xor eax, eax ;if eax is 0 everything is fine
    pop ebp
    ret

failed_to_read:
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    popfd
    mov eax, 1 ;set error code to one
    pop ebp
    ret