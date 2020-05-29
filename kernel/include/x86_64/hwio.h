#ifndef FREELSD_KERNEL_HWIO_HEADER
#define FREELSD_KERNEL_HWIO_HEADER

#include <stdint.h>

// Receive one byte from a hardware port.
uint8_t inportb(uint16_t port);

// Receive two bytes from a hardware port.
uint16_t inportw(uint16_t port);

// Receive four bytes from a hardware port.
uint32_t inportl(uint16_t port);

// Send a one byte hardware signal to port.
void outportb(uint16_t port, uint8_t value);

// Send a two byte hardware signal to port.
void outportw(uint16_t port, uint16_t value);

// Send a four byte hardware signal to port.
void outportl(uint16_t port, uint32_t value);

#endif
