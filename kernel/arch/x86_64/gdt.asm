%include "kernel/arch/x86_64/macros.asm"

global gdt64.data
global gdt64.code
global gdt64.ptr
global gdt64.hiptr

section .mbdata
align 8
gdt64:
    .null: equ $ - gdt64
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x00
    db 0x01
    db 0x00

    .code: equ $ - gdt64
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10011010b
    db 10101111b
    db 0x00

    .data: equ $ - gdt64
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10010010b
    db 00000000b
    db 0x00

    .ptr:
    dw $ - gdt64 - 1
    dq gdt64

    .hiptr:
    dw gdt64.ptr - gdt64 - 1
    dq gdt64 + KERNEL_VBASE
