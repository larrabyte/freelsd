#include <serial.h>
#include <stdint.h>

extern "C" void kernelmain(void) {
    serial::initialise();
    serial::printf("[kernel] hello from long mode!\n");
}