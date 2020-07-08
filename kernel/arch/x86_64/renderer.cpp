#include <gfx/renderer.hpp>
#include <multiboot.hpp>
#include <gfx/fonts.hpp>
#include <mem/libc.hpp>
#include <string.hpp>
#include <frogs.hpp>

#include <serial.hpp>

namespace gfx {
    modeinfo_t mdata;
    size_t column, row;
    pixel_t colour;

    inline void drawpixel(size_t x, size_t y, pixel_t colours) {
        mdata.buffer[y * mdata.width + x] = colours;
    }

    inline void scroll(size_t n) {
        // Copy ahead of the buffer back into the base framebuffer, this scrolls the screen up one line. Then simply memset() the final line.
        memcpy(mdata.buffer, &mdata.buffer[n * mdata.width], mdata.pitch * (mdata.height - TEXT_SPACING_H));
        memset(&mdata.buffer[mdata.width * (mdata.height - TEXT_SPACING_H)], 0, mdata.pitch * TEXT_SPACING_H);
    }

    void drawchar(size_t x, size_t y, raster_font_t font, pixel_t colours) {
        // Iterates across the bitmap font data.
        for(size_t i = 0; i < VESA_TEXT_HEIGHT; i++) {
            for(size_t j = 0; j < VESA_TEXT_WIDTH; j++) {
                // If the bit at (x, y) is set, draw the pixel.
                if(checkbit(font.rows[i], j)) drawpixel(j + (x * TEXT_SPACING_W), i + (y * TEXT_SPACING_H), colours);
            }
        }
    }

    void writechar(const char c) {
        // If character is a newline, advance rows.
        if(c == '\n') { column = 0; row++; }

        // If character is a backspace, reverse columns.
        else if(c == '\b') {
            if(column != 0) column--;
            drawchar(column, row, blankcanvas, 0x00000000);
        }

        // Otherwise, print character to screen.
        else drawchar(column++, row, fontmap[(uint8_t) c], colour);

        // Scrolling code.
        if(column == mdata.width) row++;
        else if(row == mdata.height) {
            row = mdata.height - 1;
            scroll(TEXT_SPACING_H);
        }

        // Reset the columns if we're at max height/width.
        if(column == mdata.width || row == mdata.height) column = 0;
    }

    void write(const char *str) {
        if(mdata.buffer == NULL) return;

        // Call writechar() on each character.
        for(size_t i = 0; i < strlen(str); i++) {
            writechar(str[i]);
        }
    }

    void printf(const char *format, ...) {
        // Initialise variadic argument list.
        va_list ap; va_start(ap, format);

        // Pass on argument list to printk().
        printk(&writechar, format, ap);
        va_end(ap);
    }

    void initialise(void) {
        mdata.buffer = (pixel_t*) mboot::info.fbinfo->common.framebuffer;
        mdata.height = mboot::info.fbinfo->common.height;
        mdata.width = mboot::info.fbinfo->common.width;
        mdata.bpp = mboot::info.fbinfo->common.bpp / 8;
        mdata.pitch = mboot::info.fbinfo->common.pitch;

        colour = 0xFFFFFFFF;
        column = row = 0;
        write(stdfrog);
    }
}
