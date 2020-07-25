; Kernel address bases.
KERNEL_VBASE equ 0xFFFFFFFF80000000
KERNEL_PBASE equ 0x0000000000100000

; Multiboot2 tag types.
MB2TAG_END                  equ 0
MB2TAG_INFORMATION_REQUEST  equ 1
MB2TAG_ADDRESS              equ 2
MB2TAG_ENTRY_ADDRESS        equ 3
MB2TAG_CONSOLE_FLAGS        equ 4
MB2TAG_FRAMEBUFFER          equ 5
MB2TAG_MODULE_ALIGN         equ 6
MB2TAG_EFI_BS               equ 7
MB2TAG_ENTRY_ADDRESS_EFI32  equ 8
MB2TAG_ENTRY_ADDRESS_EFI64  equ 9
MB2TAG_RELOCATABLE          equ 10

; Broadcasts a 32-bit value to all x86 GPRs.
; Does not broadcast to the instruction pointer or accumulator (eip/eax).
%macro bcast32 1
    mov ebx, %1
    mov ecx, %1
    mov edx, %1
    mov esi, %1
    mov edi, %1
    mov ebp, %1
    mov esp, %1
%endmacro

; Pushes all 64-bit general registers onto the stack.
; Does not push the stack pointer or instruction pointer (rsp/rip).
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
; Does not pop the stack pointer or instruction pointer (rsp/rip).
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
        push qword 0
        push qword %1
        jmp commonisr
%endmacro

; Defines an interrupt service routine with an error code.
; commonisr label must be defined.
%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        push qword %1
        jmp commonisr
%endmacro

; Defines an interrupt request (IRQ).
; commonisr label must be defined.
%macro IRQ 2
    global irq%1
    irq%1:
        push qword 0
        push qword %2
        jmp commonisr
%endmacro
