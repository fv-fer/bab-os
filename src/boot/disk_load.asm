disk_load:
    pusha
    ; Save parameters
    ; bx = buffer
    ; dh = sectors
    ; dl = drive
    ; cl = start sector

    push dx ; push sectors and drive
    
    mov ah, 0x02    ; BIOS read
    mov al, dh      ; sectors
    mov ch, 0x00    ; cylinder
    mov dh, 0x00    ; head
    ; cl and dl are already set
    
    int 0x13
    jc .error
    
    pop dx
    cmp al, dh
    jne .error
    
    popa
    ret

.error:
    mov bx, DISK_ERROR_MSG
    call print_string
    jmp $

DISK_ERROR_MSG db "Disk read error!", 0
