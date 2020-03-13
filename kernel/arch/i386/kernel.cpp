#include <keyboard.hpp>
#include <timer.hpp>
#include <gdt.hpp>
#include <idt.hpp>
#include <vga.hpp>

extern "C" void kernelmain() {
    gdt::initialise();
    idt::initialise();
    vga::initialise();
    kboard::initialise();
    timer::initpit(1000);

    vga::write("\n  ()-()\n.-(___)-. freelsd development kernel\n _<   >_  beep boop keeping track of time\n \\/   \\/\n\n");
}