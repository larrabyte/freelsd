OUTPUT_FORMAT(elf32-i386)
ENTRY(__bootstrapx86)

KERNEL_VIRTUAL_BASE = 0xC0000000;

SECTIONS {
    /* Begin putting sections at 1 MB physical. */
    . = 1M;

    .multiboot.header : {
        *(multiboot.header)
    }

    .multiboot.data : {
        *(.multiboot.data)
    }

    .multiboot.text : {
        *(multiboot.text)
    }

    /* Place everything else at 3GB onwards. */
    . += KERNEL_VIRTUAL_BASE;

    /* Code section and read-only data. */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE) {
        *(.text)
        *(.rodata*)
    }

    /* Read-write and read-only data. */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE) {
        *(.data)
    }

    /* Read-write data (uninitialized) and the stack. */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE) {
        *(COMMON)
        *(.bss)
    }

    kernelend = ALIGN(4K);
}
