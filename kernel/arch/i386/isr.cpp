#include <cstr.hpp>
#include <vga.hpp>
#include <isr.hpp>

extern "C" {
    void isrhandler(isr::registers_t regs) {
        char asciinum[20];

        vga::write("\n[isr] unhandled interrupt: ");
        vga::write(cstr::itoa(regs.intnum, asciinum, 10));
    }
}