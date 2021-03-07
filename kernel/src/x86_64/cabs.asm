%include "kernel/src/x86_64/macros.asm"

global readmsr, writemsr
global readcr2, writecr2
global readcr3, writecr3
global kernelpml4
global readcpuid
global readgdtr
global loadidtr

section .data
kernelpml4:
dq pml4table
gdtstorage:
times 10 db 0

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

readmsr:
    mov ecx, edi                ; Move the value of edi into ecx (1st argument register).
    rdmsr                       ; Read the MSR specified in ecx.
    shl rdx, 32                 ; Shift rdx left by 32 bits.
    or rdx, rax                 ; Set the lower bits of rdx to rax.
    mov rax, rdx                ; Move the new 64-bit value back into rax.
    ret                         ; Return to the calling function.

writemsr:
    mov ecx, edi                ; Move the value of edi into ecx (1st argument register).
    mov rdx, rsi                ; Move the value of rsi into rdx (2nd argument register).
    shr rdx, 32                 ; Shift rdx right by 32 bits.
    mov eax, esi                ; Move the lower bits of rsi into eax.
    wrmsr                       ; Write [edx:eax] into the MSR specified in ecx.
    ret                         ; Return to the calling function.

readgdtr:
    sgdt [gdtstorage]           ; Read the GDTR into the memory address of gdtstorage.
    mov rax, gdtstorage         ; Move this memory address into rax to return.
    ret                         ; Return to the calling function.

loadidtr:
    lidt [rdi]                  ; Load the LDTR with the address specified in rdi.
    sti                         ; Re-enable interrupts.
    ret                         ; Return to calling function.

readcr2:
    mov rax, cr2                ; Read the value of cr2 into rax (return register).
    ret                         ; Return to the calling function.

writecr2:
    mov cr2, rdi                ; Write the value of rdi into cr2 (1st argument register).
    ret                         ; Return to the calling function.

readcr3:
    mov rax, cr3                ; Read the value of cr3 into rax (return register).
    ret                         ; Return to the calling function.

writecr3:
    mov cr3, rdi                ; Write the value of rdi into cr3 (1st argument register).
    ret                         ; Return to the calling function.
