disk_load:
    pusha
    push dx

    mov ah, 0x02 ; BIOS read sector function
    mov al, dh   ; Read DH sectors
    mov ch, 0x00 ; Select cylinder 0
    ; dh (head) and cl (sector) are expected to be set by the caller
    ; but we need to set head to 0 and preserve dl (drive)
    mov dh, 0x00 

    int 0x13     ; BIOS interrupt

    jc disk_error ; Jump if error (carry flag set)

    pop dx
    cmp dh, al    ; if AL (sectors read) != DH (sectors expected)
    jne disk_error
    popa
    ret

disk_error:
    mov bx, DISK_ERROR_MSG
    call print_string
    jmp $

DISK_ERROR_MSG db "Disk read error!", 0
