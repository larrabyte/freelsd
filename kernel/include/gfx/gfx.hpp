#ifndef FREELSD_KERNEL_GFX_HEADER
#define FREELSD_KERNEL_GFX_HEADER

#define VESA_TEXT_HEIGHT  8
#define VESA_TEXT_WIDTH   8
#define TEXT_SPACING_H    12
#define TEXT_SPACING_W    8

#include <multiboot.hpp>
#include <stddef.h>
#include <stdint.h>

namespace gfx {
    typedef struct pixel {
        // Converts a pixel_t into an integer.
        operator int() const { return (red << 24) | (green << 16) | (blue << 8) | alpha; }

        // Converts three 8-bit values into a pixel_t struct.
        pixel(uint8_t reds, uint8_t greens, uint8_t blues, uint8_t alphas) { red = reds; green = greens; blue = blues; alpha = alphas; }

        // Default constructor.
        pixel(void) { alpha = 0; red = 0; green = 0; blue = 0; }

        // Converts an integer into pixel_t.
        pixel(uint32_t colour) {
            alpha = colour >> 24;
            red = colour >> 16;
            green = colour >> 8;
            blue = colour;
        }

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
        // Default constructor.
        video_info(void) {
            buffer = NULL; pitch = 0; bpp = 0;
            pixelheight = 0; pixelwidth = 0;
            textwidth = 0; textheight = 0;
        }

        pixel_t *buffer;
        size_t pixelwidth, pixelheight;
        size_t textwidth, textheight;
        size_t pitch, bpp;
    } video_info_t;

    extern video_info_t *infoptr;
    extern video_info_t info;

    extern pixel_t colour;
    extern size_t column;
    extern size_t row;

    // Draw a pixel at (x, y) with specified colours.
    void drawpixel(size_t x, size_t y, pixel_t colours);

    // Draw a character on screen.
    void drawchar(size_t x, size_t y, int index, pixel_t colours);

    // A wrapper around drawpixel(). Keeps track of (x, y) for you :)
    void write(const char *str);

    // Initialise GFX namespace values.
    void initialise(mb_info_t *mbd);
}

#endif