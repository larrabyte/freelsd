#include <interrupts.hpp>
#include <serial.hpp>
#include <stdint.h>

extern "C" void kernelmain(void) {
    serial::initialise();
    idt::initialise();
    serial::printf("[kernel] hello from long mode!\n");
}
