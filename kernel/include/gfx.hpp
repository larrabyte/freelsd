#ifndef FREELSD_KERNEL_GFX_HEADER
#define FREELSD_KERNEL_GFX_HEADER

#include <stddef.h>
#include <stdint.h>

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
    } colour_td;

    // Scrolls the terminal by n lines.
    void scroll(size_t n);

    // Initialise VGA-compatiable text mode.
    void initialise(void);

    // Set VGA colours.
    void setcolour(colour_td fg, colour_td bg);

    // Place a character at (x, y).
    void putentryat(char c, uint8_t colour, size_t x, size_t y);

    // A wrapper around putentryat(). Keeps track of rows and columns for you :)
    void write(const char *data);
}

#endif