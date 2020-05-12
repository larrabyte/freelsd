global gdtflush
global idtflush
global loadcr3

section .text
gdtflush:
    mov eax, [esp+4]    ; Get address of GDT from the stack.
    lgdt [eax]          ; Load the retrieved address.
    mov ax, 0x10        ; Move 0x10 into ax.
    mov ds, ax          ; Update segments to ax's value.
    mov es, ax          ; Update segments to ax's value.
    mov fs, ax          ; Update segments to ax's value.
    mov gs, ax          ; Update segments to ax's value.
    mov ss, ax          ; Update segments to ax's value.
    jmp 0x08:.gdtfinal  ; Execute far jump to update cs.

.gdtfinal:
    ret

idtflush:
    mov eax, [esp+4]  ; Get address of IDT from the stack.
    lidt [eax]        ; Load the retrieved address.
    ret               ; Return.

loadcr3:
    mov eax, [esp+4]  ; Get address of page directory from the stack.
    mov cr3, eax      ; Move its address into control register 3.
    ret               ; Return.
