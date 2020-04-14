MBALIGN  equ  1 << 0                       ; Align loaded modules on page boundaries.
MEMINFO  equ  1 << 1                       ; Provide a memory map.
GFXMODE  equ  1 << 2                       ; Not-text mode.
FLAGS    equ  MBALIGN | MEMINFO | GFXMODE  ; The multiboot 'flag' field.
MAGIC    equ  0x1BADB002                   ; The multiboot magic number.
CHECKSUM equ -(MAGIC + FLAGS)              ; Checksum of above.

KERNEL_VBASE equ 0xC0000000            ; The kernel's virtual base.
KERNEL_PDIDX equ (KERNEL_VBASE >> 22)  ; The page directory index for the virtual base.
KERNEL_PBASE equ 0x00100000            ; The kernel's physical base.

global __bootstrapx86:function
extern kernelmain
extern kernelend
extern _init

section .multiboot.header
align 4
dd MAGIC
dd FLAGS
dd CHECKSUM
times 8 dd 0
dd 32

section .multiboot.data
align 4096
__kernelpd:
    times 4096 db 0
__kernelpt:
    times 4096 db 0
__kernelptlow:
    times 4096 db 0

section .multiboot.text
__bootstrapx86:
    cli                    ; Disable interrupts while we setup paging.
    mov esp, __stacktop    ; Setup the stack pointer (currently virtual).
    sub esp, KERNEL_VBASE  ; Adjust to a physical address.
    call __paginginitx86   ; Initialise paging.
    add esp, KERNEL_VBASE  ; Adjust back to a virtual address, now we have paging.
    mov ebp, esp           ; Set the top of the stack frame as a virtual address.

    push eax         ; Push multiboot magic number.
    push ebx         ; Push multiboot info pointer.
    call _init       ; Call global constructors.
    call kernelmain  ; Call the kernel and start FreeLSD.

.endloop:
    hlt           ; Enter an infinite loop, interrupts enabled.
    jmp .endloop  ; Stay in the infinite loop.

__paginginitx86:
    ; Save these registers, we use them.
    ; ----------------------------------
    push eax
    push ebx
    push edx
    push ecx

    ; Configure the first PDE (0-4MB of physical memory).
    ; ---------------------------------------------------
    mov eax, __kernelpd     ; Retrieve address of the kernel's page directory.
    mov ebx, __kernelptlow  ; Retreive address of the lower page table.
    or ebx, 1               ; Set the present flag, ebx is now a valid PDE.
    mov [eax], ebx          ; Set the PDE entry (address in eax) to ebx.

    ; Configure the higher PDE (3GB virtual kernel base).
    ; ---------------------------------------------------
    push eax               ; Save the address of the page directory.
    mov eax, KERNEL_VBASE  ; Get the virtual address offset.
    shr eax, 22            ; Get the 12 bits required for a PD index.
    mov ecx, 4             ; Four bytes per PD entry.
    mul ecx                ; Multiply to retrieve offset.
    mov edx, eax           ; Move into edx for later use.
    pop eax                ; Restore eax.

    push eax               ; Save the address of the page directory.
    add eax, edx           ; Add the byte offset retrieved earlier.
    mov ebx, __kernelpt    ; Retrieve address of the virtual page table.
    or ebx, 1              ; Set this page directory entry as present.
    mov [eax], ebx         ; Set the PDE entry (address in eax) to ebx.

    ; Start mapping pages (first, identity map the first 4MB of memory).
    ; ------------------------------------------------------------------
    pop ebx            ; Pop the address of the kernel's PD from the stack.
    mov ecx, 0x100000  ; Set ecx to 1MB, what we want to map.

