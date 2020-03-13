#include <keyboard.hpp>
#include <hwio.hpp>
#include <cstr.hpp>
#include <idt.hpp>
#include <vga.hpp>

void kbcallback(idt::registers_t regs) {
    char numascii[20];

    uint8_t scancode = inportb(0x60);
    vga::write("[key] scancode ");
    vga::write(cstr::itoa(scancode, numascii, 10));
    vga::write(" recieved\n");
}

void kboard::initialise(void) {
    idt::registerhandler(IRQ1, &kbcallback);
}