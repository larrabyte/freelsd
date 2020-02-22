#include "head/cstr.hpp"
#include "head/vga.hpp"
#include <stddef.h>
#include <stdint.h>

static const size_t VGA_HEIGHT = 25;
static const size_t VGA_WIDTH = 80;

static inline uint8_t entrycolour(vga::colour_td fg, vga::colour_td bg) {  
    return fg | bg << 4;
}

static inline uint16_t entry(unsigned char uc, uint8_t colour) {
    return (uint16_t) uc | (uint16_t) colour << 8;  
}

void vga::setcolour(uint8_t colour) {
    vga::colour = colour;
}

void vga::putentryat(char c, uint8_t colour, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    vga::buffer[index] = entry(c, colour);
}

void vga::write(const char *data) {
    for(size_t i = 0; i < cstr::len(data); i++) {
        if(data[i] == '\n') {
            vga::column = 0;
            vga::row++;
            continue;
        }

        vga::putentryat(data[i], vga::colour, vga::column++, vga::row);
        if(vga::column == VGA_WIDTH) {
            if(vga::row == VGA_HEIGHT) vga::row = 0;
            else vga::row++;

            vga::column = 0;
        }
    }
}

void vga::initialise(void) {
    vga::colour = entrycolour(vga::VGA_LIGHT_GREY, vga::VGA_BLACK);
    vga::column = 0;
    vga::row = 0;

    vga::buffer = (uint16_t*) 0xb8000;
    for(size_t y = 0; y < VGA_HEIGHT; y++) {
        for(size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga::buffer[index] = entry(' ', vga::colour);
        }
    }
}