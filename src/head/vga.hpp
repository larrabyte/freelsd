#pragma once

#include <stddef.h>
#include <stdint.h>

namespace vga {
    static uint16_t *buffer;
    static uint8_t colour;
    static size_t column;
    static size_t row;

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

    // Initialise a VGA text mode terminal.
    void initialise(void);

    // Set the terminal colour.
    void setcolour(uint8_t colour);

    // Place a character at (x, y).
    void putentryat(char c, uint8_t colour, size_t x, size_t y);

    // A wrapper around termputentryat(). Keeps track of rows and columns for you :)
    void write(const char *data);

}