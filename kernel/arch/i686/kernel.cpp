#include <multiboot.hpp>
#include <keyboard.hpp>
#include <memory.hpp>
#include <timer.hpp>
#include <cstr.hpp>
#include <gdt.hpp>
#include <idt.hpp>
#include <vga.hpp>

extern "C" {
    void kernelmain(mb_info_t *mbd, uint32_t magic) {
        gdt::initialise();
        idt::initialise();
        timer::initpit(1000);
        kboard::initialise();

        memory::set((void*) mbd->framebufferaddr, 0xFF, 640*480*4);
        vga::write("\n  ()-()\n.-(___)-. freelsd development kernel\n _<   >_  beep boop keeping track of time\n \\/   \\/\n\n");

        if(checkbit(mbd->flags, 0)) {
            // Read number of memory blocks (1024-bytes each). Divide by 1024 to get megabytes.
            uint64_t memorypages = mbd->lowermem + mbd->uppermem;
            uint64_t megapages = memorypages / 1024;
            char numascii[20];

            vga::write("[kernel] memory available: ");
            vga::write(cstr::itoa(megapages, numascii, 10));
            vga::write("MB\n");
        }
    }
}