#include <memory.hpp>
#include <cstr.hpp>
#include <vga.hpp>

static const size_t VGA_HEIGHT = 25;
static const size_t VGA_WIDTH = 80;

uint16_t *vga::buffer;  // Pointer to video memory.
uint8_t vga::colour;    // Current terminal colours.
size_t vga::column;     // Current terminal column.
size_t vga::row;        // Current terminal row.

static inline uint8_t entrycolour(vga::colour_td fg, vga::colour_td bg) {  
    return fg | bg << 4;
}

static inline uint16_t entry(unsigned char uc, uint8_t colour) {
    return (uint16_t) uc | (uint16_t) colour << 8;
}

void vga::setcolour(uint8_t scolour) {
    colour = scolour;
}

void vga::putentryat(char c, uint8_t colour, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    buffer[index] = entry(c, colour);
}

void vga::scroll(size_t n) {
    memory::copy(buffer, buffer + VGA_WIDTH * n, VGA_WIDTH * VGA_HEIGHT * 2);
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

void vga::initialise(void) {
    colour = entrycolour(VGA_LIGHT_GREY, VGA_BLACK);
    column = 0;
    row = 0;

    buffer = (uint16_t*) 0xb8000;
    for(size_t y = 0; y < VGA_HEIGHT; y++) {
        for(size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            buffer[index] = entry(' ', colour);
        }
    }
}