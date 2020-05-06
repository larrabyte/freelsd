#include <gfx/renderer.hpp>
#include <gfx/bitmap.hpp>
#include <gfx/fonts.hpp>
#include <mem/libc.hpp>
#include <string.hpp>
#include <errors.hpp>

namespace gfx {
    static video_info_t vdata;
    video_info_t *data;

    pixel_t colour;
    size_t column;
    size_t row;

    inline void drawpixel(size_t x, size_t y, pixel_t colours) {
        vdata.buffer[y * vdata.pwidth + x] = colours;
    }

    inline void scroll(size_t n) {
        // Copy ahead of the buffer back into the base framebuffer, this scrolls the screen up one line. Then simply memset() the final line.
        memcpy(vdata.buffer, &vdata.buffer[n * vdata.pwidth], vdata.pitch * (vdata.pheight - TEXT_SPACING_H));
        memset(&vdata.buffer[vdata.pwidth * (vdata.pheight - TEXT_SPACING_H)], 0, vdata.pitch * TEXT_SPACING_H);
    }

    void drawbmp(const void *address, size_t x, size_t y) {
        // Cast the address into a bitmap header pointer.
        bmp_header_t *head = (bmp_header_t*) address;
        bmp_pixel24_t *bitmap = (bmp_pixel24_t*) ((char*) head + head->offset);
        pixel_t converter;

        // Just support Win32 bitmap headers for now.
        if(head->dibsize != 40) return;

        // Convert the bitmap's 24-bits into a 32-bit pixel_t for drawing.
        for(int32_t i = 0, j = 0; i < head->dibs.win32.width; i++, j = 0) {
            for(int32_t k = head->dibs.win32.height; k > 0; k--) {
                converter.red = bitmap[k * head->dibs.win32.width + i].red;
                converter.green = bitmap[k * head->dibs.win32.width + i].green;
                converter.blue = bitmap[k * head->dibs.win32.width + i].blue;
                converter.alpha = 0;

                drawpixel(x + i, y + j++, converter);
            }
        }
    }

    void drawchar(size_t x, size_t y, int index, pixel_t colours) {
        raster_font_t c = fontmap[index];

        // Iterates across the bitmap font data.
        for(size_t i = 0; i < VESA_TEXT_HEIGHT; i++) {
            for(size_t j = 0; j < VESA_TEXT_WIDTH; j++) {
                // If the bit at (x, y) is set, draw the pixel.
                if(checkbit(c.rows[i], j)) drawpixel(j + (x * TEXT_SPACING_W), i + (y * TEXT_SPACING_H), colours);
            }
        }
    }

    void writechar(const char c) {
        // If character is a newline, advance rows.
        if(c == '\n') { column = 0; row++; }

        // If character is a backspace, reverse columns.
        else if(c == '\b') {
            if(column != 0) column--;
            drawchar(column, row, 128, 0x000000);
        }

        // Otherwise, print character to screen.
        else drawchar(column++, row, c, colour);

        // Scrolling code.
        if(column == vdata.twidth) row++;
        else if(row == vdata.theight) {
            row = vdata.theight - 1;
            scroll(TEXT_SPACING_H);
        }

        // Reset the columns if we're at max height/width.
        if(column == vdata.twidth || row == vdata.theight) column = 0;
    }

    void write(const char *str) {
        if(vdata.buffer == NULL) return;

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

    void initialise(mb_info_t *mbd) {
        // Copy video mode data into a video_mode_t.
        vdata.theight = mbd->framebufferheight / TEXT_SPACING_H;
        vdata.twidth = mbd->framebufferwidth / TEXT_SPACING_W;
        vdata.pheight = mbd->framebufferheight;
        vdata.pwidth = mbd->framebufferwidth;

        // Copy the framebuffer address, pitch, bytes per pixel and assign colour.
        vdata.buffer = (pixel_t*) mbd->framebufferaddr;
        vdata.pitch = mbd->framebufferpitch;
        vdata.bpp = mbd->framebufferbpp / 8;
        colour = 0xFFFFFFFF;

        // Match address to video_mode_t.
        data = &vdata;

        // Frog test case :)
        write(stdfrog);
    }
}
