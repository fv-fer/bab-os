[bits 32]

; Higher-half kernel offset
KERNEL_VIRTUAL_BASE equ 0xC0000000
; Page Directory index for the higher half (0xC0000000 >> 22 = 768)
PDE_INDEX equ (KERNEL_VIRTUAL_BASE >> 22)

section .text
[extern kmain]
[global _start]

_start:
    ; At this point, we are at physical address 0x1000.
    ; Symbols are linked at 0xC0001000.
    
    ; 1. Zero the boot page directory
    ; We must use physical addresses here.
    mov edi, (boot_page_directory - KERNEL_VIRTUAL_BASE)
    xor eax, eax
    mov ecx, 1024
    rep stosd

    ; 2. Set up the first page table (0-4MB)
    mov eax, 0x000 | 0x003 ; Physical 0 | Present + RW
    mov ecx, 1024          ; 1024 entries
    mov edi, (boot_page_table - KERNEL_VIRTUAL_BASE)
.fill_pt1:
    stosd
    add eax, 4096
    loop .fill_pt1

    ; 3. Set up the second page table (4-8MB)
    ; eax is already 4MB here from the previous loop
    mov ecx, 1024
    mov edi, (boot_page_table2 - KERNEL_VIRTUAL_BASE)
.fill_pt2:
    stosd
    add eax, 4096
    loop .fill_pt2

    ; 4. Fill the boot page directory
    ; Map first 4MB identity
    mov eax, (boot_page_table - KERNEL_VIRTUAL_BASE)
    or eax, 0x003 ; Present + Read/Write
    mov [boot_page_directory - KERNEL_VIRTUAL_BASE], eax
    ; Map first 4MB to higher-half (0xC0000000)
    mov [boot_page_directory - KERNEL_VIRTUAL_BASE + PDE_INDEX * 4], eax

    ; Map second 4MB identity
    mov eax, (boot_page_table2 - KERNEL_VIRTUAL_BASE)
    or eax, 0x003 ; Present + Read/Write
    mov [boot_page_directory - KERNEL_VIRTUAL_BASE + 4], eax
    ; Map second 4MB to higher-half (0xC0004000)
    mov [boot_page_directory - KERNEL_VIRTUAL_BASE + (PDE_INDEX + 1) * 4], eax
    
    ; 5. Enable Paging
    mov eax, (boot_page_directory - KERNEL_VIRTUAL_BASE)
    mov cr3, eax
    
    mov eax, cr0
    or eax, 0x80000001    ; Enable Paging (bit 31) and Protected Mode (bit 0)
    mov cr0, eax
    
    ; 6. Jump to the higher-half kernel
    lea eax, [.higher_half]
    jmp eax

.higher_half:
    ; Now we are running in the higher half!
    ; We can safely call kmain.
    
    call kmain
    jmp $

section .bss
align 4096
boot_page_directory:
    resb 4096
boot_page_table:
    resb 4096
boot_page_table2:
    resb 4096
