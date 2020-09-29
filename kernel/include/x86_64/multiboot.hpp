#ifndef FREELSD_KERNEL_MULTIBOOT2_HEADER
#define FREELSD_KERNEL_MULTIBOOT2_HEADER

#include <acpi.hpp>
#include <stdint.h>

// Checks if the n-th bit of bits is set.
#define checkbit(bits, n) ((bits) & (1 << (n)))

// How many bytes from the start of the file we search for the header.
#define MULTIBOOT_SEARCH                            32768
#define MULTIBOOT_HEADER_ALIGN                      8

// Multiboot 2 magic values.
#define MULTIBOOT2_HEADER_MAGIC                     0xE85250D6
#define MULTIBOOT2_BOOTLOADER_MAGIC                 0x36D76289

// Alignment of multiboot structures.
#define MULTIBOOT_MOD_ALIGN                         0x00001000
#define MULTIBOOT_INFO_ALIGN                        0x00000008
#define MULTIBOOT_TAG_ALIGN                         0x00000008

// Multiboot memory types.
#define MULTIBOOT_MEMORY_AVAILABLE                  1
#define MULTIBOOT_MEMORY_RESERVED                   2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE           3
#define MULTIBOOT_MEMORY_NVS                        4
#define MULTIBOOT_MEMORY_BADRAM                     5
#define MULTIBOOT_MEMORY_PERSISTENT                 12
#define MULTIBOOT_MEMORY_COREBOOT                   16
#define MULTIBOOT_MEMORY_FIRMWARE                   20

// ---------------------------------------------------------
// Flags, set in the 'flags' member of the multiboot header.
// ---------------------------------------------------------

#define MULTIBOOT_TAG_TYPE_END                      0
#define MULTIBOOT_TAG_TYPE_CMDLINE                  1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME         2
#define MULTIBOOT_TAG_TYPE_MODULE                   3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO            4
#define MULTIBOOT_TAG_TYPE_BOOTDEV                  5
#define MULTIBOOT_TAG_TYPE_MMAP                     6
#define MULTIBOOT_TAG_TYPE_VBE                      7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER              8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS             9
#define MULTIBOOT_TAG_TYPE_APM                      10
#define MULTIBOOT_TAG_TYPE_EFI32                    11
#define MULTIBOOT_TAG_TYPE_EFI64                    12
#define MULTIBOOT_TAG_TYPE_SMBIOS                   13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD                 14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW                 15
#define MULTIBOOT_TAG_TYPE_NETWORK                  16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP                 17
#define MULTIBOOT_TAG_TYPE_EFI_BS                   18
#define MULTIBOOT_TAG_TYPE_EFI32_IH                 19
#define MULTIBOOT_TAG_TYPE_EFI64_IH                 20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR           21

#define MULTIBOOT_HEADER_TAG_END                    0
#define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST    1
#define MULTIBOOT_HEADER_TAG_ADDRESS                2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS          3
#define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS          4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER            5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGN           6
#define MULTIBOOT_HEADER_TAG_EFI_BS                 7
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32    8
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64    9
#define MULTIBOOT_HEADER_TAG_RELOCATABLE            10

#define MULTIBOOT_ARCHITECTURE_I386                 0
#define MULTIBOOT_ARCHITECTURE_MIPS32               4
#define MULTIBOOT_HEADER_TAG_OPTIONAL               1

#define MULTIBOOT_LOAD_PREFERENCE_NONE              0
#define MULTIBOOT_LOAD_PREFERENCE_LOW               1
#define MULTIBOOT_LOAD_PREFERENCE_HIGH              2

#define MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED    1
#define MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED  2

// ---------------------------------------
// Multiboot structs and type definitions.
// ---------------------------------------

typedef struct multiboot_header {
    uint32_t magic;
    uint32_t arch;
    uint32_t length;
    uint32_t checksum;
} mb_hdr_t;

typedef struct multiboot_header_tag {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
} mb_hdrtag_t;

typedef struct multiboot_header_tag_info_request {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t requests[1];
} mb_hdrtag_inforequest_t;

typedef struct multiboot_header_tag_address {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t addr;
} mb_hdrtag_addr_t;

typedef struct multiboot_header_tag_console_flags {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t conflags;
} mb_hdrtag_conflags_t;

typedef struct multiboot_header_tag_framebuffer {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} mb_hdrtag_framebuffer_t;

typedef struct multiboot_header_tag_alignment {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
} mb_hdrtag_align_t;

typedef struct multiboot_header_tag_relocatable {
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    uint32_t minaddr;
    uint32_t maxaddr;
    uint32_t align;
    uint32_t preference;
} mb_hdrtag_relocatable_t;

typedef struct multiboot_colour {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} mb_colour_t;

