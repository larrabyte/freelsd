global inportb
global inportw
global inportl
global outportb
global outportw
global outportl

inportb:
    mov edx, edi  ; Move the first argument into edi.
    in al, dx     ; Use IN to retrieve one byte from the I/O port.
    ret           ; Return.

inportw:
    mov edx, edi  ; Move the requested I/O port into edi.
    in ax, dx     ; Use IN to retrieve two bytes from the I/O port.
    ret           ; Return.

inportl:
    mov edx, edi  ; Move the requested I/O port into edi.
    in eax, dx    ; Use IN to retrieve four bytes from the I/O port.
    ret           ; Return.

outportb:
    mov edx, edi  ; Move the requested I/O port into edx.
    mov eax, esi  ; Move the output data into eax.
    out dx, al    ; Output one byte of eax (al) to the I/O port.
    ret           ; Return.

outportw:
    mov edx, edi  ; Move the requested I/O port into edx.
    mov eax, esi  ; Move the output data into eax.
    out dx, ax    ; Output two bytes of eax (ax) to the I/O port.
    ret           ; Return.

outportl:
    mov edx, edi  ; Move the requested I/O port into edx.
    mov eax, esi  ; Move the output data into eax.
    out dx, eax   ; Output the eax register to the I/O port.
    ret           ; Return.
