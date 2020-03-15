#include <multiboot.hpp>
#include <keyboard.hpp>
#include <timer.hpp>
#include <gdt.hpp>
#include <idt.hpp>
#include <vga.hpp>

extern "C" {
    void kernelmain(mb_info_t *mbd, uint32_t magic) {
        vga::initialise();
        gdt::initialise();
        idt::initialise();
        timer::initpit(1000);
        kboard::initialise();

        vga::write("\n  ()-()\n.-(___)-. freelsd development kernel\n _<   >_  beep boop keeping track of time\n \\/   \\/\n\n");
    }
}