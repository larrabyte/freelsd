OUTPUT_FORMAT(elf64-x86-64)
ENTRY(bootstrap)

KERNEL_VBASE = 0xFFFFFFFF80000000;

SECTIONS {
    /* Begin putting sections at 1 MB physical. */
    . = 1M;

    /* Multiboot header and bootstrap code. */
    .multiboot : ALIGN(4K) {
        *(.mbheader)
        *(.mbtext)
        *(.mbdata)
    }

    . += KERNEL_VBASE;

    /* Global constructors/destructors. */
    .crtstuff ALIGN(4K) : AT(ADDR(.crtstuff) - KERNEL_VBASE) {
        *(.ctors)
        *(.dtors)
        *(.tm_clone_table)
        *(.eh_frame)
    }

    /* Code section and read-only data. */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VBASE) {
        *(.init)
        *(.text)
        *(.fini)
        *(.rodata*)
    }

    /* Read-write and read-only data. */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VBASE) {
        *(.data)
    }

    /* Read-write data (uninitialized) and the stack. */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VBASE) {
        startbss = . - KERNEL_VBASE;
        *(.bss)
        endbss = . - KERNEL_VBASE;
        *(COMMON)
    }

    kernelend = ALIGN(4K);
}
