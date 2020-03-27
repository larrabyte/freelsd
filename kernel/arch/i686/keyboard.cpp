#include <interrupts.hpp>
#include <multiboot.hpp>
#include <keyboard.hpp>
#include <gfx/gfx.hpp>
#include <serial.hpp>
#include <hwio.hpp>

namespace kboard {
    static const char uslayout_lower[128] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	'9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0 /* 29 - control */, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0/* left shift */, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0 /* Right shift */,
        '*', 0 /* alt */, ' ' /* spacebar */, 0 /* capslock */, 0 /* 59 - F1 key ... > */, 0, 0, 0, 0, 0, 0, 0, 0,
        0,	/* < ... F10 */ 0 /* numlock */, 0 /* scrollock */, 0 /* home */, 0 /* up arrow */, 0 /* pageup */,	
        '-', 0 /* left arrow */, 0, 0 /* right arrow */, '+', 0 /* end */, 0 /* down arrow */, 0 /* pagedown */,
        0 /* insert */,	0 /* delete */,	0, 0, 0, 0 /* f11 */, 0 /* f12 */, 0, /* All other keys are undefined */
    };

    static const char uslayout_upper[] = {
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
        0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0,
        '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?'
    };

    void handler(idt::regs32_t *regs) {
        uint8_t scancode = inportb(0x60);

        // Check if bit 7 is set. If not, key has been pressed.
        if(!checkbit(scancode, 7)) {
            serial::printf("[kboard] scancode (hex %p, dec %d)\n", scancode, scancode);
            gfx::writechar(uslayout_lower[scancode]);
        }
    }

    void initialise(void) {
        idt::registerhandler(IRQ1, &handler);
    }
}
