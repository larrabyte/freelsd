#include <memory.hpp>
#include <timer.hpp>
#include <stdint.h>
#include <cstr.hpp>
#include <gdt.hpp>
#include <idt.hpp>
#include <vga.hpp>

extern "C" {
    void kernelmain() {
        gdt::initialise();
        idt::initialise();
        vga::initialise();

        vga::write("\n  ()-()\n.-(___)-. freelsd development kernel\n _<   >_  welcome 2 larrabyte's hell\n \\/   \\/\n\n");
        timer::initpit(50);
    }
}