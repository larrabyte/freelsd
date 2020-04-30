#ifndef FREELSD_KERNEL_GFX_BITMAP_HEADER
#define FREELSD_KERNEL_GFX_BITMAP_HEADER

#include <stdint.h>

namespace gfx {
    typedef enum bitmap_compression {
        BI_RGB = 0,             // No compression.
        BI_RLE8 = 1,            // RLE compression, 8 bits per pixel.
        BI_RLE4 = 2,            // RLE compression, 4 bits per pixel.
        BI_BITFIELDS = 3,       // Huffman coding.
        BI_JPEG = 4,            // RLE compression, 24 bits per pixel.
        BI_PNG = 5,             // ?
        BI_ALPHABITFIELDS = 6,  // RGBA bitfield masks.
        BI_CMYK = 11,           // No compression.
        BI_CMYKRLE8 = 12,       // RLE compression, 8 bits per pixel.
        BI_CMYKRLE4 = 13        // RLE compression, 4 bits per pixel.
    } bmp_compression_t;

    typedef struct bitmap_pixel24 {
        uint8_t red, green, blue;
    } __attribute__((packed)) bmp_pixel24_t;

    typedef struct bitmap_coreheader_os2 {
        uint16_t width;
        uint16_t height;
        uint16_t colourplanes;
        uint16_t bpp;
    } __attribute__((packed)) bmpdib_os2_t;

    typedef struct bitmap_coreheader_win2x {
        int16_t width;
        int16_t height;
        uint16_t colourplanes;
        uint16_t bpp;
    } __attribute__((packed)) bmpdib_win2x_t;

    typedef struct bitmap_coreheader_win32 {
        int32_t width;
        int32_t height;
        uint16_t colourplanes;
        uint16_t bpp;
        uint32_t compression;
        uint32_t imgsize;
        int32_t ppmwidth;
        int32_t ppmheight;
        uint32_t colourcount;
        uint32_t important;
    } __attribute__((packed)) bmpdib_win32_t;

    typedef struct bitmap_header {
        uint16_t magic;
        uint32_t filesize;
        uint32_t reserved;
        uint32_t offset;
        uint32_t dibsize;

        union coreheaders {
            struct bitmap_coreheader_os2 os2;
            struct bitmap_coreheader_win2x win2x;
            struct bitmap_coreheader_win32 win32;
        } dibs;
    } __attribute__((packed)) bmp_header_t;

}

#endif
