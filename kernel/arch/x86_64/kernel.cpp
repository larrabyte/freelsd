#include <serial.hpp>
#include <stdint.h>

extern "C" void kernelmain(void) {
    serial::initialise();
    serial::printf("[kernel] hello from long mode!\n");
}
