; Multiboot2 tag types.
MULTIBOOT2_HEADERTAG_END                  equ 0
MULTIBOOT2_HEADERTAG_INFORMATION_REQUEST  equ 1
MULTIBOOT2_HEADERTAG_ADDRESS              equ 2
MULTIBOOT2_HEADERTAG_ENTRY_ADDRESS        equ 3
MULTIBOOT2_HEADERTAG_CONSOLE_FLAGS        equ 4
MULTIBOOT2_HEADERTAG_FRAMEBUFFER          equ 5
MULTIBOOT2_HEADERTAG_MODULE_ALIGN         equ 6
MULTIBOOT2_HEADERTAG_EFI_BS               equ 7
MULTIBOOT2_HEADERTAG_ENTRY_ADDRESS_EFI32  equ 8
MULTIBOOT2_HEADERTAG_ENTRY_ADDRESS_EFI64  equ 9
MULTIBOOT2_HEADERTAG_RELOCATABLE          equ 10

; Pushes all 64-bit general registers onto the stack.
; Does not push the stack pointer, rsp.
%macro pushaq 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

; Pops all 64-bit general registers off the stack.
; Does not pop the stack pointer, rsp.
%macro popaq 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

; Defines an interrupt service routine with no error code.
; commonisr label must be defined.
%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        push 0
        push %1
        jmp commonisr
%endmacro

; Defines an interrupt service routine with an error code.
; commonisr label must be defined.
%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        push %1
        jmp commonisr
%endmacro

; Defines an interrupt request (IRQ).
; commonisr label must be defined.
%macro IRQ 2
    global irq%1
    irq%1:
        push 0
        push %2
        jmp commonisr
%endmacro
