[bits 16]
[org 0x7c00]

; Define some constants
KERNEL_OFFSET equ 0x1000

start:
    mov [BOOT_DRIVE], dl ; BIOS stores boot drive in dl

    mov bp, 0x9000       ; Set up the stack
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call print_string

    ; In a real OS, we would load Stage 2 here
    ; call load_kernel
    
    jmp $                ; Hang

%include "src/boot/print_string.asm"

; Data
BOOT_DRIVE      db 0
MSG_REAL_MODE   db "Started in 16-bit Real Mode", 0

; Padding and magic number
times 510-($-$$) db 0
dw 0xaa55
