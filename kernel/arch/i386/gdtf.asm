global gdtflush
extern gdtptr

gdtflush:
    lgdt [gdtptr]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.gdtfinal

.gdtfinal:
    ret