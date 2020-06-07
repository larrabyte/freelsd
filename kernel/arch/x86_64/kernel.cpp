#include <interrupts.hpp>
#include <serial.hpp>
#include <stdint.h>

extern "C" void *kernelend;

extern "C" void kernelmain(uint64_t magic, void *mb_info_t) {
    serial::initialise();
    idt::initialise();

    // Write some debugging information to serial.
    serial::printf("[kernel] hello from long mode!\n");
    serial::printf("[kernel] multiboot magic/struct: 0x%lx/0x%lx\n", magic, mb_info_t);
    serial::printf("[kernel] end-of-kernel address: 0x%lx\n", &kernelend);
}
