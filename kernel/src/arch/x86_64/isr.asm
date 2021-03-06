%include "kernel/src/arch/x86_64/macros.asm"

global spuriousisr
extern isrdispatcher

section .text
commonisr:
    pushaq              ; Push all general purpose registers (bar rsp).
    mov rdi, rsp        ; Point the dispatcher to where our GPRs were just pushed.
    call isrdispatcher  ; Call the common ISR dispatcher in C++-land.

    popaq               ; Pop all general purpose registers (bar rsp).
    add rsp, 16         ; Deallocate any error codes pushed manually or automatically.
    iretq               ; Interrupt return.

spuriousisr:            ; Spurious interrupt handler.
    iretq               ; Literally do nothing, just return.

ISR_NOERRCODE 000
ISR_NOERRCODE 001
ISR_NOERRCODE 002
ISR_NOERRCODE 003
ISR_NOERRCODE 004
ISR_NOERRCODE 005
ISR_NOERRCODE 006
ISR_NOERRCODE 007
ISR_ERRCODE   008
ISR_NOERRCODE 009
ISR_ERRCODE   010
ISR_ERRCODE   011
ISR_ERRCODE   012
ISR_ERRCODE   013
ISR_ERRCODE   014
ISR_NOERRCODE 015
ISR_NOERRCODE 016
ISR_ERRCODE   017
ISR_NOERRCODE 018
ISR_NOERRCODE 019
ISR_NOERRCODE 020
ISR_ERRCODE   021
ISR_NOERRCODE 022
ISR_NOERRCODE 023
ISR_NOERRCODE 024
ISR_NOERRCODE 025
ISR_NOERRCODE 026
ISR_NOERRCODE 027
ISR_NOERRCODE 028
ISR_NOERRCODE 029
ISR_NOERRCODE 030
ISR_NOERRCODE 031

IRQ 000, 032
IRQ 001, 033
IRQ 002, 034
IRQ 003, 035
IRQ 004, 036
IRQ 005, 037
IRQ 006, 038
IRQ 007, 039
IRQ 008, 040
IRQ 009, 041
IRQ 010, 042
IRQ 011, 043
IRQ 012, 044
IRQ 013, 045
IRQ 014, 046
IRQ 015, 047
IRQ 016, 048
IRQ 017, 049
IRQ 018, 050
IRQ 019, 051
IRQ 020, 052
IRQ 021, 053
IRQ 022, 054
IRQ 023, 055
