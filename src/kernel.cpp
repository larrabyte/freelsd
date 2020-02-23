#include "head/cstr.hpp"
#include "head/vga.hpp"
#include <stdint.h>

extern "C" {
    void kernelmain() {
        vga::initialise();
        vga::write("Hello, kernel world!\n\n> be me\n> making os\n\n> be you\n> not making os\n> pepe.png\n\n");

        for(int i = 0; i < 17; i++) {
            vga::write("This is a scrolling test.\n");
        }
    }
}
