#include <memory.hpp>
#include <hwio.hpp>
#include <cstr.hpp>
#include <gfx.hpp>

static const size_t VGA_HEIGHT = 25;
static const size_t VGA_WIDTH = 80;

namespace gfx {
    pixel_t *buffer = NULL;  // Pointer to framebuffer.
    uint32_t textmaxw = 0;   // Number of 8x8 characters that can be displayed horizontally.
    uint32_t textmaxh = 0;   // Number of 8x8 characters that can be displayed vertically.
    uint32_t height = 0;     // Height of the screen.
    uint32_t width = 0;      // Width of the screen.
    uint32_t pitch = 0;      // Number of bytes to advance one line.
    uint8_t bpp = 0;         // Number of bytes per pixel.

    uint32_t cursorx = 0;    // Current location of text cursor (x).
    uint32_t cursory = 0;    // Current location of text cursor (y).
    pixel_t colour = 0x0;    // Currently selected colour.

    uint64_t fontmap[129] = {
        0x0200000000000000UL,  // U+000 (nul)

        0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, // U+001 -> U+0020 (space)

        0x183C3C1818001800UL,  // U+0021 (!)
        0x3636000000000000UL,  // U+0022 (")
        0x36367F367F363600UL,  // U+0023 (#)
        0x0C3E031E301F0C00UL,  // U+0024 ($)
        0x006333180C666300UL,  // U+0025 (%)
        0x1C361C6E3B336E00UL,  // U+0026 (&)
        0x0606030000000000UL,  // U+0027 (')
        0x180C0606060C1800UL,  // U+0028 (()
        0x060C1818180C0600UL,  // U+0029 ())
        0x00663CFF3C660000UL,  // U+002A (*)
        0x000C0C3F0C0C0000UL,  // U+002B (+)
        0x00000000000C0C06UL,  // U+002C (,)
        0x0000003F00000000UL,  // U+002D (-)
        0x00000000000C0C00UL,  // U+002E (.)
        0x6030180C06030100UL,  // U+002F (/)
        0x3E63737B6F673E00UL,  // U+0030 (0)
        0x0C0E0C0C0C0C3F00UL,  // U+0031 (1)
        0x1E33301C06333F00UL,  // U+0032 (2)
        0x1E33301C30331E00UL,  // U+0033 (3)
        0x383C36337F307800UL,  // U+0034 (4)
        0x3F031F3030331E00UL,  // U+0035 (5)
        0x1C06031F33331E00UL,  // U+0036 (6)
        0x3F3330180C0C0C00UL,  // U+0037 (7)
        0x1E33331E33331E00UL,  // U+0038 (8)
        0x1E33333E30180E00UL,  // U+0039 (9)
        0x000C0C00000C0C00UL,  // U+003A (:)
        0x000C0C00000C0C06UL,  // U+003B (//)
        0x180C0603060C1800UL,  // U+003C (<)
        0x00003F00003F0000UL,  // U+003D (=)
        0x060C1830180C0600UL,  // U+003E (>)
        0x1E3330180C000C00UL,  // U+003F (?)
        0x3E637B7B7B031E00UL,  // U+0040 (@)
        0x0C1E33333F333300UL,  // U+0041 (A)
        0x3F66663E66663F00UL,  // U+0042 (B)
        0x3C66030303663C00UL,  // U+0043 (C)
        0x1F36666666361F00UL,  // U+0044 (D)
        0x7F46161E16467F00UL,  // U+0045 (E)
        0x7F46161E16060F00UL,  // U+0046 (F)
        0x3C66030373667C00UL,  // U+0047 (G)
        0x3333333F33333300UL,  // U+0048 (H)
        0x1E0C0C0C0C0C1E00UL,  // U+0049 (I)
        0x7830303033331E00UL,  // U+004A (J)
        0x6766361E36666700UL,  // U+004B (K)
        0x0F06060646667F00UL,  // U+004C (L)
        0x63777F7F6B636300UL,  // U+004D (M)
        0x63676F7B73636300UL,  // U+004E (N)
        0x1C36636363361C00UL,  // U+004F (O)
        0x3F66663E06060F00UL,  // U+0050 (P)
        0x1E3333333B1E3800UL,  // U+0051 (Q)
        0x3F66663E36666700UL,  // U+0052 (R)
        0x1E33070E38331E00UL,  // U+0053 (S)
        0x3F2D0C0C0C0C1E00UL,  // U+0054 (T)
        0x3333333333333F00UL,  // U+0055 (U)
        0x33333333331E0C00UL,  // U+0056 (V)
        0x6363636B7F776300UL,  // U+0057 (W)
        0x6363361C1C366300UL,  // U+0058 (X)
        0x3333331E0C0C1E00UL,  // U+0059 (Y)
        0x7F6331184C667F00UL,  // U+005A (Z)
        0x1E06060606061E00UL,  // U+005B ([)
        0x03060C1830604000UL,  // U+005C (\)
        0x1E18181818181E00UL,  // U+005D (])
        0x081C366300000000UL,  // U+005E (^)
        0x00000000000000FFUL,  // U+005F (_)
        0x0C0C180000000000UL,  // U+0060 (`)
        0x00001E303E336E00UL,  // U+0061 (a)
        0x0706063E66663B00UL,  // U+0062 (b)
        0x00001E3303331E00UL,  // U+0063 (c)
        0x3830303e33336E00UL,  // U+0064 (d)
        0x00001E333f031E00UL,  // U+0065 (e)
        0x1C36060f06060F00UL,  // U+0066 (f)
        0x00006E33333E301FUL,  // U+0067 (g)
        0x0706366E66666700UL,  // U+0068 (h)
        0x0C000E0C0C0C1E00UL,  // U+0069 (i)
        0x300030303033331EUL,  // U+006A (j)
        0x070666361E366700UL,  // U+006B (k)
        0x0E0C0C0C0C0C1E00UL,  // U+006C (l)
        0x0000337F7F6B6300UL,  // U+006D (m)
        0x00001F3333333300UL,  // U+006E (n)
        0x00001E3333331E00UL,  // U+006F (o)
        0x00003B66663E060FUL,  // U+0070 (p)
        0x00006E33333E3078UL,  // U+0071 (q)
        0x00003B6E66060F00UL,  // U+0072 (r)
        0x00003E031E301F00UL,  // U+0073 (s)
        0x080C3E0C0C2C1800UL,  // U+0074 (t)
        0x0000333333336E00UL,  // U+0075 (u)
        0x00003333331E0C00UL,  // U+0076 (v)
        0x0000636B7F7F3600UL,  // U+0077 (w)
        0x000063361C366300UL,  // U+0078 (x)
        0x00003333333E301FUL,  // U+0079 (y)
        0x00003F190C263F00UL,  // U+007A (z)
        0x380C0C070C0C3800UL,  // U+007B ({)
        0x1818180018181800UL,  // U+007C (|)
        0x070C0C380C0C0700UL,  // U+007D (})
        0x6E3B000000000000UL,  // U+007E (~)
        0x0000000000000000UL,  // U+007F
        0xFFFFFFFFFFFFFFFFUL,  // U+0080 (backspace character for now.)
    };

