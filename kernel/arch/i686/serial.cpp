#include <serial.hpp>
#include <hwio.hpp>
#include <cstr.hpp>

inline uint8_t transmitempty(void) {
    return inportb(SERIAL_COM1 + 5) & 0x20;
}

void serial::initialise(void) {
    outportb(SERIAL_COM1 + 1, 0x00);  // Disable serial interrupts.
    outportb(SERIAL_COM1 + 3, 0x80);  // Enable DLAB: divisor latch access bit - set baud rate divisor.
    outportb(SERIAL_COM1 + 0, 0x03);  // Set the divisor to 3 (lo byte): 38.4k baud.
    outportb(SERIAL_COM1 + 1, 0x00);  // Set the divisor to 3 (hi byte): 38.4k baud.
    outportb(SERIAL_COM1 + 3, 0x03);  // 8 bits, no parity, one stop bit.
    outportb(SERIAL_COM1 + 2, 0xC7);  // Enable FIFO and clear with a 14-byte threshold.
    outportb(SERIAL_COM1 + 4, 0x0B);  // Re-enable serial interrupts.
}

void serial::write(const char *str) {
    for(size_t i = 0; i < cstr::len(str); i++) {
        while(transmitempty() == 0);
        outportb(SERIAL_COM1, str[i]);
    }
}