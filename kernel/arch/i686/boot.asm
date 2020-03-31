MBALIGN  equ  1 << 0                       ; Align loaded modules on page boundaries.
MEMINFO  equ  1 << 1                       ; Provide a memory map.
GFXMODE  equ  1 << 2                       ; Not-text mode.
FLAGS    equ  MBALIGN | MEMINFO | GFXMODE  ; The multiboot 'flag' field.
MAGIC    equ  0x1BADB002                   ; The multiboot magic number.
CHECKSUM equ -(MAGIC + FLAGS)              ; Checksum of above.

extern kernelmain
extern _init
extern _fini
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0
    dd 32

section .bss
align 16
stack_bottom:
resb 16384
stack_top:

section .text
global _start:function (_start.end - _start)

_start:
    cli                 ; Disable interrupts.
    mov esp, stack_top  ; Move the stackptr into esp.
    and esp, -16        ; Ensure stack is 16-bit aligned.
    push eax            ; Push pointer to mb_info_t.
    push ebx            ; Push the magic number.

    call _init          ; Call global constructors.
    call kernelmain     ; Call the kernel and start FreeLSD.

.hang:
    hlt                 ; Infinite loop :)
    jmp .hang           ; Infinite loop :)

.end:
