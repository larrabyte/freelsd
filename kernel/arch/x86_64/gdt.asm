global gdt64.pointer
global gdt64.data
global gdt64.code

section .data
align 8
gdt64:
    .null: equ $ - gdt64
    dw 0xFFFF  ; Segment limit: bits 0-15.
    dw 0x0000  ; Base address: bits 0-15.
    db 0x00    ; Base address: bits 16-23.
    db 0x00    ; Access byte: bits 0-8.
    db 0x01    ; Flags byte: bits 0-8.
    db 0x00    ; Base address: bits 24-31.

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

    .pointer:
    dw $ - gdt64 - 1
    dq gdt64
