#ifndef FREELSD_KERNEL_GFX_RENDERER
#define FREELSD_KERNEL_GFX_RENDERER

#include <stddef.h>
#include <stdint.h>

#define VESA_TEXT_HEIGHT  8
#define VESA_TEXT_WIDTH   8
#define TEXT_SPACING_H    12
#define TEXT_SPACING_W    8

namespace gfx {
    typedef struct pixel {
        // Converts an integer into pixel_t.
        pixel(uint32_t colour) {
            blue = colour >> 24;
            green = colour >> 16;
            red = colour >> 8;
            alpha = colour;
        }

        // Default constructor for a pixel_t.
        pixel(void) { alpha = red = green = blue = 0; }

        uint8_t red, green, blue, alpha;
    } __attribute__((packed)) pixel_t;

    typedef struct raster_font {
        // Converts a 64-bit integer into a bitmap_font_t.
        raster_font(uint64_t bitmap) {
            rows[0] = bitmap >> 56; rows[1] = bitmap >> 48;
            rows[2] = bitmap >> 40; rows[3] = bitmap >> 32;
            rows[4] = bitmap >> 24; rows[5] = bitmap >> 16;
            rows[6] = bitmap >>  8; rows[7] = bitmap;
        }

        uint8_t rows[8];
    } raster_font_t;

    typedef struct modeinfo {
        pixel_t *buffer;
        size_t width, height;
        size_t pitch, bpp;
    } modeinfo_t;

    extern modeinfo_t mdata;
    extern size_t column, row;
    extern pixel_t colour;

    // Draw a character on screen at (x, y) using the specified bitmap and colours.
    void drawchar(size_t x, size_t y, raster_font_t font, pixel_t colours);

    // Write a character to the framebuffer. Wrapper around drawchar().
    void writechar(const char c);

    // Write a string to the framebuffer. Wrapper around writechar().
    void write(const char *str);

    // A wrapper for printk(), uses gfx::writechar to print.
    void printf(const char *format, ...);

    // Initialise the namespace and renderer.
    void initialise(void);
}

#endif
