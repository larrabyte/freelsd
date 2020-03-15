#ifndef FREELSD_KERNEL_MULTIBOOT_HEADER
#define FREELSD_KERNEL_MULTIBOOT_HEADER

#include <stdint.h>

// How many bytes from the start of the file we search for the header.
#define MULTIBOOT_SEARCH                     8192
#define MULTIBOOT_HEADER_ALIGN               4

// Multiboot framebuffer types.
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED   0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB       1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT  2

// Multiboot memory define values.
#define MULTIBOOT_MEMORY_AVAILABLE           1
#define MULTIBOOT_MEMORY_RESERVED            2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE    3
#define MULTIBOOT_MEMORY_NVS                 4
#define MULTIBOOT_MEMORY_BADRAM              5

// The multiboot magic number.
#define MULTIBOOT_HEADER_MAGIC               0x1BADB002

// This should be in the eax register.
#define MULTIBOOT_BOOTLOADER_MAGIC           0x2BADB002

// Alignment of multiboot modules.
#define MULTIBOOT_MOD_ALIGN                  0x00001000

// Alignment of the multiboot info structure.
#define MULTIBOOT_INFO_ALIGN                 0x00000004

// ---------------------------------------------------------
// Flags, set in the 'flags' member of the multiboot header.
// ---------------------------------------------------------

#define MULTIBOOT_PAGE_ALIGN                 0x00000001
#define MULTIBOOT_MEMORY_INFO                0x00000002
#define MULTIBOOT_VIDEO_MODE                 0x00000004
#define MULTIBOOT_AOUT_KLUDGE                0x00010000

// ----------------------------------------------------------------------
// Flags to be set in the 'flags' member of the multiboot info structure.
// ----------------------------------------------------------------------

#define MULTIBOOT_INFO_MEMORY                0x00000001
#define MULTIBOOT_INFO_BOOTDEV               0x00000002
#define MULTIBOOT_INFO_CMDLINE               0x00000004
#define MULTIBOOT_INFO_MODS                  0x00000008

// These two are mutually exclusive.
#define MULTIBOOT_INFO_AOUT_SYMS             0x00000010
#define MULTIBOOT_INFO_ELF_SHDR              0x00000020

#define MULTIBOOT_INFO_MEM_MAP               0x00000040
#define MULTIBOOT_INFO_DRIVE_INFO            0x00000080
#define MULTIBOOT_INFO_CONFIG_TABLE          0x00000100
#define MULTIBOOT_INFO_BOOT_LOADER_NAME      0x00000200
#define MULTIBOOT_INFO_APM_TABLE             0x00000400
#define MULTIBOOT_INFO_VBE_INFO              0x00000800
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO      0x00001000

// ---------------------------------------
// Multiboot structs and type definitions.
// ---------------------------------------

typedef struct multiboot_header {
    // Must be MULTIBOOT_MAGIC.
    uint32_t magic;

    // Multiboot flags.
    uint32_t flags;

    // The above values plus this one must equal 0 mod 2^32.
    uint32_t checksum;

    // Values below are only valid if MULTIBOOT_AOUT_KLUDGE is defined.
    uint32_t headeraddr;
    uint32_t loadaddr;
    uint32_t loadendaddr;
    uint32_t bssendaddr;
    uint32_t entryaddr;

    // Values below are only valid if MULTIBOOT_VIDEO_MODE is defined.
    uint32_t modetype;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
} mb_header_t;

typedef struct multiboot_symbol_table_aout {
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;
} mbst_aout_t;

typedef struct multiboot_header_table_elf {
    uint32_t num;
    uint32_t size;
    uint32_t addr;
    uint32_t shndx;
} mbht_elf_t;

typedef struct multiboot_info {
    // Multiboot version number.
    uint32_t flags;

    // Available memory, fetched from BIOS/UEFI.
    uint32_t lowermem;
    uint32_t uppermem;

    // 'root' partition.
    uint32_t bootdevice;

    // Kernel command line.
    uint32_t cmdline;

    // Boot module list.
    uint32_t modcount;
    uint32_t modaddr;

    union {
        mbst_aout_t aout;
        mbht_elf_t elf;
    } u;

    // Memory mapping buffer.
    uint32_t mmaplength;
    uint32_t mmapaddr;

    // Drive info buffer.
    uint32_t drivelength;
    uint32_t driveaddr;

    // ROM configuration table.
    uint32_t configtable;

    // Bootloader name.
    uint32_t bootloadername;

    // APM table.
    uint32_t apmtable;

    // Video variables.
    uint32_t vbecontrolinfo;
    uint32_t vbemodeinfo;
    uint32_t vbemode;
    uint32_t vbeinterfaceseg;
    uint32_t vbeinterfaceoff;
    uint32_t vbeinterfacelen;

    uint64_t framebufferaddr;
    uint64_t framebufferpitch;
    uint64_t framebufferwidth;
    uint64_t framebufferheight;
    uint8_t framebuffertype;

    union {
        struct {
            uint32_t framebufferpaletteaddr;
            uint16_t framebufferpalettenumcolours;
        };

        struct {
            uint8_t framebufferredfieldpos;
            uint8_t framebufferredmasksize;
            uint8_t framebuffergreenfieldpos;
            uint8_t framebuffergreenmasksize;
            uint8_t framebufferbluefieldpos;
            uint8_t framebufferbluemasksize;
        };
    };
} mb_info_t;

typedef struct multiboot_colour {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} mb_colour_t;

typedef struct multiboot_memorymap_entry {
    uint32_t size;
    uint32_t lowaddr;
    uint32_t highaddr;
    uint32_t lowlen;
    uint32_t highlen;
    uint32_t type;
} __attribute__((packed)) mb_mmap_t;

typedef struct multiboot_mod_list {
    uint32_t modstart;
    uint32_t modend;
    uint32_t cmdline;
    uint32_t pad;
} mb_modlist_t;

typedef struct multiboot_apm_info {
    uint16_t version;
    uint16_t cseg;
    uint32_t offset;
    uint16_t cseg16;
    uint16_t dseg;
    uint16_t flags;
    uint16_t cseglen;
    uint16_t cseg16len;
    uint16_t dseglen;
} mb_apm_t;

#endif