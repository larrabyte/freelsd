global initf
global finif

section .init
initf:
    push rbp
    mov rbp, rsp

section .fini
finif:
    push rbp
    mov rbp, rsp