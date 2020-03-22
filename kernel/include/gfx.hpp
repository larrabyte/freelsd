#ifndef FREELSD_KERNEL_GFX_HEADER
#define FREELSD_KERNEL_GFX_HEADER

#include <multiboot.hpp>
#include <stddef.h>
#include <stdint.h>

namespace gfx {
    extern struct pixel *buffer;  // Pointer to framebuffer.
    extern uint32_t textmaxw;     // Number of 8x8 characters that can be displayed horizontally.
    extern uint32_t textmaxh;     // Number of 8x8 characters that can be displayed vertically.
    extern uint32_t height;       // Height of the screen.
    extern uint32_t width;        // Width of the screen.
    extern uint32_t pitch;        // Number of bytes to advance one line.
    extern uint8_t bpp;           // Number of bytes per pixel.

    extern uint32_t cursorx;      // Current location of text cursor (x).
    extern uint32_t cursory;      // Current location of text cursor (y).
    extern struct pixel colour;   // Currently selected colour.

    typedef struct pixel {
        // Explicitly convert pixel_t into an integer, using the lowest 24-bits.
        operator int() const { return (red << 16) | (green << 8) | blue; }

        // Converts three 8-bit values into a pixel_t struct.
        pixel(uint8_t reds, uint8_t greens, uint8_t blues) {
            red = reds; green = greens; blue = blues;
        }

        // Converts an integer into pixel_t.
        pixel(int colour) {
            red = colour >> 16;
            green = colour >> 8;
            blue = colour;
        }

        uint8_t red, green, blue;
    } __attribute__((packed)) pixel_t;

    typedef struct bitmap_font {
        // Converts eight 8-bit values into a bitmap_font_t struct.
        bitmap_font(uint8_t r1, uint8_t r2, uint8_t r3, uint8_t r4, uint8_t r5, uint8_t r6, uint8_t r7, uint8_t r8) {
            rows[0] = r1; rows[1] = r2; rows[2] = r3; rows[3] = r4; rows[4] = r5; rows[5] = r6; rows[6] = r7; rows[7] = r8;
        }

        // Converts a 64-bit integer into a bitmap_font_t.
        bitmap_font(uint64_t bitmap) {
            rows[0] = bitmap >> 56; rows[1] = bitmap >> 48;
            rows[2] = bitmap >> 40; rows[3] = bitmap >> 32;
            rows[4] = bitmap >> 24; rows[5] = bitmap >> 16;
            rows[6] = bitmap >>  8; rows[7] = bitmap;
        }

        uint8_t rows[8];
    } __attribute__((packed)) bitmap_font_t;

    // Initialise GFX namespace values.
    void initialise(mb_info_t *mbd);

    // Draw a pixel at (x, y) with specified colours.
    void drawpixel(size_t x, size_t y, pixel_t colours);

    // Draw a character.
    void drawchar(size_t x, size_t y, int index, pixel_t colours);

    // A wrapper around drawpixel(). Keeps track of (x, y) for you :)
    void write(const char *str);
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
    void write(const char *str);
}

#endif