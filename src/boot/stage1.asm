[bits 16]
[org 0x7c00]

STAGE2_OFFSET equ 0x7e00

start:
    ; Initialize segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov [BOOT_DRIVE], dl ; BIOS stores boot drive in dl

    mov bp, 0x90000      ; Set up the stack at a safer location
    mov sp, bp

    mov bx, MSG_STAGE1
    call print_string

    ; Load Stage 2 from disk (sector 2)
    mov bx, STAGE2_OFFSET
    mov dh, 2            ; Read 2 sectors (Stage 2)
    mov dl, [BOOT_DRIVE]
    mov cl, 0x02         ; Sector 2
    call disk_load

    ; Jump to Stage 2
    jmp STAGE2_OFFSET

%include "src/boot/print_string.asm"
%include "src/boot/disk_load.asm"

; Data
BOOT_DRIVE db 0
MSG_STAGE1 db "Stage 1 Loaded. Jumping to Stage 2...", 0

; Padding and magic number
times 510-($-$$) db 0
dw 0xaa55
