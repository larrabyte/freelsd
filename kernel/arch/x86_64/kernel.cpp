#include <interrupts.hpp>
#include <multiboot.hpp>
#include <serial.hpp>
#include <stdint.h>

extern "C" void *kernelend;

extern "C" void kernelmain(uint64_t magic, uintptr_t mbaddr) {
    serial::initialise();
    mboot::initialise(mbaddr);
    idt::initialise();

    // Check if bootloader is Multiboot2-compliant.
    if(magic != MULTIBOOT2_BOOTLOADER_MAGIC || mbaddr & 0x07) {
        serial::printf("[kernel] bootloader is not multiboot2 compliant!\n");
        serial::printf("[kernel] magic/mbi: 0x%lx/0x%lx\n", magic, mbaddr);
    }

    // Write some debugging information to serial.
    serial::printf("[kernel] hello from long mode!\n");
    serial::printf("[kernel] end-of-kernel address: 0x%lx\n", &kernelend);
}
