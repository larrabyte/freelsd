#ifndef FREELSD_KERNEL_HWIO_HEADER
#define FREELSD_KERNEL_HWIO_HEADER

#include <stdint.h>

extern "C" {
    // Receive one byte from a hardware I/O port.
    uint8_t inportb(uint16_t port);

    // Receive two bytes from a hardware I/O port.
    uint16_t inportw(uint16_t port);

    // Receive four bytes from a hardware I/O port.
    uint32_t inportl(uint16_t port);

    // Send one byte of data to a specified hardware I/O port.
    void outportb(uint16_t port, uint8_t value);

    // Send two bytes of data to a specified hardware I/O port.
    void outportw(uint16_t port, uint16_t value);

    // Send four bytes of data to a specified hardware I/O port.
    void outportl(uint16_t port, uint32_t value);
}

#endif
