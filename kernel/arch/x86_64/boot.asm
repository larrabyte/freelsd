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
    mov edi, 0x1000          ; Move the PML4's address into edi.
    mov cr3, edi             ; Move the PML4's address into cr3.
    mov eax, 0               ; Zero out the eax register.
    mov ecx, 1024            ; How many bytes to repeat for.
    rep stosd                ; memset(&pml4, 0, 4096);
    mov edi, cr3             ; Reset edi to the PML4's address.

    mov dword [edi], 0x2003  ; Point the first PML4 entry to the first PDPT.
    add edi, 0x1000          ; Set edi to the address of the first PDPT.
    mov dword [edi], 0x3003  ; Point the first PDPT to the first PDT.
    add edi, 0x1000          ; Set edi to the address of the first PDT.
    mov dword [edi], 0x4003  ; Point the first PDT to the first PT.
    add edi, 0x1000          ; Set edi to the address of the first PT.

    mov ebx, 0x3             ; Set ebx to a template page table entry.
    mov ecx, 512             ; How many page table entries to set.

.ptentry:
    mov dword [edi], ebx     ; Move our entries into the address in edi.
    add ebx, 0x1000          ; Add 0x1000 to our page table entry.
    add edi, 8               ; Move along eight bytes.
    loop .ptentry            ; Repeat until ecx is zero.

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

.loop:
    hlt
    jmp .loop
