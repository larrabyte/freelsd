#include <gfx/fonts.hpp>
#include <gfx/gfx.hpp>
#include <memory.hpp>
#include <string.hpp>
#include <hwio.hpp>

namespace gfx {
    video_info_t *infoptr;
    video_info_t info;

    pixel_t colour;
    size_t column;
    size_t row;

    void drawpixel(size_t x, size_t y, pixel_t colours) {
        info.buffer[y * info.pixelwidth + x] = colours;
    }

    void drawchar(size_t x, size_t y, int index, pixel_t colours) {
        bitmap_font_t c = fontmap[index];

        // Iterates across the bitmap font data.
        for(size_t i = 0; i < VESA_TEXT_HEIGHT; i++) {
            for(size_t j = 0; j < VESA_TEXT_WIDTH; j++) {
                // If the bit at (x, y) is set, draw the pixel.
                if(checkbit(c.rows[i], j)) drawpixel(j + (x * TEXT_SPACING_W), i + (y * TEXT_SPACING_H), colours);
            }
        }
    }

    void scroll(size_t n) {
        // Copies ahead of the buffer back into the base pointer, effectively scrolling.
        memory::copy(info.buffer, info.buffer + info.pixelwidth * n, info.pixelwidth * info.pixelheight * info.bpp);
    }

    void write(const char *str) {
        if(info.buffer == NULL) return;

        for(size_t i = 0; i < strlen(str); i++) {
            // If character is a newline, advance rows.
            if(str[i] == '\n') { column = 0; row++; }

            // If character is a backspace, reverse columns.
            else if(str[i] == '\b') {
                if(column != 0) column--;
                drawchar(column, row, 128, 0x000000);
            }

            // Otherwise, print character to screen.
            else drawchar(column++, row, str[i], colour);

            // Scrolling code.
            if(column == info.textwidth) row++;
            else if(row == info.textheight) {
                row = info.textheight - 1;
                scroll(1);
            }

            if(column == info.textwidth || row == info.textheight) column = 0;
        }
    }

    void initialise(mb_info_t *mbd) {
        // Cache resolutions into a video_mode_t.
        info.textheight = mbd->framebufferwidth / VESA_TEXT_HEIGHT;
        info.textwidth = mbd->framebufferwidth / VESA_TEXT_WIDTH;
        info.pixelheight = mbd->framebufferheight;
        info.pixelwidth = mbd->framebufferwidth;

        // Cache address, pitch, bytes per pixel and assign colour.
        info.buffer = (pixel_t*) mbd->framebufferaddr;
        info.pitch = mbd->framebufferpitch;
        info.bpp = mbd->framebufferbpp;
        colour = 0xFFFFFFFF;

        // Match address to video_mode_t.
        infoptr = &info;
    }
}