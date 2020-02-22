#include "head/vga.hpp"

extern "C" {
    void kernelmain() {
        vga::initialise();
        vga::write("Hello, kernel world!\n\n> be me\n> making os\n\n> be you\n> not making os\n> pepe.png");
    }
}
