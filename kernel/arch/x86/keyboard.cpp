#include <gfx/renderer.hpp>
#include <interrupts.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <serial.hpp>
#include <errors.hpp>
#include <hwio.hpp>
#include <stdint.h>

namespace kboard {
    static const char uslayout_lower[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0,
        ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0,
        '+', 0, 0, 0, 0, 0,	0, 0, 0, 0, 0, 0
    };

    static const char uslayout_upper[128] = {
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0,
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0,
        ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0,
        '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    uint16_t flags = 0;

    void handler(idt::regs32_t *regs) {
        uint8_t scancode = inportb(0x60);

        switch(scancode) {
            // Regular scancode for flags.
            case 0x2A: flags |= 1 << 0; break;
            case 0x36: flags |= 1 << 1; break;
            case 0x1D: flags |= 1 << 2; break;
            case 0x38: flags |= 1 << 3; break;

            // Regular scancode + 0x80 for flags.
            case 0xAA: flags ^= 1 << 0; break;
            case 0xB6: flags ^= 1 << 1; break;
            case 0x9D: flags ^= 1 << 2; break;
            case 0xB8: flags ^= 1 << 3; break;

            // Manually initiate kernel panic with the Escape key.
            case 0x01: panic("escape pressed, user initiated crash.");

            default:
                if(!checkbit(scancode, 7)) {
                    if(checkbit(flags, 0) || checkbit(flags, 1)) gfx::writechar(uslayout_upper[scancode]);
                    else gfx::writechar(uslayout_lower[scancode]);
                }
        }
    }

    void initialise(void) {
        idt::registerhandler(IRQ1, &handler);
    }
}
