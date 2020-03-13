#include <memory.hpp>
#include <cstr.hpp>
#include <vga.hpp>

static const size_t VGA_HEIGHT = 25;
static const size_t VGA_WIDTH = 80;

uint8_t vga::colour = VGA_LIGHT_GREY | VGA_BLACK << 4;  // Current terminal colours.
uint16_t *vga::buffer = (uint16_t*) 0xb8000;            // Pointer to video memory.
size_t vga::column = 0;                                 // Current terminal column.
size_t vga::row = 0;                                    // Current terminal row.

static inline uint16_t entry(unsigned char uc, uint8_t colour) {
    return (uint16_t) uc | (uint16_t) colour << 8;
}

inline void vga::putentryat(char c, uint8_t colour, size_t x, size_t y) {
    buffer[y * VGA_WIDTH + x] = entry(c, colour);
}

void vga::write(const char *data) {
    for(size_t i = 0; i < cstr::len(data); i++) {
        if(data[i] == '\n') { column = 0; row++; }
        else putentryat(data[i], colour, column++, row);

        if(column == VGA_WIDTH) row++;
        else if(row == VGA_HEIGHT) {
            row = VGA_HEIGHT - 1;
            scroll(1);
        }

        if(column == VGA_WIDTH || row == VGA_HEIGHT) column = 0;
    }
}

void vga::setcolour(colour_td fg, colour_td bg) {
    colour = fg | bg << 4;
}

void vga::scroll(size_t n) {
    // Copies ahead of the buffer back into the base pointer, effectively scrolling.
    memory::copy(buffer, buffer + VGA_WIDTH * n, VGA_WIDTH * VGA_HEIGHT * 2);
}

void vga::initialise(void) {
    // Zero the VGA buffer. Effectively clears the screen.
    memory::set(buffer, 0, VGA_WIDTH * VGA_HEIGHT * 2);
}