.firstmbloop:
    mov edx, ecx       ; Identity map (edx is our physical address we want to map to).
    call __mappagex86  ; Perform the mapping in a seperate routine.
    sub ecx, 0x1000    ; Move one page down (4096 bytes).
    jnz .firstmbloop   ; If not zero, keep going.

    ; Map the kernel to the higher half (3GB onwards in virtual memory).
    ; ------------------------------------------------------------------
    mov ecx, KERNEL_PBASE  ; Move the physical address of the kernel into ecx.
    mov eax, kernelend     ; Move the virtual end of the kernel into eax.
    sub eax, KERNEL_VBASE  ; Adjust it to be the physical end.

.higherhalfloop:
    mov edx, ecx       ; Identity map again.
    call __mappagex86  ; Perform the mapping in a seperate routine.

    push ecx               ; Save the value of ecx.
    add ecx, KERNEL_VBASE  ; Map the virtual address to the same physical one.
    call __mappagex86      ; Perform the mapping in a seperate routine.
    pop ecx                ; Restore ecx.

    add ecx, 0x1000      ; Add 4096 bytes for the next page.
    cmp ecx, eax         ; Compare the kernel end address to ecx.
    jle .higherhalfloop  ; Continue until the whole kernel is mapped.

    ; Enable paging.
    ; --------------
    mov cr3, ebx        ; Set CR3 to the address of the kernel's page directory.
    mov eax, cr0        ; Get the current CR0 value.
    or eax, 0x80000000  ; Set bit 31 to enable paging.
    mov cr0, eax        ; Move eax back into CR0.

    ; Restore the saved registers from the start and return.
    ; ------------------------------------------------------
    pop ecx
    pop edx
    pop ebx
    pop eax
    ret

__mappagex86:
    ; This function maps a virtual address to a physical address.
    ; ebx: The physical address of the kernel's page directory.
    ; ecx: The virtual address we want to map.
    ; edx: The physical address to redirect the virtual address to.

    ; Save these registers for popping later.
    ; ---------------------------------------
    push eax
    push ebx
    push ecx
    push edx

    push edx  ; Push the physical address.
    push ecx  ; Push the virtual address.

    ; Find the page directory index for our virtual address.
    ; ------------------------------------------------------
    mov eax, ecx    ; Move the virtual address into eax.
    shr eax, 22     ; Shift eax by 22 bits to retrieve the PD index.
    mov ecx, 4      ; Four bytes per PDE.
    mul ecx         ; Byte offset is now calculated, saved in eax.
    add ebx, eax    ; Add the byte offset to ebx (now ebx is the address of a page table).
    mov eax, [ebx]  ; Load the PDE into eax.

    ; Check if the PDE is present and proceed accordingly.
    ; ----------------------------------------------------
    mov ecx, eax               ; Move the PDE into ecx.
    and ecx, 0x1               ; Store if bit zero is set in ecx.
    cmp ecx, 0x0               ; Is bit zero set?
    je __bootstrapx86.endloop  ; Nope, panic!

    ; We now know the PDE exists. Calculate our PTE address.
    ; ------------------------------------------------------
    and eax, 0xFFFFF000  ; Clear all flags, retrieve the address of the page table.
    pop edx              ; Retrieve the virtual address on the stack.
    push eax             ; Save the value of eax for popping.
    mov eax, edx         ; Move the virtual address into eax.
    shr eax, 12          ; Shift the address by 12 bits (PTE index).
    and eax, 0x3FF       ; AND these bits specifically to get PTE index.
    mov ecx, 4           ; Four bytes per entry.
    mul ecx              ; Multiply to receive byte offset.
    mov ebx, eax         ; Move the PTE index to ebx.
    pop eax              ; Restore eax.

    ; Configure the page table entry.
    ; -------------------------------
    pop edx         ; Retrieve the physical address to map.
    add eax, ebx    ; Add the offset to retrieve the address of the PTE.
    or edx, 0x1     ; Set the present bit into our PTE.
    mov [eax], edx  ; Insert page table entry into table (address in eax).

    ; Restore the saved registers and return.
    ; ---------------------------------------
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

section .bss
align 16
__stackbottom:
resb 65536
__stacktop:
