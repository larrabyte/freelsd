#ifndef FREELSD_KERNEL_SERIAL_HEADER
#define FREELSD_KERNEL_SERIAL_HEADER

#define SERIAL_COM1 0x3F8

namespace serial {
    // Write a character to the serial port.
    void writechar(const char c);

    // Write a string to the serial port.
    void write(const char *str);

    // A wrapper around printf(), uses &serial::writechar.
    void printf(const char *format, ...);

    // Initialise the serial port.
    void initialise(void);
}

#endif
