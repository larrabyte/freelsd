#ifndef FREELSD_KERNEL_GFX_RENDERER_HEADER
#define FREELSD_KERNEL_GFX_RENDERER_HEADER

#define VESA_TEXT_HEIGHT  8
#define VESA_TEXT_WIDTH   8
#define TEXT_SPACING_H    12
#define TEXT_SPACING_W    8

#include <multiboot.hpp>
#include <stddef.h>
#include <stdint.h>

namespace gfx {
    typedef struct pixel {
        pixel(uint32_t colour) {  // Converts an integer into pixel_t.
            alpha = colour >> 24; red = colour >> 16;
            green = colour >> 8; blue = colour;
        }

        // Default constructor for a pixel_t.
        pixel(void) { alpha = 0; red = 0; green = 0; blue = 0; }

        uint8_t red, green, blue, alpha;
    } __attribute__((packed)) pixel_t;

    typedef struct bitmap_font {
        // Converts a 64-bit integer into a bitmap_font_t.
        bitmap_font(uint64_t bitmap) {
            rows[0] = bitmap >> 56; rows[1] = bitmap >> 48;
            rows[2] = bitmap >> 40; rows[3] = bitmap >> 32;
            rows[4] = bitmap >> 24; rows[5] = bitmap >> 16;
            rows[6] = bitmap >>  8; rows[7] = bitmap;
        }

        uint8_t rows[8];
    } bitmap_font_t;

    typedef struct video_info {
        pixel_t *buffer;
        size_t pwidth, pheight;
        size_t twidth, theight;
        size_t pitch, bpp;
    } video_info_t;

    // Pointer to the system-wide video_info_t struct.
    extern video_info_t *data;

    extern pixel_t colour;  // Current renderer colour.
    extern size_t column;   // Current text column.
    extern size_t row;      // Current text row.

    // Draw a character on screen.
    void drawchar(size_t x, size_t y, int index, pixel_t colours);

    // A wrapper around drawchar(). Prints a character.
    void writechar(const char c);

    // A wrapper around writechar(). Prints a string.
    void write(const char *str);

    // A wrapper around printf(), uses &gfx::writechar.
    void printf(const char *format, ...);

    // Initialise the screen renderer.
    void initialise(mb_info_t *mbd);
}

#endif