    void drawpixel(size_t x, size_t y, pixel_t colours) {
        buffer[y * width + x] = colours;
    }

    void drawchar(size_t x, size_t y, int index, pixel_t colours) {
        bitmap_font_t c = fontmap[index];

        for(size_t i = 0; i < 8; i++) {
            for(size_t j = 0; j < 8; j++) {
                if(checkbit(c.rows[i], j)) drawpixel(j + (x * 8), i + (y * 8), colours);
            }
        }
    }

    void scroll(size_t n) {
        // Copies ahead of the buffer back into the base pointer, effectively scrolling.
        memory::copy(buffer, buffer + width * n, width * height * bpp);
    }

    void write(const char *str) {
        if(buffer == NULL) return;

        for(size_t i = 0; i < cstr::len(str); i++) {
            // If character is a newline, advance rows.
            if(str[i] == '\n') { cursorx = 0; cursory++; }

            // If character is a backspace, reverse columns.
            else if(str[i] == '\b') {
                if(cursorx != 0) cursorx--;
                drawchar(cursorx, cursory, 128, 0x000000);
            }

            // Otherwise, print character to screen.
            else drawchar(cursorx++, cursory, str[i], colour);

            // Scrolling code.
            if(cursorx == width) cursory++;
            else if(cursory == height) {
                cursory = height - 1;
                scroll(1);
            }

            if(cursorx == width || cursory == height) cursorx = 0;
        }
    }

    void initialise(mb_info_t *mbd) {
        buffer = (pixel_t*) mbd->framebufferaddr;
        textmaxh = mbd->framebufferheight / 8;
        textmaxw = mbd->framebufferwidth / 8;
        height = mbd->framebufferheight;
        width = mbd->framebufferwidth;
        pitch = mbd->framebufferpitch;
        bpp = mbd->framebufferpitch / 8;
        colour = 0xFFFFFF;
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

    void putentryat(char c, uint8_t colour, size_t x, size_t y) {
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