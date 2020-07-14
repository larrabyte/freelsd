%include "kernel/arch/x86_64/macros.asm"

global bootstrap, pge64sel
extern _init, kernelmain

extern gdt64.ptr, gdt64.code, gdt64.data
extern _scrtbss, _ecrtbss
extern _skbss, _ekbss

; Multiboot 2 header definitions.
; See section .mbheader for tags.
MAGIC        equ 0xE85250D6
ARCH         equ 0x00000000
LENGTH       equ hdrend - hdrstart
CHECKSUM     equ -(MAGIC + ARCH + LENGTH)

; Bootstrap paging information and data structures.
; Should allow for an initial 1GB of address space.
PML4_VOFFSET equ ((KERNEL_VBASE >> 39) & 0x1FF) * 8
PDPT_VOFFSET equ ((KERNEL_VBASE >> 30) & 0x1FF) * 8

PML4         equ 0x0000000000000000
PDPTPHYS     equ 0x0000000000001000
PDPTVIRT     equ 0x0000000000002000
PGESIZE      equ PDPTVIRT - PML4 + 0x1000

section .data
pge64sel:
dq PML4                      ; The start address of the bootstrap paging structures.
dq PDPTVIRT                  ; The end address of the bootstrap paging structures.
dq PGESIZE                   ; The size of the bootstrap paging structures.

section .bss
align 16                     ; 16-byte alignment to conform with compiler.
resb 16384                   ; Reserve 16KB in the kernel's BSS section for the stack.
stacktop:                    ; x86 stacks grow downwards, hence we start from the top.

section .mbheader
hdrstart:
dd MAGIC                     ; Denotes the Multiboot2 magic value.
dd ARCH                      ; The specified architecture of our CPU.
dd LENGTH                    ; Length of the header, including tags.
dd CHECKSUM                  ; Checksum to verify header integrity.

align 8                      ; Multiboot2 tags must be 8-byte aligned.
fbtag.start:                 ; Start of the framebuffer tag.
dw MB2TAG_FRAMEBUFFER        ; Denotes the tag's type: framebuffer.
dw 0                         ; No flags required in an MB2TAG_FRAMEBUFFER.
dd fbtag.end - fbtag.start   ; Size of the tag in bytes.
dd 0                         ; Requested width.
dd 0                         ; Requested height.
dd 0                         ; Requested depth.
fbtag.end:

align 8                      ; Multiboot2 tags must be 8-byte aligned.
dw MB2TAG_END                ; Denotes the end of tags in the header.
dw 0                         ; No flags required in an MB2TAG_END.
dd 8                         ; Size of an end tag is always 8.
hdrend:                      ; End of the Multiboot2 header.

section .mbtext
[BITS 32]
bootstrap:
    cli                      ; Disable interrupts.
    mov esi, eax             ; Store the multiboot magic into esi.
    xor eax, eax             ; Zero out the eax register.

    mov edi, _scrtbss        ; Set edi to crtbegin.o's BSS section.
    mov ecx, _ecrtbss        ; Set ecx to the end of the BSS section.
    sub ecx, edi             ; Calculate the size of the BSS and store in ecx.
    rep stosb                ; Zero out memory in 1-byte chunks.

    mov edi, _skbss          ; Set edi to the kernel's BSS section.
    mov ecx, _ekbss          ; Set ecx to the end of the BSS section.
    sub ecx, edi             ; Calculate the size of the BSS and store in ecx.
    rep stosb                ; Repeatedly zero memory in 1-byte chunks.

    mov edi, PML4            ; Move the base paging structure's address into edi.
    mov cr3, edi             ; Move the PML4's address into cr3.
    mov ecx, 3072            ; Set ecx to the size of the PML4 and both PDPTs.
    rep stosd                ; Repeatedly zero memory in 4-byte chunks.

    mov edi, PML4            ; Move the base address of the PML4 into edi.
    mov eax, PDPTPHYS        ; Move the address of the lower PDPT into eax.
    or eax, 1 << 0           ; Enable the present flag for the PML4 entry.
    or eax, 1 << 1           ; Enable the read/write flag for the PML4 entry.
    mov dword [edi], eax     ; Point the first entry of the PML4 to the lower PDPT.

    add edi, PML4_VOFFSET    ; Add the virtual PDPT's offset in the PML4 to edi.
    mov eax, PDPTVIRT        ; Move the address of the higher PDPT into eax.
    or eax, 1 << 0           ; Enable the present flag for the PML4 entry.
    or eax, 1 << 1           ; Enable the read/write flag for the PML4 entry.
    mov dword [edi], eax     ; Point the higher entry of the PML4 to the higher PDPT.

    mov edi, PDPTPHYS        ; Set edi to the lower PDPT's address.
    mov dword [edi], 0x83    ; Set the first entry as a 1GB page, present and read/write.

    mov edi, PDPTVIRT        ; Set edi to the higher PDPT's address.
    add edi, PDPT_VOFFSET    ; Add the higher offset onto edi.
    mov dword [edi], 0x83    ; Set the entry as a 1GB page, present and read/write.

    mov eax, cr4             ; Copy contents of cr4 into eax.
    or eax, 1 << 5           ; Enable the PAE bit.
    mov cr4, eax             ; Copy contents of eax into cr4.

    mov ecx, 0xC0000080      ; Set the MSR to read (EFER).
    rdmsr                    ; Read the EFER register into eax.
    or eax, 1 << 8           ; Enable the long mode bit.
    wrmsr                    ; Write eax back into the EFER register.

    mov eax, cr0             ; Copy contents of cr0 into eax.
    or eax, 1 << 31          ; Enable the paging bit.
    or eax, 1 << 0           ; Enable the protected mode bit.
    mov cr0, eax             ; Write eax back into cr0.
    lgdt [gdt64.ptr]         ; Load the GDTR with eax's address (physical).

    ; Far jump to load our GDT and switch to long mode.
    jmp gdt64.code:longmode - KERNEL_VBASE

section .text
[BITS 64]
longmode:
    mov cx, gdt64.data       ; Move the data descriptor into cx.
    mov ss, cx               ; Set the stack segment to cx.
    mov ds, cx               ; Set the data segment to cx.
    mov es, cx               ; Set the extra segment to cx.
    mov fs, cx               ; Set the F segment to cx.
    mov gs, cx               ; Set the G segment to cx.

    mov rcx, gdt64.ptr + 2   ; Set rcx to the address of the GDT pointer value.
    mov rax, [rcx]           ; Load the GDT pointer's value into rbx.
    add rax, KERNEL_VBASE    ; Add the kernel's virtual offset to the pointer.
    mov [rcx], rax           ; Move the new pointer value back into the GDT's pointer.
    lgdt [gdt64.ptr]         ; Reload the GDTR with our new virtual address.

    mov rsp, stacktop        ; Setup the stack pointer now that paging is done.
    push qword 0             ; Push a zero 64-bit integer.
    popf                     ; Zero out RFLAGS.

    push rsi                 ; Push the multiboot magic number to the stack.
    push rbx                 ; Push the multiboot struct address onto the stack.
    call _init               ; Initialise global constructors.

    pop rsi                  ; rsi is the second argument in the x86_64 System V ABI.
    pop rdi                  ; rdi is the first argument in the x86_64 System V ABI.
    jmp kernelmain           ; Start FreeLSD.
