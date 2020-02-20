#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static const size_t VGA_HEIGHT = 25;
static const size_t VGA_WIDTH = 80;
uint16_t *terminalbuffer;
uint8_t terminalcolor;
size_t terminalcolumn;
size_t terminalrow;

typedef enum vgacolors {
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
} vgacolors_enum;

static inline uint8_t vgaentrycolor(vgacolors_enum fg, vgacolors_enum bg) {
    return fg | bg << 4;
}

static inline uint16_t vgaentry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while(str[len]) len++;
    return len;
}

void terminitialise(void) {
    terminalcolor = vgaentrycolor(VGA_LIGHT_GREY, VGA_BLACK);
    terminalcolumn = 0;
    terminalrow = 0;

    terminalbuffer = (uint16_t*) 0xb8000;
    for(size_t y = 0; y < VGA_HEIGHT; y++) {
        for(size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminalbuffer[index] = vgaentry(' ', terminalcolor);
        }
    }
}

void termsetcolor(uint8_t color) {
    terminalcolor = color;
}

void termputentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminalbuffer[index] = vgaentry(c, color);
}

void termputchar(char c) {
    termputentryat(c, terminalcolor, terminalcolumn, terminalrow);
    if(++terminalcolumn == VGA_WIDTH) {
        if(++terminalrow == VGA_HEIGHT) terminalrow = 0;
        terminalcolumn = 0;
    }
}

void termwrite(const char *data, size_t size) {
    for(size_t i = 0; i < size; i++) termputchar(data[i]);
}

void termwritestr(const char *data) {
    termwrite(data, strlen(data));
}

void kernelmain() {
    terminitialise();
    termwritestr("Hello, kernel World!\n");
}