typedef struct multiboot_memorymap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} mb_mmap_entry_t;

typedef struct multiboot_tag {
    uint32_t type;
    uint32_t size;
} mb_tag_t;

typedef struct multiboot_tag_string {
    uint32_t type;
    uint32_t size;
    char string[1];
} mb_tag_string_t;

typedef struct multiboot_tag_module {
    uint32_t type;
    uint32_t size;
    uint32_t start;
    uint32_t end;
    char cmdline[1];
} mb_tag_module_t;

typedef struct multiboot_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t lower;
    uint32_t upper;
} mb_tag_basicmem_t;

typedef struct multiboot_tag_bootdev {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t slice;
    uint32_t part;
} mb_tag_bootdev_t;

typedef struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entrysize;
    uint32_t entryversion;
    mb_mmap_entry_t entries[1];
} mb_tag_mmap_t;

typedef struct multiboot_vbe_info_block {
    uint8_t externalspec[512];
} mb_vbeinfo_t;

typedef struct multiboot_vbe_mode_block {
    uint8_t externalspec[256];
} mb_vbemode_t;

typedef struct multiboot_tag_vbe {
    uint32_t type;
    uint32_t size;
    uint16_t ifmode;
    uint16_t ifoff;
    uint16_t iflen;
    
    mb_vbeinfo_t vbecontrol;
    mb_vbemode_t vbemode;
} mb_tag_vbe_t;

typedef struct multiboot_tag_framebuffer_common {
    uint32_t type;
    uint32_t size;

    uint64_t framebuffer;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t fbtype;
    uint16_t reserved;
} mb_tag_framebuffer_common_t;

typedef struct multiboot_tag_framebuffer {
    mb_tag_framebuffer_common_t common;

    union ucolour {
        struct spalette {
            uint16_t palettecolour;
            mb_colour_t palette[1];
        } palette;

        struct sfieldmask {
            uint8_t rfieldpos;
            uint8_t rmasksize;
            uint8_t gfieldpos;
            uint8_t gmasksize;
            uint8_t bfieldpos;
            uint8_t bmasksize;
        } fieldmasks;
    } colours;
} mb_tag_framebuffer_t;

typedef struct multiboot_tag_elf_sections {
    uint32_t type;
    uint32_t size;
    uint32_t num;
    uint32_t entsize;
    uint32_t shndx;
    char sections[1];
} mb_tag_elf_t;

typedef struct multiboot_tag_apm {
    uint32_t type;
    uint32_t size;
    uint16_t version;
    uint16_t csegment;
    uint32_t offset;
    uint16_t csegment16;
    uint16_t dsegment;
    uint16_t flags;
    uint16_t csegmentlen;
    uint16_t csegment16len;
    uint16_t dsegmentlen;
} mb_tag_apm_t;

typedef struct multiboot_tag_efi32 {
    uint32_t type;
    uint32_t size;
    uint32_t pointer;
} mb_tag_efi32_t;

typedef struct multiboot_tag_efi64 {
    uint32_t type;
    uint32_t size;
    uint64_t pointer;
} mb_tag_efi64_t;

typedef struct multiboot_tag_smbios {
    uint32_t type;
    uint32_t size;
    uint8_t major;
    uint8_t minor;
    uint8_t reserved[6];
    uint8_t tables[1];
} mb_tag_smbios_t;

typedef struct multiboot_tag_acpi {
    uint32_t type;
    uint32_t size;
    uint8_t rsdp[1];
} mb_tag_acpi_t;

typedef struct multiboot_tag_network {
    uint32_t type;
    uint32_t size;
    uint8_t dhcpack[1];
} mb_tag_network_t;

typedef struct multiboot_tag_efi_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t descsize;
    uint32_t descvers;
    uint8_t efimmap[1];
} mb_tag_efimmap_t;

typedef struct multiboot_tag_load_baseaddr {
    uint32_t type;
    uint32_t size;
    uint32_t baseaddr;
} mb_tag_baseaddr_t;

// --------------------------------------
// FreeLSD-specific multiboot structures.
// --------------------------------------

typedef struct multiboot_information {
    char *cmdline, *bootloader;
    mb_tag_framebuffer_t *fbinfo;
    mb_tag_basicmem_t *meminfo;
    mb_tag_bootdev_t *bootdev;
    mb_tag_mmap_t *mmap;
    acpi::rsdp_t *rsdp;
} mb_info_t;

namespace mboot {
    // Multiboot information, stored in a neat little struct.
    extern mb_info_t info;

    // Get a string representing the memory map type given.
    const char *getmmaptype(uint8_t index);

    // Fetch data from multiboot information structures.
    void initialise(uint64_t magic, uintptr_t mbaddr);
}

#endif
