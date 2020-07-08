global loadcr3

loadcr3:
    mov rax, 0x000FFFFFFFFFF000  ; Load rax with the PML4 address bitmask.
    shl rdi, 12                  ; Shift rdi by 12 bits to get address in correct format.
    and rdi, rax                 ; Zero out any remaining bits using rax's bitmask.
    mov cr3, rdi                 ; Move the result into CR3.
    ret                          ; Return.
