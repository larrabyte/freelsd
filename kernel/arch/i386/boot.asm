MBALIGN  equ  1 << 0             ; Align loaded modules on page boundaries.
MEMINFO  equ  1 << 1             ; Provide a memory map.
FLAGS    equ  MBALIGN | MEMINFO  ; The multiboot 'flag' field.
MAGIC    equ  0x1BADB002         ; The multiboot magic number.
CHECKSUM equ -(MAGIC + FLAGS)    ; Checksum of above.

extern kernelmain
extern _init
extern _fini

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
resb 16384
stack_top:

section .text
global _start:function (_start.end - _start)

_start:
    cli
    mov esp, stack_top

    call _init
    call kernelmain
    call _fini

.hang:
    hlt
    jmp .hang

.end: