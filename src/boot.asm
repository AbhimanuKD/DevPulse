; DevPulse OS V-1 - boot sector (BIOS, 16-bit real mode -> 32-bit protected mode)
; Build with: nasm -f bin -dKSECT=<kernel sectors> boot.asm -o boot.bin
[org 0x7C00]
[bits 16]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    mov [bootdrive], dl

    ; ---- find out how much RAM we have (saved at 0x500, in KB) ----
    ; E801 reports the same values in AX/BX or, on some BIOSes, CX/DX.
    mov ax, 0xE801
    int 0x15
    jc .old
    test ax, ax
    jnz .e801
    mov ax, cx
    mov bx, dx
.e801:
    test ax, ax
    jz .old
    movzx eax, ax            ; KB between 1 MB and 16 MB
    movzx ebx, bx            ; 64 KB blocks above 16 MB
    shl ebx, 6
    add eax, ebx
    add eax, 1024            ; the first 1 MB
    jmp .store
.old:
    mov ah, 0x88
    int 0x15
    jc .unknown
    movzx eax, ax
    add eax, 1024
    jmp .store
.unknown:
    xor eax, eax             ; report unknown RAM as 0, not garbage
.store:
    mov [0x500], eax

    ; ---- load the kernel from disk to 0x10000 ----
    mov si, dap
    mov ah, 0x42
    mov dl, [bootdrive]
    int 0x13
    jnc .loaded

    ; ---- fallback for CD/floppy boot: read one sector at a time (CHS, 18 per track, 2 heads) ----
    xor ax, ax
    mov dl, [bootdrive]
    int 0x13
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov di, KSECT
    mov cx, 0x0002           ; cylinder 0, sector 2
    xor dh, dh               ; head 0
.chs:
    mov ax, 0x0201           ; read 1 sector
    mov dl, [bootdrive]
    int 0x13
    jc disk_error
    add bx, 512
    inc cl
    cmp cl, 19
    jne .next
    mov cl, 1
    inc dh
    cmp dh, 2
    jne .next
    xor dh, dh
    inc ch
.next:
    dec di
    jnz .chs
    xor ax, ax
    mov es, ax
.loaded:

    ; ---- go to 32-bit protected mode ----
    cli
    in al, 0x92              ; fast A20 gate
    or al, 2
    out 0x92, al
    lgdt [gdt_ptr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm_start

disk_error:
    mov si, msg_err
.p: lodsb
    test al, al
    jz .h
    mov ah, 0x0E
    int 0x10
    jmp .p
.h: hlt
    jmp .h

[bits 32]
pm_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    jmp 0x08:0x10000

[bits 16]
bootdrive: db 0
msg_err:   db "DevPulse OS: disk read error", 0

align 4
dap:
    db 0x10, 0
    dw KSECT
    dw 0x0000, 0x1000        ; offset:segment = 0x1000:0000 = 0x10000
    dq 1                     ; start at sector 1 (right after this one)

align 8
gdt:
    dq 0
    dq 0x00CF9A000000FFFF    ; code, 4 GB, ring 0
    dq 0x00CF92000000FFFF    ; data, 4 GB, ring 0
gdt_ptr:
    dw gdt_ptr - gdt - 1
    dd gdt

times 510 - ($ - $$) db 0
dw 0xAA55
