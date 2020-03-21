#ifndef FREELSD_KERNEL_GFX_HEADER
#define FREELSD_KERNEL_GFX_HEADER

#include <multiboot.hpp>
#include <stddef.h>
#include <stdint.h>

namespace gfx {
    extern struct pixel *buffer;  // Pointer to framebuffer.
    extern uint32_t height;       // Height of the screen.
    extern uint32_t width;        // Width of the screen.
    extern uint32_t pitch;        // Number of bytes to advance one line.
    extern uint8_t bpp;           // Number of bytes per pixel.

    typedef struct pixel {
        // Explicity convert pixel_t into an integer, using the lowest 24-bits.
        operator int() const { return (red << 16) | (green << 8) | blue; }

        // Converts three 8-bit values into a pixel_t struct.
        pixel(uint8_t reds, uint8_t greens, uint8_t blues) {
            red = reds;
            green = greens;
            blue = blues;
        }

        // Converts an integer into pixel_t.
        pixel(int colour) {
            red = colour >> 16;
            green = colour >> 8;
            blue = colour;
        }

        uint8_t red;
        uint8_t green;
        uint8_t blue;
    } __attribute__((packed)) pixel_t;

    // Initialise GFX namespace values.
    void initialise(mb_info_t *mbd);

    // Draw a pixel at (x, y) with specified colours.
    void drawpixel(size_t x, size_t y, pixel_t colours);
}

namespace vgatext {
    extern uint16_t *buffer;  // Pointer to video memory.
    extern uint8_t colour;    // Current terminal colours.
    extern size_t column;     // Current terminal column.
    extern size_t row;        // Current terminal row.

    typedef enum colours {
        VGA_BLACK          = 0,
        VGA_BLUE           = 1,
        VGA_GREEN          = 2,
        VGA_CYAN           = 3,
        VGA_RED            = 4,
        VGA_MAGENTA        = 5,
        VGA_BROWN          = 6,
        VGA_LIGHT_GREY     = 7,
        VGA_DARK_GREY      = 8,
        VGA_LIGHT_BLUE     = 9,
        VGA_LIGHT_GREEN    = 10,
        VGA_LIGHT_CYAN     = 11,
        VGA_LIGHT_RED      = 12,
        VGA_LIGHT_MAGENTA  = 13,
        VGA_LIGHT_BROWN    = 14,
        VGA_WHITE          = 15,
    } colour_t;

    // Scrolls the terminal by n lines.
    void scroll(size_t n);

    // Initialise VGA-compatiable text mode.
    void initialise(void);

    // Set VGA colours.
    void setcolour(colour_t fg, colour_t bg);

    // Place a character at (x, y).
    void putentryat(char c, uint8_t colour, size_t x, size_t y);

    // A wrapper around putentryat(). Keeps track of rows and columns for you :)
    void write(const char *data);
}

#endif