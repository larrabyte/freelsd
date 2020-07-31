%include "kernel/arch/x86_64/macros.asm"

global readcr2, loadcr2
global readcr3, loadcr3
global kernelpml4
global readcpuid
global loadidtr

section .data
kernelpml4:
dq pml4table

section .bss
align 4096
pml4table: equ $ - KERNEL_VBASE
resb 4096

section .text
readcpuid:
    mov eax, edi                ; Move the CPUID leaf specifier into eax.
    push rbx                    ; Save rbx as it is a preserved register.
    cpuid                       ; Execute the CPUID instruction.

    mov dword [rsi + 0x0], eax  ; Move eax into the cpuid_t struct (offset 0x0).
    mov dword [rsi + 0x4], ebx  ; Move ebx into the cpuid_t struct (offset 0x4).
    mov dword [rsi + 0x8], ecx  ; Move ecx into the cpuid_t struct (offset 0x8).
    mov dword [rsi + 0xC], edx  ; Move edx into the cpuid_t struct (offset 0xC).
    pop rbx                     ; Restore rbx.
    ret                         ; Return.

loadidtr:
    lidt [rdi]                  ; Load the LDTR with the address specified in rdi.
    sti                         ; Re-enable interrupts.
    ret                         ; Return to calling function.

readcr2:
    mov rax, cr2                ; Read the value of cr2 into rax (return register).
    ret                         ; Return to the calling function.

loadcr2:
    mov cr2, rdi                ; Write the value of rdi into cr2 (1st argument register).
    ret                         ; Return to the calling function.

readcr3:
    mov rax, cr3                ; Read the value of cr3 into rax (return register).
    ret                         ; Return to the calling function.

loadcr3:
    mov cr3, rdi                ; Write the value of rdi into cr3 (1st argument register).
    ret                         ; Return to the calling function.
