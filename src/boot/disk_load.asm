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
    jc disk_load_error
    
    pop dx
    cmp al, dh
    jne disk_load_error
    
    popa
    ret

disk_load_error:
    push cs
    pop ds
    mov bx, DISK_ERROR_MSG
    call print_string
    jmp $

; =============================================================================
; disk_load_lba
; Parameters:
;   AX    = Start Sector (1-based)
;   CX    = Number of Sectors to read
;   ES:BX = Destination buffer (Physical address = ES*16 + BX)
;   DL    = Drive ID (usually 0x80)
; =============================================================================
disk_load_lba:
    pusha
    
    ; 1. CRITICAL: Force DS to 0. 
    ; Since your code uses [org 0x7e00], all labels (like 'dap') 
    ; are offsets from 0. If DS is not 0, the BIOS looks in the wrong place.
    xor si, si
    mov ds, si

    ; 2. Check for LBA Extensions (Safety Check)
    push ax
    push bx
    push cx
    mov ah, 0x41
    mov bx, 0x55AA
    int 0x13
    jc .no_extensions
    cmp bx, 0xAA55
    jne .no_extensions
    pop cx
    pop bx
    pop ax

.read_loop:
    cmp cx, 0
    je .done

    ; Calculate chunk size (Max 64 sectors to prevent segment overflow)
    mov di, 64
    cmp cx, di
    jbe .last_chunk
    jmp .do_read
.last_chunk:
    mov di, cx

.do_read:
    push di
    push ax

    ; 3. Setup Disk Address Packet (DAP)
    mov word [dap_size], 0x0010     ; Packet size (16 bytes)
    mov [dap_sectors], di           ; Number of sectors to read
    mov [dap_buffer_off], bx        ; Target Offset
    mov [dap_buffer_seg], es        ; Target Segment
    
    ; Convert 1-based sector in AX to 0-based LBA for BIOS
    dec ax
    mov [dap_lba_low], ax
    mov word [dap_lba_high], 0
    mov dword [dap_lba_upper], 0

    ; 4. Execute BIOS Read
    mov si, dap                     ; DS:SI points to the packet
    mov ah, 0x42                    ; Extended Read function
    int 0x13
    jc .error                       ; If Carry Flag is set, it's a hardware error

    pop ax
    pop di

    ; 5. Update Iterators for next chunk
    add ax, di                      ; Move LBA forward
    sub cx, di                      ; Decrease remaining count
    
    ; 6. Advance RAM pointer (ES:BX)
    ; Each sector is 512 bytes. In segment units (16 bytes), that is 32 units.
    ; Offset = di * 32. 
    mov dx, di
    shl dx, 5                       ; Multiply di by 32 (2^5)
    mov si, es
    add si, dx                      ; Add to segment register
    mov es, si
    
    jmp .read_loop

.done:
    popa
    ret

.no_extensions:
    pop cx
    pop bx
    pop ax
    mov bx, LBA_UNSUPPORTED_MSG
    call print_string
    jmp $

.error:
    ; Clean up stack before erroring
    pop ax
    pop di
    mov bx, DISK_ERROR_MSG
    call print_string
    ; Print AX hex code here if you have a print_hex function
    jmp $

; -----------------------------------------------------------------------------
; Disk Address Packet Structure
; -----------------------------------------------------------------------------
align 4
dap:
dap_size:       db 0x10
dap_reserved:   db 0x00
dap_sectors:    dw 0x0000
dap_buffer_off: dw 0x0000
dap_buffer_seg: dw 0x0000
dap_lba_low:    dw 0x0000
dap_lba_high:   dw 0x0000
dap_lba_upper:  dd 0x0000

DISK_ERROR_MSG       db "Disk Error!", 0
LBA_UNSUPPORTED_MSG  db "LBA Unsupported!", 0
