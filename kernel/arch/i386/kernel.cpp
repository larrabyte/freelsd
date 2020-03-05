#include <memory.hpp>
#include <stdint.h>
#include <cstr.hpp>
#include <gdt.hpp>
#include <vga.hpp>

extern "C" {
    void kernelmain() {
        gdt::initialise();
        vga::initialise();

        vga::write("\n  ()-()\n.-(___)-. freelsd development kernel\n _<   >_  welcome 2 larrabyte's hell\n \\/   \\/");
    }
}