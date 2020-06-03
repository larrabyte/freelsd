; Pushes all x86_64 general purpose registers.
; Does not push the 64-bit stack pointer, rsp.
%macro pushaq 0
    push rax
    push rcx
    push rdx
    push rbx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

; Pops all x86_64 general purpose registers.
; Does not pop the 64-bit stack pointer, rsp.
%macro popaq 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

; Specifies an interrupt service routine that doesn't have an error code.
; Must implement commonisr for the macro to function.
%macro NOOISR_ERRCODE 1
    global isr%1
    isr%1:
        push 0
        push %1
        jmp commonisr
%endmacro

; Specifies an interrupt service routine that does have an error code.
; Must implement commonisr for the macro to function.
%macro YESISR_ERRCODE 1
    global isr%1
    isr%1:
        push %1
        jmp commonisr
%endmacro

; Specifies an IRQ.
; Must implement commonisr for the macro to function.
%macro IRQ 2
global irq%1
    irq%1:
        push 0
        push %2
        jmp commonisr
%endmacro
