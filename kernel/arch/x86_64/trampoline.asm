global smpdata

section .data
smpdata:
dq smpstart
dq smpsize

section .text
[BITS 16]
smpstart: equ $
    mov eax, 0xDEADBEEF
    hlt

smpsize: equ $ - smpstart
