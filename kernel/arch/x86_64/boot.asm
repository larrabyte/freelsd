BITS 32 ; 32-bit bootstrap code to jump to 64-bits.

MAGIC    equ 0xE85250D6                ; The Multiboot 2 magic number.
ARCH     equ 0                         ; Boot into 32-bit protected mode.
LENGTH   equ hdrend - hdrstart         ; The size of the Multiboot 2 header.
CHECKSUM equ -(MAGIC + ARCH + LENGTH)  ; Checksum + magic + arch = 0.

global bootstrap

section .multiboot
align 8
hdrstart:
dd MAGIC
dd ARCH
dd LENGTH
dd CHECKSUM
dd 0
dd 8
hdrend:

section .text
bootstrap:
    cli
    mov edx, 0xC001C0DE

.loop:
    hlt
    jmp .loop
