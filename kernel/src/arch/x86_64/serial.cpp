#include <serial.hpp>
#include <string.hpp>
#include <frogs.hpp>
#include <hwio.hpp>

namespace serial {
    static inline uint8_t transmitempty(void) {
        // Ask if the data has been received yet.
        return inportb(SERIAL_COM1 + 5) & 0x20;
    }

    void writechar(const char c) {
        while(transmitempty() == 0);  // Wait until we can transmit.
        outportb(SERIAL_COM1, c);     // Write character to COM1.
    }

    void write(const char *str) {
        // Loop through each character and writechar() it.
        for(size_t i = 0; i < strlen(str); i++) writechar(str[i]);
    }

    void printf(const char *format, ...) {
        // Initialise variadic argument list.
        va_list ap; va_start(ap, format);

        // Pass on argument list to printk().
        printk(&writechar, format, ap);
        va_end(ap);
    }

    void initialise(void) {
        outportb(SERIAL_COM1 + 1, 0x00);  // Disable serial interrupts.
        outportb(SERIAL_COM1 + 3, 0x80);  // Enable DLAB: divisor latch access bit - set baud rate divisor.
        outportb(SERIAL_COM1 + 0, 0x03);  // Set the divisor to 3 (lo byte): 38.4k baud.
        outportb(SERIAL_COM1 + 1, 0x00);  // Set the divisor to 3 (hi byte): 38.4k baud.
        outportb(SERIAL_COM1 + 3, 0x03);  // 8 bits, no parity, one stop bit.
        outportb(SERIAL_COM1 + 2, 0xC7);  // Enable FIFO and clear with a 14-byte threshold.
        outportb(SERIAL_COM1 + 4, 0x0B);  // Re-enable serial interrupts.

        write(stdfrog);  // Frog test case :)
    }
}
