#include <gfx/renderer.hpp>
#include <multiboot.hpp>
#include <gfx/fonts.hpp>
#include <mem/libc.hpp>
#include <string.hpp>
#include <logger.hpp>
#include <frogs.hpp>

namespace gfx {
    size_t column, row;
    size_t cmax, rmax;
    modeinfo_t mdata;
    pixel_t colour;
    uint8_t fill;

    inline void drawpixel(size_t x, size_t y, pixel_t colours) {
        mdata.buffer[y * mdata.width + x] = colours;
    }

    inline void scroll(size_t n) {
        // Copy ahead of the buffer back into the base framebuffer, this scrolls the screen up one line. Then simply memset() the final line.
        memcpy(mdata.buffer, &mdata.buffer[n * mdata.width], mdata.pitch * (mdata.height - TEXT_SPACING_H));
        memset(&mdata.buffer[mdata.width * (mdata.height - TEXT_SPACING_H)], fill, mdata.pitch * TEXT_SPACING_H);
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

        // Column scroller (LR).
        if(column == cmax) {
            column = 0;
            row++;
        }

        // Row scroller (UD).
        if(row == rmax) {
            scroll(TEXT_SPACING_H);
            row = rmax - 1;
            column = 0;
        }
    }

    void write(const char *str) {
        if(mdata.buffer == nullptr) return;

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
        // Cache values from the multiboot framebuffer tag.
        mdata.buffer = (pixel_t*) mboot::info.fbinfo->common.framebuffer;
        mdata.height = mboot::info.fbinfo->common.height;
        mdata.width = mboot::info.fbinfo->common.width;
        mdata.bpp = mboot::info.fbinfo->common.bpp / 8;
        mdata.pitch = mboot::info.fbinfo->common.pitch;

        // Set the maximum values for columns and rows.
        cmax = mdata.width / TEXT_SPACING_W;
        rmax = mdata.height / TEXT_SPACING_H;

        // Set the default colours and write the standard frog.
        column = row = fill = 0;
        colour = 0xFFFFFFFF;
        write(stdfrog);


        // Register the framebuffer as a logging device and print some debug information.
        log::registerwriter(&writechar, false, true, true, true);
        log::trace("[render] registered %p (gfx::writechar) as a logging device.\n", &writechar);
        log::trace("[render] current resolution: %zd/%zd/%zd\n\n", mdata.width, mdata.height, mdata.bpp * 8);
    }
}
