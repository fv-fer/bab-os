[bits 16]
[org 0x7e00]

VBE_INFO_BLOCK equ 0x8000 ; Address to store VBE information

stage2_start:
    mov [BOOT_DRIVE], dl ; Save boot drive from dl

    ; Initialize segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov bx, MSG_STAGE2
    call print_string

    ; --- VESA/VBE Initialization ---
    ; Get VBE Mode Info for 1024x768x32bpp (Mode 0x4118)
    ; 0x4000 bit means use Linear Frame Buffer
    mov ax, 0x4F01
    mov cx, 0x4118
    mov di, VBE_INFO_BLOCK
    int 0x10
    cmp ax, 0x004F
    jne vbe_error

    ; Set VBE Mode
    mov ax, 0x4F02
    mov bx, 0x4118 | 0x4000 ; mode | LFB bit
    int 0x10
    cmp ax, 0x004F
    jne vbe_error

    call load_kernel     ; Load kernel from disk
    call switch_to_pm    ; Switch to 32-bit Protected Mode
    
    jmp $                ; Never reached

vbe_error:
    mov bx, MSG_VBE_ERROR
    call print_string
    jmp $

%include "src/boot/print_string.asm"
%include "src/boot/disk_load.asm"
%include "src/boot/gdt.asm"
%include "src/boot/switch_to_pm.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print_string

    mov bx, 0x1000        ; KERNEL_OFFSET
    mov dh, 50            ; Read 50 sectors (25KB)
    mov dl, [BOOT_DRIVE]
    mov cl, 0x04          ; Sector 4 (Stage 1=1, Stage 2=2&3)
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
MSG_VBE_ERROR   db "VBE Error!", 0
