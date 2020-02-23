#include "head/memory.hpp"
#include "head/cstr.hpp"
#include "head/vga.hpp"
#include <stdint.h>

extern "C" {
    void kernelmain() {
        vga::initialise();
        int64_t counter = 0;
        char numascii[20];

        for(size_t i = 0; i < 501; i++) {
            vga::write(cstr::itoa(counter++, numascii, 10));
            vga::write("\n");
        }

        vga::write("\nHello, kernel world!\n\n> be me\n> making os\n\n> be you\n> not making os\n> pepe.png");
    }
}
