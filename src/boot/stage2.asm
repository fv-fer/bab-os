[bits 16]
%ifidn __OUTPUT_FORMAT__, bin
    org 0x7e00
%endif

VBE_INFO_BLOCK equ 0x0500 ; Address to store VBE information
KERNEL_LOAD_SEG equ 0x1000
KERNEL_LOAD_OFF equ 0x0000
KERNEL_START_SECTOR equ 4
%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 55
%endif

INITRD_LOAD_SEG equ 0x4000
INITRD_LOAD_OFF equ 0x0000
INITRD_START_SECTOR equ 513
%ifndef INITRD_SECTORS
%define INITRD_SECTORS 16
%endif

stage2_start:
    mov [BOOT_DRIVE], dl ; Save boot drive from dl

    ; Initialize segment registers
    xor ax, ax
    mov ds, ax
    mov es, ax
    cli
    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xFFFE
    sti

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
    call load_initrd     ; Load initrd from disk
    mov bx, MSG_BEFORE_PM
    call print_string
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
    xor ax, ax
    mov ds, ax
    mov bx, MSG_LOAD_KERNEL
    call print_string

    mov ax, KERNEL_LOAD_SEG
    mov es, ax
    mov bx, KERNEL_LOAD_OFF
    mov dl, [BOOT_DRIVE]
    mov ax, KERNEL_START_SECTOR
    mov cx, KERNEL_SECTORS
    call disk_load_lba
    ret

load_initrd:
    xor ax, ax
    mov ds, ax
    mov bx, MSG_LOAD_INITRD
    call print_string

    mov ax, INITRD_LOAD_SEG
    mov es, ax
    mov bx, INITRD_LOAD_OFF
    mov dl, [BOOT_DRIVE]
    mov ax, INITRD_START_SECTOR
    mov cx, INITRD_SECTORS
    call disk_load_lba
    ret

[bits 32]
BEGIN_PM:
    call 0x10000          ; Jump to the entry point of our loaded C kernel
    jmp $

; Data
BOOT_DRIVE      db 0
MSG_STAGE2      db "Entered Stage 2 (16-bit)", 0
MSG_LOAD_KERNEL db "Loading kernel...", 0
MSG_LOAD_INITRD db "Loading initrd...", 0
MSG_BEFORE_PM   db "Switching to protected mode...", 0
MSG_VBE_ERROR   db "VBE Error!", 0
