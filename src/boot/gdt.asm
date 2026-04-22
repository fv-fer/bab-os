gdt_start:
    dq 0x0 ; null descriptor

gdt_code: ; code descriptor
    dw 0xffff    ; limit (0-15)
    dw 0x0       ; base (0-15)
    db 0x0       ; base (16-23)
    db 10011010b ; 1st flags, type flags
    db 11001111b ; 2nd flags, limit (16-19)
    db 0x0       ; base (24-31)

gdt_data: ; data descriptor
    dw 0xffff    ; limit (0-15)
    dw 0x0       ; base (0-15)
    db 0x0       ; base (16-23)
    db 10010010b ; 1st flags, type flags
    db 11001111b ; 2nd flags, limit (16-19)
    db 0x0       ; base (24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Constants for the GDT segment offset
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
