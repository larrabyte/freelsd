#include <memory.hpp>
#include <hwio.hpp>
#include <cstr.hpp>
#include <gfx.hpp>

static const size_t VGA_HEIGHT = 25;
static const size_t VGA_WIDTH = 80;

namespace gfx {
    pixel_t *buffer = NULL;  // Pointer to framebuffer.
    uint32_t height = 0;      // Height of the screen.
    uint32_t width = 0;       // Width of the screen.
    uint32_t pitch = 0;       // Number of bytes to advance one line.
    uint8_t bpp = 0;          // Number of bytes per pixel.

    void drawpixel(size_t x, size_t y, pixel_t colours) {
        buffer[y * width + x] = colours;
    }

    void initialise(mb_info_t *mbd) {
        buffer = (pixel_t*) mbd->framebufferaddr;
        height = mbd->framebufferheight;
        width = mbd->framebufferwidth;
        pitch = mbd->framebufferpitch;
        bpp = mbd->framebufferpitch / 8;
    }
}

namespace vgatext {
    uint16_t *buffer = NULL;  // Pointer to video memory.
    size_t column = 0;        // Current terminal column.
    size_t row = 0;           // Current terminal row.
    uint8_t colour;           // Current terminal colours.

    static inline uint16_t entry(unsigned char uc, uint8_t colour) {
        return (uint16_t) uc | (uint16_t) colour << 8;
    }

    inline void putentryat(char c, uint8_t colour, size_t x, size_t y) {
        buffer[y * VGA_WIDTH + x] = entry(c, colour);
    }

    void setcolour(colour_t fg, colour_t bg) {
        colour = fg | bg << 4;
    }

    void scroll(size_t n) {
        // Copies ahead of the buffer back into the base pointer, effectively scrolling.
        memory::copy(buffer, buffer + VGA_WIDTH * n, VGA_WIDTH * VGA_HEIGHT * 2);
    }

    void write(const char *str) {
        // Don't try and print if VGA isn't initialised.
        if(buffer == NULL) return;

        for(size_t i = 0; i < cstr::len(str); i++) {
            // If character is a newline, advance rows.
            if(str[i] == '\n') { column = 0; row++; }

            // If character is a backspace, reverse columns.
            else if(str[i] == '\b') {
                if(column != 0) column--;
                putentryat(' ', colour, column, row);
            }

            // Otherwise, print character to screen.
            else putentryat(str[i], colour, column++, row);

            // Scrolling code.
            if(column == VGA_WIDTH) row++;
            else if(row == VGA_HEIGHT) {
                row = VGA_HEIGHT - 1;
                scroll(1);
            }

            if(column == VGA_WIDTH || row == VGA_HEIGHT) column = 0;
        }
    }

    void initialise(void) {
        // Set the default VGA colours.
        colour = VGA_LIGHT_GREY | VGA_BLACK << 4;

        // Assign the address of the VGA text buffer.
        buffer = (uint16_t*) 0xb8000;

        // Zero the buffer (clearing the screen).
        memory::set(buffer, 0, VGA_WIDTH * VGA_HEIGHT * 2);  
    }
}