[bits 32]
global _start
extern kmain
section .text
_start:
    cld
    call kmain
.hang:
    cli
    hlt
    jmp .hang
