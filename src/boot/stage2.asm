[bits 16]
[org 0x7e00]

stage2_start:
    mov [BOOT_DRIVE], dl ; Save boot drive from dl

    ; Initialize segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov bx, MSG_STAGE2
    call print_string

    call load_kernel     ; Load kernel from disk
    call switch_to_pm    ; Switch to 32-bit Protected Mode
    
    jmp $                ; Never reached

%include "src/boot/print_string.asm"
%include "src/boot/disk_load.asm"
%include "src/boot/gdt.asm"
%include "src/boot/switch_to_pm.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print_string

    mov bx, 0x1000        ; KERNEL_OFFSET
    mov dh, 15            ; Read 15 sectors
    mov dl, [BOOT_DRIVE]
    mov cl, 0x03          ; Sector 3
    call disk_load
    ret

[bits 32]
BEGIN_PM:
    call 0x1000           ; Jump to the entry point of our loaded C kernel
    jmp $

; Data
BOOT_DRIVE      db 0
MSG_STAGE2      db "Entered Stage 2 (16-bit)", 0
MSG_LOAD_KERNEL db "Loading kernel...", 0
