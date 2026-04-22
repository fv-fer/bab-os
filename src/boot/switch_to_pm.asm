[bits 16]
switch_to_pm:
    cli                     ; 1. Disable interrupts
    lgdt [gdt_descriptor]   ; 2. Load our Global Descriptor Table
    mov eax, cr0
    or eax, 0x1             ; 3. Set the first bit of CR0 to 1
    mov cr0, eax
    jmp CODE_SEG:init_pm    ; 4. Far jump to 32-bit code to flush pipeline

[bits 32]
init_pm:
    mov ax, DATA_SEG        ; 5. Update segment registers
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000        ; 6. Update stack to top of free space
    mov esp, ebp

    call BEGIN_PM           ; 7. Jump back to stage1 for 32-bit tasks
